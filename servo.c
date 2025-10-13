#include <stdio.h>       // IO
#include <suas_servo.h>  // Own header

// Helper function to compute the duty cycle
// Please note: this is an approximation which can be quite odd depending on the
// servo used
static float suas_servo_compute_duty_cycle(uint8_t angle) {
  // Only accept angles up to 180 degrees
  if (angle > 180) {
    angle = 180;
  }

  // Compute duty cycle
  return (3.0f + (angle / 36.0f) * 3.5f) / 2.0f;
}

// Initialize a servo
void suas_servo_init(suas_servo_conf_t *conf) {
  // 1. Set PWM configuration
  suas_pwm_conf_t pwm_conf = {
      .pin = conf->pin,
      .frequency = 2000,
      .frequency_divider = 40,  // frequency = 50Hz
      .duty = suas_servo_compute_duty_cycle(conf->degrees)};
  conf->pwm_conf = pwm_conf;

  // 2. Initialize PWM
  suas_pwm_init(&conf->pwm_conf);

  // 2. Start PWM
  suas_pwm_start(&conf->pwm_conf);

  // 4. Set initial duty
  suas_pwm_set_duty(&conf->pwm_conf);

  // 5. Mark as initialized
  conf->initialized = 1;
}

// Set the degrees of a servo
void suas_servo_set_degrees(suas_servo_conf_t *conf) {
  // 1. Check if initialized
  if (!conf->initialized) {
    printf("Servo is not initialized!\r\n");
    return;
  }

  // 2. Compute duty cycle
  conf->pwm_conf.duty = suas_servo_compute_duty_cycle(conf->degrees);

  // 3. Set new duty cycle
  suas_pwm_set_duty(&conf->pwm_conf);
}