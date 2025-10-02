#include <Arduino.h>

extern "C" {
    #include <bl_gpio.h>
}

// Porting layer allowing to use the Arduino rfid library on the PineCone
// code taken from https://github.com/bouffalolab/arduino-bouffalo/

// IO functions
void pinMode(uint8_t pin, uint8_t mode) {
    if (mode & INPUT) {
        bl_gpio_enable_input(pin, mode & PULLUP ? 1 : 0, mode & PULLDOWN ? 1 : 0);
    } else if (mode & OUTPUT) {
        bl_gpio_enable_output(pin, mode & PULLUP ? 1 : 0, mode & PULLDOWN ? 1 :0);
    }
}

void digitalWrite(uint8_t pin, uint8_t val) {
    bl_gpio_output_set(pin, val ? 1 : 0);
}

int digitalRead(uint8_t pin) {
    int ret;
    uint8_t val;
    ret = bl_gpio_input_get(pin, &val);
    if (ret != 0) {
        return LOW;
    } else {
        return val ? HIGH : LOW;
    }
}