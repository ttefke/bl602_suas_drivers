#include <Serial.h>

// Porting layer allowing to use the Arduino rfid library on the PineCone

extern "C" {
    #include <stdio.h>
}

void SerialClass::begin([[gnu::unused]] unsigned long baud) {
        // UART initializes on FreeRTOS startup
}

size_t SerialClass::print(const char string[]) {
    return printf(string);
}

size_t SerialClass::print(int val, int format) {
    switch (format) {
        case BIN:
            for (int i = sizeof(val) * 8 - 1; i >= 0; i--) {
                putchar((val & (1U << i)) ? '1' : '0');
            }
            return (sizeof(val)*8-1);
        break;
        case OCT:
            return printf("%o", val);
        break;
        case HEX:
            return printf("%02x", val);
        break;
        default:
            // Dec
            return printf("%d", val); 
    }
}

size_t SerialClass::println(const char string[]) {
    size_t result = this->print(string);
    this->print("\r\n");
    return result + 2;
}

size_t SerialClass::println(int val, int format) {
    size_t result = this->print(val, format);
    this->print("\r\n");
    return result + 2;
}

size_t SerialClass::println(void) {
    this->print("\r\n");
    return 2;
}

SerialClass Serial;