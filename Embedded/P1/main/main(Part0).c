#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

#define BTN1 GPIO_NUM_23
#define LED GPIO_NUM_2
#define DEBOUNCE_TIME 100
int state = 0;

clock_t last_interrupt_time = 0;

typedef struct {
    int pinNumber;
    clock_t time;
}btn_controller_t;

enum btn_state{
    IDLE,
    PRESSED,
    RELEASED
};
QueueHandle_t handlerQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *arg){
    uint32_t gpio_num = (uint32_t) arg;
    if (state == IDLE){
        last_interrupt_time = clock();
        state = PRESSED;
        gpio_set_level(LED, 1);
    }
    if((clock() - last_interrupt_time) < DEBOUNCE_TIME){
        return;
    }
    if(1){
        state = IDLE;
        xQueueSendFromISR(handlerQueue,  &gpio_num, NULL);
    }
}

void LED_Control_task(void *params){
    int pinNumber, count = 0;
    while(true){
        if(xQueueReceive(handlerQueue, &pinNumber, portMAX_DELAY)){
            printf("GPIO %d pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BTN1));
            gpio_set_level(LED, state);
        }
    }
}

void app_main(void){
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    gpio_set_direction(BTN1, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN1);
    gpio_pulldown_dis(BTN1);
    gpio_set_intr_type(BTN1, GPIO_INTR_ANYEDGE);

    handlerQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(LED_Control_task, "LED_Control_task", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN1, gpio_interrupt_handler, (void *)BTN1 );
  
}