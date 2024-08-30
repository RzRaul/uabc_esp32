#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "io_handler.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include <math.h>

const static char *TAG = "EXAMPLE";
//Normalmente iría en un archivo de configuración, pero los pongo aquí para que sea más sencillo revisar el código
#define ADC1_FIRST_CHANNEL ADC_CHANNEL_0
#define ADC1_SECOND_CHANNEL ADC_CHANNEL_3
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11
#define BETA 3950
#define R0 10000
#define SAMPLES 5
#define ROOMS 3
#define TIME_FOR_MEASURE_MS 1000
#define LATCHING_TEMP 3
#define MAX_LUMENS 10000
#define MIN_LUMENS 0
#define BULBS 4
#define BLINKS 4
#define LED_1 GPIO_NUM_22
#define LED_2 GPIO_NUM_21
#define LED_3 GPIO_NUM_19
#define LED_4 GPIO_NUM_18
#define LED_AC GPIO_NUM_16
#define BUTTON1 GPIO_NUM_32
#define BUTTON2 GPIO_NUM_33

void IRAM_ATTR gpio_isr_handler(void* arg);
void interruptHandler(void *arg);
const float GAMMA = 0.7;
const float RL10 = 50;

static QueueHandle_t gpio_evt_queue = NULL; //Queue to handle the button events
adc_oneshot_unit_handle_t adc1_handle;
t_adc_config adc_config = {
    .quant = 2,
    .channels = (adc_channel_t[]){ADC1_FIRST_CHANNEL, ADC1_SECOND_CHANNEL},
    .atten = (adc_atten_t[]){EXAMPLE_ADC_ATTEN, EXAMPLE_ADC_ATTEN},
    .bitwidth = (adc_bitwidth_t[]){ADC_BITWIDTH_10, ADC_BITWIDTH_10},
    .unit_config = &(adc_oneshot_unit_init_cfg_t){
        .unit_id = ADC_UNIT_1,
    },
    .handle = &adc1_handle,
};

typedef struct {
    int desiredTemp[ROOMS];
    int desiredLumens[ROOMS];
} t_desiredValues;

t_desiredValues desiredValues = {
    .desiredTemp = {25, 40, 60},
    .desiredLumens = {2500, 4500, 9000},
};
typedef struct {
    int isACOn[ROOMS];
    int currentRoom;
} t_ACStatus;

t_ACStatus statusControl = {
    .isACOn = {0, 0, 0},
    .currentRoom = 0,
};

int lumenSteps = (MAX_LUMENS - MIN_LUMENS) / BULBS;

void app_main(void)
{
    float celsius, lux, voltage, resistance;
    int avgReading;
    //------------ Setup --------------//
    setupADC(&adc_config);
    setupMultipleLeds((int[]){LED_1, LED_2, LED_3, LED_4, LED_AC}, 5);
    updateLeds(0, (int[]){LED_1, LED_2, LED_3, LED_4}, 4);
    setupPushButton(BUTTON1, gpio_isr_handler);
    setupPushButton(BUTTON2, gpio_isr_handler);
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(interruptHandler, "interruptHandler", 2048, NULL, 10, NULL);

    //------------ Main loop --------------//
    while (1) {
        //----------------- Temperature -----------------//
        avgReading = readAvgADC(adc1_handle, ADC1_FIRST_CHANNEL, SAMPLES);
        celsius = 1 / (log(1 / (1023. / avgReading - 1)) / BETA + 1.0 / 298.15) - 273.15;
        ESP_LOGI(TAG, "Temperature: %f - Reading: %dC", celsius, avgReading);
        if (celsius < desiredValues.desiredTemp[statusControl.currentRoom] - LATCHING_TEMP) {
            statusControl.isACOn[statusControl.currentRoom] = 1;
            turnOnLed(LED_AC);
        } else if (celsius > desiredValues.desiredTemp[statusControl.currentRoom] + LATCHING_TEMP) {
            statusControl.isACOn[statusControl.currentRoom] = 0;
            turnOffLed(LED_AC);
        }

        //----------------- Luminosity -----------------//
        avgReading = readAvgADC(adc1_handle, ADC1_SECOND_CHANNEL, SAMPLES);
        voltage = avgReading / 1024. * 5;
        resistance = 2000 * voltage / (1 - voltage / 5);
        lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
        if (lux < desiredValues.desiredLumens[statusControl.currentRoom]) {
            updateLeds( (int) (desiredValues.desiredLumens[statusControl.currentRoom] - lux) / lumenSteps, (int[]){LED_1, LED_2, LED_3, LED_4}, 4);
        } else {
            updateLeds(0, (int[]){LED_1, LED_2, LED_3, LED_4}, 4);
        }
        printf("Luminosity: %f lux\n", lux);

        vTaskDelay(pdMS_TO_TICKS(TIME_FOR_MEASURE_MS));
    }
}

void IRAM_ATTR gpio_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void showRoom(){
    for (int i = 0; i < BLINKS; i++) {
        updateLeds(statusControl.currentRoom+1, (int[]){LED_1, LED_2, LED_3, LED_4}, 4);
        vTaskDelay(pdMS_TO_TICKS(300));
        updateLeds(0, (int[]){LED_1, LED_2, LED_3, LED_4}, 4);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
}

//The callback function for button 1 changes the room and the other one displays the room number on the leds blink twice with 500 ms in between
void interruptHandler(void *arg){
    int io_num;
    //disables ALL interrupts on the core for the baunce for 70 ms
    portDISABLE_INTERRUPTS();
    vTaskDelay(pdMS_TO_TICKS(70));
    portENABLE_INTERRUPTS();
    
    for(;;){
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)){
            if (io_num == BUTTON1) {
                statusControl.currentRoom = (statusControl.currentRoom + 1) % ROOMS;
                ESP_LOGI(TAG, "Room: %d. Temp: %d. Lumens: %d", statusControl.currentRoom, desiredValues.desiredTemp[statusControl.currentRoom], desiredValues.desiredLumens[statusControl.currentRoom]);
            }else{
                showRoom();
            }
            }
    }
}

