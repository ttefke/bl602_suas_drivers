# Note: the DHT22 sensor communication over one-wire requires meeting very short and specific time frames
# Therefore, the part which requires strict deadlines is written in assembly to guarantee sufficient speed

.global suas_dht22_get_readings_asm # Make this function globally available

.text                          # Start the text segment (program parts)

# Define constants used in our routines -> these will be replaced with their actual value at compile time
# Constants for GPIO communication
.set DHT22_VOLTAGE_HIGH,     1 # Binary one equals 3.3V
.set DHT22_VOLTAGE_LOW,      0 # Binary zero equals 0V
.set DHT22_DISABLE_PULLUP,   0 # Flag to disable pullup resistor
.set DHT22_DISABLE_PULLDOWN, 0 # Flag to disable pulldown resistor

# Constants for our routines to keep the stack consistent
# Here we define several offsets from the stack pointer for variables stored in memory
.set SAVE_FP_OFFSET,         0  # Offset for the frame pointer
.set SAVE_RA_OFFSET,         4  # Offset for the return address
.set SENSOR_DATA_PTR_OFFSET, 8  # Offset for the data pointer
.set SENSOR_PIN_OFFSET,      12 # Offset for the sensor pin
.set SENSOR_VALUE_OFFSET,    16 # Offset for the currently read sensor value
.set COUNTER_VALUE_OFFSET,   20 # Offset for the counter
.set MAX_OFFSET,             32 # Offset to know how many bytes to increase stack pointer (must be a multiple of 16)

# Macro to wait for one us and get the current GPIO state at the sensor's pin
.macro WAIT_ONE_US_AND_GET_GPIO
    # Wait for 1us
    li   a0, 1                       # Set parameter for delay function (1us delay)
    call bl_timer_delay_us           # Call delay function

    # Receive current GPIO input
    lw   a0, SENSOR_PIN_OFFSET(sp)   # Read pin number
    addi a1, sp, SENSOR_VALUE_OFFSET # Load sensor value address
    call bl_gpio_input_get           # Call input get function
.endm

# Macro to wait until the current GPIO state at the sensor's pin turns to high (1)
.macro WAIT_UNTIL_GPIO_IS_HIGH
.Lstart\@:
    # Wait for 1us and get GPIO state
    WAIT_ONE_US_AND_GET_GPIO         # Wait for one us and get GPIO state

    # Check if input equals DHT22_VOLTAGE_LOW -> loop until it is no longer low
    lbu  t0, SENSOR_VALUE_OFFSET(sp) # Load sensor value
    li   t1, DHT22_VOLTAGE_LOW       # Load low voltage
    beq  t0, t1, .Lstart\@           # Loop while sensor value is low
.endm

# Macro to wait until the current GPIO state at the sensor's pin turns to low (0)
# Here we allow to hand over a jump label because when counting the ms we need a counter value
# and this is not addressed by this macro. In this case, the caller has to define a counter value
# and increase it by using the jump label.
.macro WAIT_UNTIL_GPIO_IS_LOW jump_label=
.Lstart\@:
    # Wait for 1us and get GPIO state
    WAIT_ONE_US_AND_GET_GPIO         # Wait for one us and get GPIO state

    # Check if input equals DHT22_VOLTAGE_HIGH -> loop until it is no longer high
    lbu  t0, SENSOR_VALUE_OFFSET(sp) # Load sensor value
    li   t1, DHT22_VOLTAGE_HIGH      # Load high voltage
.ifc \jump_label,
    beq  t0, t1, .Lstart\@           # Loop while sensor value is high
.else
    beq  t0, t1, \jump_label         # Loop while sensor value is high and jump to custom jump label
.endif
.endm

# Our function to read data from the sensor
# Input parameters: a0 -> sensor pin number (uint8_t)
#                   a1 -> data array address (uint8_t *)
suas_dht22_get_readings_asm:
    # 0. Prologue: save required information to return from this function without breaking anything else
    addi sp, sp, -MAX_OFFSET       # Increase stack pointer
    sw   fp, SAVE_FP_OFFSET(sp)    # Save frame pointer on stack
    sw   ra, SAVE_RA_OFFSET(sp)    # Save return address on stack

    # 1. Store all information that is handed over as parameter
    sw   a0, SENSOR_PIN_OFFSET(sp)      # Save sensor pin number on stack (uint8_t in c)
    sw   a1, SENSOR_DATA_PTR_OFFSET(sp) # Save data array address on stack (pointer in C)

    # 2. Initialize counter as 0
    lw   t0, COUNTER_VALUE_OFFSET(sp) # Load counter variable
    mv   t0, zero                     # Zeroize
    sw   t0, COUNTER_VALUE_OFFSET(sp) # Store zeroized counter

    # 3. Set data pin as output pin to be able to request data
    lw   a0, SENSOR_PIN_OFFSET(sp)  # Restore pin number from stack
    li   a1, DHT22_DISABLE_PULLUP   # Disable pullup resistor
    li   a2, DHT22_DISABLE_PULLDOWN # Disable pulldown resistor
    call bl_gpio_enable_output      # Set GPIO pin as output pin

    # 4. Request data aquisition 
    # Send start signal (1/6): set pin to high voltage (as expected by DHT22 according to documentation)
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    li   a1, DHT22_VOLTAGE_HIGH    # Set high voltage
    call bl_gpio_output_set        # Set GPIO pin to high

    # Send start signal (2/6): wait for 10,000us
    li   a0, 10000         # Load delay value
    call bl_timer_delay_us # Call delay function

    # Send start signal (3/6): set pin to low
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    li   a1, DHT22_VOLTAGE_LOW     # Set parameter to low voltage
    call bl_gpio_output_set        # Set output

    # Send start signal (4/6): delay by at least 1ms (we use 1,250 us to be sure)
    li   a0, 1250          # Load delay value
    call bl_timer_delay_us # Call delay function

    # Send start signal (5/6): set signal to high again
    lw   a0, SENSOR_PIN_OFFSET(sp) # Restore pin number from stack
    li   a1, DHT22_VOLTAGE_HIGH    # Set parameter to high voltage
    call bl_gpio_output_set        # Set output

    # Send start signal (6/6): wait for 40us
    li   a0, 40            # Load delay value
    call bl_timer_delay_us # Call delay function

    # 5. Set data pin as input to be able to read the sensors response
    lw   a0, SENSOR_PIN_OFFSET(sp)  # Restore pin number from stack
    li   a1, DHT22_DISABLE_PULLUP   # Disable pullup resistor
    li   a2, DHT22_DISABLE_PULLDOWN # Disable pulldown resistor
    call bl_gpio_enable_input       # Set as input pin from now on

    # 6. Read the response from the sensor
    # 6.1 The sensor now pulls low for a short time to signal incoming data
    WAIT_UNTIL_GPIO_IS_HIGH

    # 6.2 The sensor now pulls high for short amount of time
    WAIT_UNTIL_GPIO_IS_LOW

    # 7. Data arrives
    # This process happens 40 times: we receive low for 50us and then high for either ~28us (0) or ~70us (1)
1:
    # 7.1 Wait until low is over
    WAIT_UNTIL_GPIO_IS_HIGH

    li   t3, 0     # Use t3 as counter
    li   t4, 1     # Store a one here to increase the counter
2:
    add  t3, t3, t4 # Increase counter
    
    # 7.2 Wait until high is over
    # Here we jump back to 2b because we also have to increase the counter
    WAIT_UNTIL_GPIO_IS_LOW 2b

    # 7.3 If current us count is less than 28, we count this as low
    li   t1, 28     # Load 28
    bleu t3, t1, 3f # Jump directly to check the number of iterations if the bit is 0 -> no need to store anything
    
    # 8. This function part is used to store the current bit if it is one
    # 8.1 Compute byte and bit positions
    li   t0, 8                        # Load 8 into t0
    lw   t1, COUNTER_VALUE_OFFSET(sp) # Read current counter value into t1
    div  t2, t1, t0                   # Byte position: t2 = t1 / 8
    li   t3, 7                        # We have MSB, so we have to calculate the bit position as 7 - (counter % 8)
    rem  t4, t1, t0                   # Calculate LSB position: t3 = t1 % 8
    sub  t3, t3, t4                   # Subtract the seven -> Convert LSB to MSB

    # 8.2 Load required byte
    lw   t0, SENSOR_DATA_PTR_OFFSET(sp) # Add data location pointer
    add  t0, t0, t2                     # Increase by byte position
    lbu  t1, 0(t0)                      # Load byte to be modified into t1

    # 8.3 Set bit at computed position to 1
    li   t4, 1       # Load a 1
    sll  t4, t4, t3  # Shift left to reach bit position
    or   t1, t1, t4  # Apply boolean or to set bit to 1
    sb   t1, 0(t0)   # Store byte into previously obtained location
3:
    # 9. Jump back until we got our 40 iterations
    # 9.1 Increase counter
    li   t0, 1                        # Load 1 into t0
    lw   t1, COUNTER_VALUE_OFFSET(sp) # Load counter into t1
    add  t1, t0, t1                   # Increase counter
    sw   t1, COUNTER_VALUE_OFFSET(sp) # Store counter

    # Check if the counter is 40
    li   t2, 40                       # Load 40 into t2
    bne  t2, t1, 1b                   # Check if t1 (still contains counter) is fourty -> go back to 1 if not

    # If we are here, all data is received and we can continue in C as all time-critical operations are done
    # 10. Prologue: ensure we hand over a clean stack to the next function
    lw   fp, SAVE_FP_OFFSET(sp)    # Restore frame pointer
    lw   ra, SAVE_RA_OFFSET(sp)    # Restore return address
    addi sp, sp, MAX_OFFSET     
    ret