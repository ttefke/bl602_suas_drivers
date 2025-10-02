// Required to use bool
#include <stdbool.h>

// Library headers
#include <suas_uart.h>
#include <suas_mmWave.h>

// Driver for waveshare HMMD mmWave: https://www.waveshare.com/wiki/HMMD_mmWave_Sensor

void suas_init_mmwave() {
    /* Initialize UART: set baud rate -> from manual */
    suas_uart_init(115200);
}

bool suas_mmwave_is_human_present() {
    // Data array
    // -> we get on/off from sensor and use the second character to decide if someone is present
    int8_t data[2];

    for (uint8_t i = 0; i < 2;) {
        int8_t result = suas_uart_read_byte(); // Read incoming data

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