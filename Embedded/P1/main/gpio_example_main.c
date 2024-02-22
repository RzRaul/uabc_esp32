#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define SWITCH_1 GPIO_NUM_25
#define SWITCH_2 GPIO_NUM_33
#define SWITCH_3 GPIO_NUM_32
#define SWITCH_4 GPIO_NUM_35
#define SWITCH_5 GPIO_NUM_34
#define SWITCH_QUANTITY 5


#define LED_1 GPIO_NUM_23
#define LED_2 GPIO_NUM_22
#define LED_3 GPIO_NUM_21
#define LED_4 GPIO_NUM_19
#define LED_5 GPIO_NUM_18
#define LED_QUANTITY 5

static char tag[] = "adc1";

int leds[LED_QUANTITY] = {LED_1, LED_2, LED_3, LED_4, LED_5};
int switches[SWITCH_QUANTITY] = {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4, SWITCH_5};

static QueueHandle_t gpio_evt_queue = NULL;

typedef struct {
  int direction;
  int delayMS;
} t_AnimationController;

t_AnimationController animationController={
  .direction = 1,
  .delayMS = 1000
};


gpio_config_t configIO;
void setUpOutputs(void){
    configIO.pin_bit_mask = (1ULL<<LED_1) | (1ULL<<LED_2) | (1ULL<<LED_3) | (1ULL<<LED_4) | (1ULL<<LED_5);
    configIO.mode = GPIO_MODE_OUTPUT;
    configIO.pull_up_en = GPIO_PULLUP_ENABLE;
    configIO.pull_down_en = GPIO_PULLDOWN_DISABLE;
    configIO.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&configIO);
}

void IRAM_ATTR gpio_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void setupInterrupts(void){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL<<SWITCH_1) | (1ULL<<SWITCH_2) | (1ULL<<SWITCH_3) | (1ULL<<SWITCH_4) | (1ULL<<SWITCH_5);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(SWITCH_1, gpio_isr_handler, (void*) SWITCH_1);
    gpio_isr_handler_add(SWITCH_2, gpio_isr_handler, (void*) SWITCH_2);
    gpio_isr_handler_add(SWITCH_3, gpio_isr_handler, (void*) SWITCH_3);
    gpio_isr_handler_add(SWITCH_4, gpio_isr_handler, (void*) SWITCH_4);
    gpio_isr_handler_add(SWITCH_5, gpio_isr_handler, (void*) SWITCH_5);
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
}

int getNewDelay(){
    int ones=0;
    for(int i=1; i<SWITCH_QUANTITY;i++)
        ones+=!gpio_get_level(switches[i]);

    switch(ones){
        case 1:
            return 500;
        case 2:
            return 250;
        case 3:
            return 125;
        case 4:
            return 62;
        default:
            return 1000;
    }
}

void interruptHandler(void* arg){
    int io_num;
    for(;;){
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){
            if(io_num == SWITCH_1){
                animationController.direction *= -1;
            } else {
                animationController.delayMS = getNewDelay();
            }
        }
    }
}



void app_main() {
    setUpOutputs();
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    setupInterrupts();
    xTaskCreate(interruptHandler, "interruptHandler", 2048, NULL, 10, NULL);
    int i=0;
    while(1){
        gpio_set_level(leds[i], 1);
        vTaskDelay(animationController.delayMS / portTICK_PERIOD_MS);
        gpio_set_level(leds[i], 0);
        vTaskDelay(animationController.delayMS / portTICK_PERIOD_MS);
        i += animationController.direction;
        if(i == LED_QUANTITY){
            i = 0;
        } else if(i == -1){
            i = LED_QUANTITY - 1;
        }
    }
}




