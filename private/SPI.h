#ifndef __SUAS_SPI_H
#define __SUAS_SPI_H

// Porting layer allowing to use the Arduino rfid library on the PineCone
#ifndef __cplusplus
#error "CPP only"
#endif

#include <cstdint>

// Chip select pin
#ifndef SS
#define SS 2
#endif

// MOSI
#ifndef MOSI
#define MOSI 1
#endif

// MISO
#ifndef MISO
#define MISO 4
#endif

// SCK
#ifndef SCK
#define SCK 3
#endif

#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

#define SPI_LSBFIRST 0
#define SPI_MSBFIRST 1

class SPISettings {
public:
  SPISettings() : clock(1000000), bitOrder(SPI_MSBFIRST), dataMode(SPI_MODE0) {}
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) : clock(clock), bitOrder(bitOrder), dataMode(dataMode) {}
private:
  uint32_t clock;
  uint8_t bitOrder;
  uint8_t dataMode;
  friend class SPIClass;
};

class SPIClass {
public:
  void begin();
  void beginTransaction(SPISettings settings);
  void setFrequency(uint32_t frequency);
  uint8_t transfer(uint8_t data);
  void endTransaction(void);

private:
  bool inTransaction = false;
  uint32_t currentFrequency = 0;
  friend void spiInterrupt(void* ctx);
};

extern SPIClass SPI;

#endif /* __SUAS_SPI_H */