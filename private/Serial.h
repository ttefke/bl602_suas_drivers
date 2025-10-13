#ifndef __SUAS_SERIAL_H
#define __SUAS_SERIAL_H

// Porting layer allowing to use the Arduino rfid library on the PineCone
#ifndef __cplusplus
#error "CPP only"
#endif

#include <cstddef>
#include <cstdint>

// Encodings
#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

class SerialClass {
 public:
  void begin(unsigned long baud);
  size_t print(const char[]);
  size_t print(int, int = DEC);

  size_t println(const char[]);
  size_t println(int, int = DEC);
  size_t println(void);
};

extern SerialClass Serial;
#endif