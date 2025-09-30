// Input/output library
#include <stdio.h>

// Use all integer types
#include <inttypes.h>

// ADC libraries
#include <bl602_adc.h>  //  ADC driver
#include <bl_adc.h>     //  ADC HAL
#include <bl_dma.h>     //  DMA HAL

#include "include/suas_adc.h"        // Our own header

static void set_adc_gain(uint32_t gain1, uint32_t gain2) {
    // Read configuration hardware register
    uint32_t reg = BL_RD_REG(AON_BASE, AON_GPADC_REG_CONFIG2);
    
    // Set ADC gain bits
    reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_PGA1_GAIN, gain1);
    reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_PGA2_GAIN, gain2);
    
    // Set chop mode
    if (gain1 != ADC_PGA_GAIN_NONE || gain2 != ADC_PGA_GAIN_NONE) {
        reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_CHOP_MODE, 2);
    } else {
        reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_CHOP_MODE, 1);
    }
    
    // Enable ADC amplifier
    reg = BL_CLR_REG_BIT(reg, AON_GPADC_PGA_VCMI_EN);
    if (gain1 != ADC_PGA_GAIN_NONE || gain2 != ADC_PGA_GAIN_NONE) {
        reg = BL_SET_REG_BIT(reg, AON_GPADC_PGA_EN);
    } else {
        reg = BL_CLR_REG_BIT(reg, AON_GPADC_PGA_EN);
    }
    
    // Update ADC configuration hardware register
    BL_WR_REG(AON_BASE, AON_GPADC_REG_CONFIG2, reg);
}

// NOTE: pin must be of the following 4, 5, 6, 9, 10, 11, 12, 13, 14, 15
// Otherwise you may damage your device!
int init_adc(adc_conf_t *conf) {
    // Ensure a valid pin was selected
    if (adc_channel_exists(conf->pin) == -1) {
        printf("Invalid pin selected for ADC\r\n");
        return -1;
    }

    if (conf->conversion_mode == NORMAL_ADC_CONVERSION_MODE) {
        if ((conf->frequency < 40) || (conf->frequency > 1400)) {
            printf("Selected frequency does not match for normal ADC conversion mode!\r\n");
            return -1;
        }
    } else if (conf->conversion_mode == MICROPHONE_ADC_CONVERSION_MODE) {
        if ((conf->frequency < 500) || (conf->frequency > 16000)) {
            printf("Selected frequency does not match for microphone ADC conversion mode!\r\n");
            return -1;
        }
    }
    
    // Set frequency and single channel conversion mode
    bl_adc_freq_init(conf->conversion_mode, conf->frequency); // always returns 0 -> nothing to evaluate
    
    // Initialize GPIO pin for single channel conversion mode
    bl_adc_init(conf->conversion_mode, conf->pin); // always returns 0 -> nothing to evaluate
    
    // Enable ADC gain
    set_adc_gain(ADC_PGA_GAIN_1, ADC_PGA_GAIN_1); // returns nothing

    // Initialize DMA for the ADC channel and for single channel conversion mode
    int result = bl_adc_dma_init(conf->conversion_mode, conf->number_of_samples);
    if (result != 0) {
        printf("Error occurred during DMA initialization\r\n");
        return result;
    }

    // Configure GPIO pin as ADC input
    bl_adc_gpio_init(conf->pin); // always returns 0

    // Mark pin and ADC as configured
    int channel = bl_adc_get_channel_by_gpio(conf->pin); // returns -1 if channel does not exist but we already tested for that
    adc_ctx_t *ctx = bl_dma_find_ctx_by_channel(ADC_DMA_CHANNEL);
    ctx -> chan_init_table |= (1 << channel);

    // Start reading process
    bl_adc_start(); // always returns 0

    conf->initialized = 1;
    return 0;
}

uint32_t read_adc(adc_conf_t *conf) {
    // Check if initialized
    if (!conf->initialized) {
        printf("ADC is not initialized!\r\n");
        return 0;
    }

    // Array storing samples statically
    uint32_t adc_data[conf->number_of_samples];
    
    // Get DMA context for ADC channel to read data
    adc_ctx_t *ctx = bl_dma_find_ctx_by_channel(ADC_DMA_CHANNEL);
    
    // Return if sampling failed or did not finish
    if (ctx -> channel_data == NULL) {
        return 0;
    }
    
    // Copy read samples from memory written dynamically by DMA to static array
    uint32_t received_number_of_samples =
        ctx->data_size <= sizeof(adc_data) ? ctx->data_size : sizeof(adc_data);
    
    memcpy(
        (uint8_t*) adc_data, /* destination: adc_data array */
        (uint8_t*) (ctx -> channel_data), /* source: channel_data element of ctx struct */
        received_number_of_samples  /* amount of bytes to be copied */
    );

    // Calculate mean value
    uint32_t mean = 0;
    for (uint32_t i = 0; i < received_number_of_samples; i++) {
        mean += adc_data[i];
    }

    mean /= received_number_of_samples;

    // Scale down mean and return
    uint32_t scaled_mean = ((mean & 0xFFFF) * 4096) >> 16;  
    return scaled_mean;
}