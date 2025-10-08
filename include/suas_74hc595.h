#ifndef SUAS_74HC595_H
#define SUAS_74HC595_H

#include <stdint.h>

struct config_74hc595 {
    /* Obligatory pins */
    uint8_t pin_data_signal: 5;
    uint8_t pin_shift_clock: 5;
    uint8_t pin_store_clock: 5;

    /* Optional pins*/
    uint8_t with_output_enable: 1;
    uint8_t pin_output_enable:  5;

    uint8_t with_master_reset: 1;
    uint8_t pin_master_reset: 5;

    /* Number of registers for cascade */
    uint8_t number_of_registers: 5;
};

void suas_configure_74hc595(struct config_74hc595 *config);

void suas_store_data_74hc595(struct config_74hc595 *config, uint8_t data);

void suas_clear_74hc595(struct config_74hc595 *config);

#endif
