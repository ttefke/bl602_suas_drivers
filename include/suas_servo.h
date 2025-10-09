#ifndef __SUAS_SERVO_H
#define __SUAS_SERVO_H

// Include PWM functionality
#include <suas_pwm.h>

// Data structure to manage configuration
typedef struct suas_servo_conf_t {
    uint8_t pin: 5;    /* GPIO pin*/
    uint8_t degrees; /* Dregrees to rotate*/
    uint8_t initialized: 1; /* Whether the struct is initialized*/
    suas_pwm_conf_t pwm_conf; /* PWM configuration */
} suas_servo_conf_t;

void suas_servo_init(suas_servo_conf_t *conf);
void suas_servo_set_degrees(suas_servo_conf_t *conf);

#endif