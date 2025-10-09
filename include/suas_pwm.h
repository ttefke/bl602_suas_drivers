#ifndef __SUAS_PWM_H
#define __SUAS_PWM_H

// Support for all integer types
#include <inttypes.h>

// Data structure to manage all required information
typedef struct suas_pwm_conf_t {
    uint8_t channel: 3; /* PWM channel*/
    uint8_t pin:     5; /* GPIO pin*/
    uint8_t initialized: 1; /* Whether the configuration is initialized*/
    uint32_t frequency: 23; /* Frequency (must be between 2,000 and 800,000, can be adjusted with the divider);
                              20 bits are sufficient but half bytes can not be used anyway */
    uint16_t frequency_divider; /* Frequency divider*/
    float duty; /* Duty cycle*/
} suas_pwm_conf_t;

int8_t suas_pwm_init(suas_pwm_conf_t *conf);
int8_t suas_pwm_start(suas_pwm_conf_t *conf);
int8_t suas_pwm_stop(suas_pwm_conf_t *conf);
int8_t suas_pwm_set_freq(suas_pwm_conf_t *conf);
int8_t suas_pwm_set_duty(suas_pwm_conf_t *conf);
int8_t suas_pwm_get_duty(suas_pwm_conf_t *conf);
#endif
