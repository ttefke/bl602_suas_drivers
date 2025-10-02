#ifndef __SUAS_ARDUINO_H
#define __SUAS_ARDUINO_H

// Porting layer allowing to use the Arduino rfid library on the PineCone
#ifndef __cplusplus
#error "CPP only"
#endif

#include <Serial.h>
#include <SPI.h>
#include <cstdint>
#include <cstring>

extern "C" {
    #include <FreeRTOS.h>
    #include <task.h>
    #include <bl_timer.h>
}

// Arduino types
typedef uint8_t byte;
typedef bool    boolean;

// Progmem compatibility
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *) (addr))

// Flash strings
#define __FlashStringHelper char
#define F(x) (x)

//MSB/LSB
#define LSBFIRST 0
#define MSBFIRST 1

// IO

// High/Low
#define HIGH 1
#define LOW 0

#define INPUT 0x01
#define OUTPUT 0x02
#define PULLUP 0x04
#define PULLDOWN 0x08

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);

// Timing
#define delay(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#define delayMicroseconds(us) bl_timer_delay_us(us)
#define millis() ((xTaskGetTickCount() * 1000) / configTICK_RATE_HZ)
#define yield() portYIELD()

#endif