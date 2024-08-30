#include <driver/gpio.h>
#include <inttypes.h>
#include "esp_adc/adc_oneshot.h"

void setupPushButton(int pin, void*(void*)) ;
//It takes the arrays of channels, the length of the arrays and the handle of the ADC unit to configure the channels
typedef struct{
    int quant;
    adc_channel_t* channels;
    adc_atten_t* atten;
    adc_bitwidth_t* bitwidth;
    adc_oneshot_unit_init_cfg_t* unit_config;
    adc_oneshot_unit_handle_t* handle;
}t_adc_config;

int readAvgADC(adc_oneshot_unit_handle_t handle, adc_channel_t channel, int samples) ;
void setupMultipleLeds(int *leds, int length) ;
void setupADC(t_adc_config* config);
void setupLed(int pin);
void turnOnLed(int pin) ;
void turnOffLed(int pin) ;
void updateLeds(int value, int *leds, int length) ;
void pushIntoQ(int *q, int length, int value) ;
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);
