# Note: the ultrasonic sensor communication uses the one-write protocol which requires meeting some very short time frames
# Therefore, the time-critical part of this driver is written in assembly.

.global suas_ultrasonic_get_duration_asm # Make this function globally available

# Timeout message
.section .rodata
message: .asciz "\r\nUltrasonic: Timeout occurred - please ensure the sensor is connected and there is sufficient time between two subsequent calls to the sensor\r\n"

.text # Program starts here

# Constants used to save/restore the stack
.set SAVE_FP_OFFSET,         0  # Offset for the frame pointer
.set SAVE_RA_OFFSET,         4  # Offset for the return address
.set SENSOR_PIN_OFFSET,      8  # Offset to store the pin of the sensor
.set START_TIME_OFFSET,      12 # Offset to store the time we start with our measurement
.set PULSE_BEGIN_OFFSET,     16 # Offset to store the time the pulse starts
.set MAX_OFFSET,             32 # Offset must be a multiple of 16

# Macro that sets the selected sensor pin to low
.macro SET_PIN_TO_LOW
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    mv   a1, zero                  # Write desired output state (low = zero) to a1
    call bl_gpio_output_set        # Call function
.endm

# Macro that checks whether a timeout occurred
.macro CHECK_IF_TIMEOUT_OCCURRED
    call bl_timer_now_us           # Get current time -> then in a0
    lw   t0, START_TIME_OFFSET(sp) # Load start time into t0
    li   t1, 1000000               # Load amount of us in a s into t1
    sub  t0, a0, t0                # Compute time since start: t0 = a0 - t0 = current time - start time
    bgeu t0, t1, 4f                # Jump to end and return zero if timeout (waited longer than a sec): t0 > t1 = duration > 1s
.endm

# Macro to get the current input value
.macro GET_CURRENT_INPUT_VALUE
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    call bl_gpio_input_get_value   # Get pin value -> then in a0
.endm

# Function to get the duration of a pulse
suas_ultrasonic_get_duration_asm:
    # 0. Prologue: save information to the stack required to return from this function gracefully later
    addi sp, sp, -MAX_OFFSET    # Increase stack pointer
    sw   fp, SAVE_FP_OFFSET(sp) # Save frame pointer
    sw   ra, SAVE_RA_OFFSET(sp) # Save return adress

    # 1. Store handed over parameter: save pin
    sw   a0, SENSOR_PIN_OFFSET(sp) # Save pin number on stack

    # 2. Request data
    # 2.1. Configure pin as output
    lw   a0, SENSOR_PIN_OFFSET(sp) # Resture pin number from stack
    mv   a1, zero                  # Set pullup to zero
    mv   a2, zero                  # Set pulldown to zero
    call bl_gpio_enable_output     # Call function

    # 2.2. Set pin to low
    SET_PIN_TO_LOW

    # 2.3. Delay by 2 us
    li   a0, 2             # Load 2 into a0
    call bl_timer_delay_us # Call function

    # 2.4. Set pin to high
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    li   a1, 1                     # Load 1 ('high') into a1
    call bl_gpio_output_set        # Call function

    # 2.5. Delay by 5 us
    li   a0, 5             # Load 5 into a1
    call bl_timer_delay_us # Call function

    # 2.6. Set pin to low again
    SET_PIN_TO_LOW

    # 2.7. Configure pin as input
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    mv   a1, zero                  # Set pullup to zero
    mv   a2, zero                  # Set pulldown to zero
    call bl_gpio_enable_input      # Call function

    # 3. Start pulsing the sensor
    # 3.1. Store current time
    call bl_timer_now_us           # Get current time
    sw   a0, START_TIME_OFFSET(sp) # Store current time
1:
    # 3.2. Wait for the current pulse to end (if there is any) [pin turns low]
    # 3.2.1. Figure out whether timeout occurred
    CHECK_IF_TIMEOUT_OCCURRED

    # 3.2.2. Get current input value
    GET_CURRENT_INPUT_VALUE

    # 3.2.3. Loop while the pin is high (1)
    bnez a0, 1b # Go to definition of label 1 if pin is high
2:
    # 3.3. Wait for the pulse to start [pin turns high]
    # 3.3.1. Figure out whether a timeout occurred
    CHECK_IF_TIMEOUT_OCCURRED

    # 3.3.2. Get current input value
    GET_CURRENT_INPUT_VALUE

    # 3.3.3. Loop while the pin is low (0)
    beqz a0, 2b # Go to definition of label 2 if pin is low

    # 3.3.4. Store current time (pulse starts now!) 
    call bl_timer_now_us            # Get current time
    sw   a0, PULSE_BEGIN_OFFSET(sp) # Store current time
3:
    # 3.4. Wait for the pulse to end [pin turns low again]
    # 3.4.1. Figure out whether a timeout occurred
    CHECK_IF_TIMEOUT_OCCURRED

    # 3.4.2. Cet current input value
    GET_CURRENT_INPUT_VALUE

    # 3.4.3. Loop while the pin is high (1)
    bnez a0, 3b # Go to definition of label 3 if pin is high

    # 3.5. Compute duration of the pulse
    call bl_timer_now_us            # Get current time (end time)
    lw   t0, PULSE_BEGIN_OFFSET(sp) # Get stored time (begin time)
    sub  a0, a0, t0                 # Compute duration (a0 = end time - begin time)

    # 3.6. The duration is now already in a0 so we can directly return it
    j   5f # Jump to epilogue

4:  # Error handler
    # Show error message
    la   a0, message
    call printf
    
    # Set return value to zero
    mv   a0, zero
5:
    # 4. Epilogue
    lw   fp, SAVE_FP_OFFSET(sp)
    lw   ra, SAVE_RA_OFFSET(sp)
    addi sp, sp, MAX_OFFSET
    ret
