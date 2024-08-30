#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

#define BTN1 GPIO_NUM_23
#define BTN2 GPIO_NUM_22
#define BTN3 GPIO_NUM_21
#define LED1 GPIO_NUM_5
#define DEBOUNCE_TIME 200

void reading_inputs(){

}
void task_reading_with_debounce(void *pvParameter){
    int *value = (int *)pvParameter;
    int last_value = 1;
    while(1){
        if( gpio_get_level(BTN1) == !last_value){
            vTaskDelay(DEBOUNCE_TIME / portTICK_PERIOD_MS);
            while(gpio_get_level(BTN1) == !last_value){
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            *value = !last_value;
            last_value = !last_value;
            printf("Button pressed\n");
        }
    }
}

void setup_pins(){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << BTN1);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1 << LED1);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void app_main(void){
    setup_pins();
    int value;
    int last_value = 1;
    while(1){
        value = gpio_get_level(BTN1);
        if(!value){
            vTaskDelay(DEBOUNCE_TIME / portTICK_PERIOD_MS);
            while(!gpio_get_level(BTN1)){
                printf("Waiting for button release\n");
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            printf("Button pressed\n");
        }
    }
}
