#ifndef SUAS_74HC595_H
#define SUAS_74HC595_H

#include <stdint.h>

typedef struct suas_74hc595_conf_t {
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
    uint8_t number_of_registers: 4;

    /* Initialized indicator */
    uint8_t initialized: 1;
} suas_74hc595_conf_t;

void suas_74hc595_config(suas_74hc595_conf_t *config);
void suas_74hc595_store(suas_74hc595_conf_t *config, uint8_t data);
void suas_74hc595_clear(suas_74hc595_conf_t *config);
#endif
