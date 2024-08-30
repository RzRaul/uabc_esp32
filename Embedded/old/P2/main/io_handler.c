#include <driver/gpio.h>
#include <inttypes.h>
#include "esp_adc/adc_oneshot.h"
#include "io_handler.h"

void setupPushButton(int pin, void *callback(void *)) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, callback, (void *) pin);
}


void setupADC(t_adc_config* config) {
    ESP_ERROR_CHECK(adc_oneshot_new_unit(config->unit_config, config->handle));
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = *config->bitwidth,
        .atten = *config->atten,
    };
    for (int i = 0; i < config->quant; i++) {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(*config->handle, config->channels[i], &channel_config));
    }
}

int readAvgADC(adc_oneshot_unit_handle_t handle, adc_channel_t channel, int samples) {
    int sum = 0;
    for (int i = 0; i < samples; i++) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &raw));
        sum += raw;
    }
    return sum / samples;
}

void setupMultipleLeds(int *leds, int length) {
    for (int i = 0; i < length; i++) {
        setupLed(leds[i]);
    }
}
void setupLed(int pin) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void turnOnLed(int pin) {
    gpio_set_level(pin, 0);
}

void turnOffLed(int pin) {
    gpio_set_level(pin, 1);
}

void updateLeds(int value, int *leds, int length) {
    for (int i = 0; i < length; i++) {
        if (i<value) {
            turnOnLed(leds[i]);
        } else {
            turnOffLed(leds[i]);
        }
    }
    
}

