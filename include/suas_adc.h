// Header guard begin
#ifndef __SUAS_ADC_H__
#define __SUAS_ADC_H__ 

// ADC libraries
#include <bl_adc.h>     //  ADC HAL

// Configuration data type
typedef struct adc_conf_t {
    uint8_t  pin;
    uint8_t  conversion_mode:   1;
    uint8_t  number_of_samples: 7;
    uint16_t frequency:        14;
    uint8_t  initialized:       1;
} adc_conf_t;

// Define conversion modes
// 40Hz - 1300Hz
#define NORMAL_ADC_CONVERSION_MODE 0

// 500Hz - 16000 Hz
#define MICROPHONE_ADC_CONVERSION_MODE 1

// Give this function a second name which
// has a name that suits our use case better
#define adc_channel_exists(x) bl_adc_get_channel_by_gpio(x)

// Function prototypes
int init_adc(adc_conf_t *conf);
uint32_t read_adc(adc_conf_t *conf);

// Header guard end
#endif