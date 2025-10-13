#include <bl_gpio.h>       // GPIO HAL
#include <bl_timer.h>      // Timer HAL
#include <stdio.h>         // For IO operations
#include <suas_74hc595.h>  // Header

#define timer_delay() bl_timer_delay_us(10)

/*
    Configure connected shift register(s).
    Must be called first
*/
void suas_74hc595_config(suas_74hc595_conf_t *config) {
  /* Configure pins as output */
  if (config->with_output_enable) {
    bl_gpio_enable_output(config->pin_output_enable, 0, 0);
  }
  if (config->with_master_reset) {
    bl_gpio_enable_output(config->pin_master_reset, 0, 0);
  }

  bl_gpio_enable_output(config->pin_data_signal, 0, 0);
  bl_gpio_enable_output(config->pin_shift_clock, 0, 0);
  bl_gpio_enable_output(config->pin_store_clock, 0, 0);

  /* Set initial values */
  bl_gpio_output_set(config->pin_data_signal, 0);
  bl_gpio_output_set(config->pin_shift_clock, 0);
  bl_gpio_output_set(config->pin_store_clock, 0);

  /* Mark as initialized */
  config->initialized = 1;

  /* Clear */
  suas_74hc595_clear(config);
}

/*
    Store a byte in the shift register.
    Only works after suas_74hc595_config() was called.
*/
void suas_74hc595_store(suas_74hc595_conf_t *config, uint8_t data) {
  // 1. Check if initialized
  if (!config->initialized) {
    printf("74hc595 is not yet initialized\r\n");
    return;
  }

  /* 2. Shift out data */
  for (uint8_t i = 0; i < 8; i++) {
    /* Extract LSB */
    uint8_t lsb = (0x1 & (data >> i));

    /* Store bit */
    bl_gpio_output_set(config->pin_data_signal, lsb);
    bl_gpio_output_set(config->pin_shift_clock, 1);
    timer_delay();
    bl_gpio_output_set(config->pin_shift_clock, 0);
  }

  /* Copy data to output register */
  bl_gpio_output_set(config->pin_store_clock, 1);
  timer_delay();
  bl_gpio_output_set(config->pin_store_clock, 0);

  /* Enable output */
  if (config->with_output_enable) {
    bl_gpio_output_set(config->pin_output_enable, 0);
  }
}

/*
    Clear the output of the registers.
    Only works after suas_74hc595_config() was called.
*/
void suas_74hc595_clear(suas_74hc595_conf_t *config) {
  // 1. Check if initialized
  if (!config->initialized) {
    printf("74hc595 is not yet initialized\r\n");
    return;
  }

  // 2. Clear output
  if (config->with_master_reset) {
    bl_gpio_output_set(config->pin_master_reset, 0);
    timer_delay();
    bl_gpio_output_set(config->pin_master_reset, 1);
  } else {
    for (uint8_t i = 0; i < config->number_of_registers; i++) {
      suas_74hc595_store(config, 0);
    }
  }

  if (config->with_output_enable) {
    bl_gpio_output_set(config->pin_output_enable, 1);
  }
}