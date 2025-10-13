// Standard input/output
#include <stdio.h>

// BouffaloLab PWM HAL
#include <bl602_pwm.h>
#include <bl_pwm.h>
#include <suas_pwm.h>

// Initialize PWM
int8_t suas_pwm_init(suas_pwm_conf_t *conf) {
  // 1. Get channel for pin -> see manual p. 27f.
  conf->channel = (conf->pin % 5);

  // 2. Initialize channel
  int8_t retval = bl_pwm_init(conf->channel, conf->pin, conf->frequency);

  // 3. Set frequency divider
  PWM_Channel_Set_Div(conf->channel, conf->frequency_divider);

  // 4. Mark as initialized
  conf->initialized = 1;

  // 5. Return
  return retval;
}

// Start PWM
int8_t suas_pwm_start(suas_pwm_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("PWM is not yet initialized\r\n");
    return -1;
  }

  // 2. Start PWM
  return bl_pwm_start(conf->channel);
}

// Stop PWM
int8_t suas_pwm_stop(suas_pwm_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("PWM is not yet initialized\r\n");
    return -1;
  }

  // 2. Stop PWM
  return bl_pwm_stop(conf->channel);
}

// Set a new frequency
int8_t suas_pwm_set_freq(suas_pwm_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("PWM is not yet initialized\r\n");
    return -1;
  }

  // 2. Set frequency
  int8_t retval = bl_pwm_set_freq(conf->channel, conf->frequency);

  // 3. Set frequency divider
  PWM_Channel_Set_Div(conf->channel, conf->frequency_divider);

  // 4. Return
  return retval;
}

// Set a duty cycle
int8_t suas_pwm_set_duty(suas_pwm_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("PWM is not yet initialized\r\n");
    return -1;
  }

  // 2. Set duty
  return bl_pwm_set_duty(conf->channel, conf->duty);
}

// Get current duty
int8_t suas_pwm_get_duty(suas_pwm_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("PWM is not yet initialized\r\n");
    return -1;
  }

  // 2. Get duty
  return bl_pwm_get_duty(conf->channel, &conf->duty);
}