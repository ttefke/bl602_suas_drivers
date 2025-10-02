#include <bl_gpio.h>  // GPIO HAL
#include <bl_timer.h> // Timer HAL
#include <74hc595.h>  // Header

#define timer_delay() \
    bl_timer_delay_us(10)

/*
    Configure connected shift register(s).
    Must be called first
*/
void suas_configure_74hc595(struct config_74hc595 *config)
{
    /* Configure pins as output */
    if (config->with_output_enable)
    {
        bl_gpio_enable_output(config->pin_output_enable, 0, 0);
    }
    if (config->with_master_reset)
    {
        bl_gpio_enable_output(config->pin_master_reset, 0, 0);
    }

    bl_gpio_enable_output(config->pin_data_signal, 0, 0);
    bl_gpio_enable_output(config->pin_shift_clock, 0, 0);
    bl_gpio_enable_output(config->pin_store_clock, 0, 0);

    /* Set initial values */
    bl_gpio_output_set(config->pin_data_signal, 0);
    bl_gpio_output_set(config->pin_shift_clock, 0);
    bl_gpio_output_set(config->pin_store_clock, 0);

    suas_clear_74hc595(config);
}

/*
    Store a byte in the shift register.
    Only works after configure_74hc595() was called.
*/
void suas_store_data_74hc595(struct config_74hc595 *config, uint8_t data)
{
    /* Shift out data */
    for (uint8_t i = 0; i < 8; i++)
    {
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
    if (config->with_output_enable)
    {
        bl_gpio_output_set(config->pin_output_enable, 0);
    }
}

/*
    Clear the output of the registers.
    Only works after configure_74hc595() was called.
*/
void suas_clear_74hc595(struct config_74hc595 *config)
{
    if (config->with_master_reset)
    {
        bl_gpio_output_set(config->pin_master_reset, 0);
        timer_delay();
        bl_gpio_output_set(config->pin_master_reset, 1);
    }
    else
    {
        for (uint8_t i = 0; i < config->number_of_registers; i++)
        {
            suas_store_data_74hc595(config, 0);
        }
    }

    if (config->with_output_enable)
    {
        bl_gpio_output_set(config->pin_output_enable, 1);
    }
}