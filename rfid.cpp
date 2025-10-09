#include <cstdio>
#include <cstdint>
#include <MFRC522.h>

extern "C" {
    #include <suas_rfid.h>
}

// Porting layer allowing to use the Arduino rfid library on the PineCone
static MFRC522 rfid(SS, 5);
extern SerialClass Serial;
extern SPIClass SPI;

// Initialize chip
extern "C" void suas_rfid_init() {
    Serial.begin(9600);
    Serial.println("Initializing RFID module (1/2)");
    SPI.begin();
    Serial.println("Initializing RFID module (2/2)");
    rfid.PCD_Init();
    Serial.println("Initialized RFID module");
}

// Check if new card is present
extern "C" uint8_t suas_rfid_is_new_card_present() {
    return rfid.PICC_IsNewCardPresent();
}

// Read card
extern "C" bool suas_rfid_read_card() {
    return rfid.PICC_ReadCardSerial();
}

// Get UID from card
extern "C" suas_rfc_uid_t suas_rfid_get_uid() {
    suas_rfc_uid_t retval;
    retval.size = rfid.uid.size;
    memcpy(retval.uid_byte, rfid.uid.uidByte, rfid.uid.size);
    return retval;
}