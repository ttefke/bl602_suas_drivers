#include <MFRC522.h>

extern "C" {
#include <stdint.h>
#include <suas_rfid.h>
}

// Porting layer allowing to use the Arduino rfid library on the PineCone
static MFRC522 rfid(SS, 5);
static uint8_t nr_of_scans = 0;

extern SerialClass Serial;
extern SPIClass SPI;

// Initialize chip
extern "C" void suas_rfid_init() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

// Check if new card is present
extern "C" uint8_t suas_rfid_is_new_card_present() {
  /* Reset frequently to avoid some glitches */
  if (nr_of_scans == 127) {
    nr_of_scans = 0;
    suas_rfid_init();
  }
  return rfid.PICC_IsNewCardPresent();
}

// Read card
extern "C" bool suas_rfid_read_card() { return rfid.PICC_ReadCardSerial(); }

// Get UID from card
extern "C" suas_rfid_uid_t suas_rfid_get_uid() {
  suas_rfid_uid_t retval;
  retval.size = rfid.uid.size;
  memcpy(retval.uid_byte, rfid.uid.uidByte, rfid.uid.size);
  return retval;
}