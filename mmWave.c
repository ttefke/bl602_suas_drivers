// Required to use bool
#include <stdbool.h>

// Include UART HAL
#include <bl_uart.h>

#include "include/suas_mmWave.h" // own header

// Driver for waveshare HMMD mmWave: https://www.waveshare.com/wiki/HMMD_mmWave_Sensor

void init_mmwave() {
    /* Initialize UART for sensor
    * Ports: 3+4 (TX+RX)
    * Baudrate: 115200
    */
    bl_uart_init(
        1 /* Channel 0 = USB, Channel 1 = user defined UART*/,
        3, /* TX pin*/
        4, /* RX pin*/
        255, /* Unused*/
        255, /* Unused */
        115200 /* Baud rate -> from manual */
    );
}

bool mmwave_is_human_present() {
    // Data array
    // -> we get on/off from sensor and use the second character to decide if someone is present
    int8_t data[2];

    for (uint8_t i = 0; i < 2;) {
        int8_t result = bl_uart_data_recv(1); // Read from channel 1

        // Drop invalid result
        if (result < 0) {
            continue;
        } else if ((i == 0) && (result != 'O')) {
            // Continue until we receive an 'O'
            // -> then we know data transfer starts
            continue;
        } else {
            // Write data into data array and increase i
            data[i] = result;
            i++;
        }
    }

    /* If the second character is an 'N',
        we know we received the text 'ON'.
        Then return true, otherwise return false.
    */
    return data[1] == 'N';
}