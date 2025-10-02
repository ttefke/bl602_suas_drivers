#ifndef __SUAS_RFID_H
#define __SUAS_RFID_H

// Data structure to get the uid from the card
typedef struct uid_t {
    uint8_t size;
    uint8_t uid_byte[10];
} uid_t;

// Function prototypes
void suas_rfid_init();
uint8_t suas_rfid_is_new_card_present();
bool suas_rfid_read_card();
uid_t suas_get_uid();

#endif