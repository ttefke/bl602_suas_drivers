// FreeRTOS includes (locking)
#include <FreeRTOS.h>
#include <task.h>

// Own header
#include <suas_ultrasonic.h>

// Function to get duration using the extern assembly function
static uint32_t suas_get_ultrasonic_duration(uint8_t pin) {
    // Prototype for assembly function
    extern uint32_t suas_get_ultrasonic_duration_asm(uint8_t pin);

    // Define result data
    uint32_t result = 0;

    taskENTER_CRITICAL(); // Disable interrupts for time-critical communication
    result = suas_get_ultrasonic_duration_asm(pin); // Get duration of ultrasonic signals
    taskEXIT_CRITICAL(); // Time-critical communication is over, enable interrupts again

    // Return data
    return result;
}

// Compute distance in centimeters
uint32_t suas_measure_in_centimeters(uint8_t pin) {
    return (suas_get_ultrasonic_duration(pin) / 29 / 2);
}

// Compute distance in millimeters
uint32_t suas_measure_in_millimeters(uint8_t pin) {
    return (suas_get_ultrasonic_duration(pin)  * (10 / 2) / 29);
}