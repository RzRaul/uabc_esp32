#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <time.h>

#define COIN_1 GPIO_NUM_23
#define COIN_5 GPIO_NUM_19
#define COIN_10 GPIO_NUM_18
#define COIN_20 GPIO_NUM_5
#define LED_1 GPIO_NUM_13
#define LED_2 GPIO_NUM_14
#define LED_3 GPIO_NUM_26
#define LED_4 GPIO_NUM_33
#define LED_CONFIRM GPIO_NUM_2
#define DEBOUNCE_TIME 100
#define PARKING_COST 15
#define COINS 4
#define MAX_COIN 20

static SemaphoreHandle_t g_mutex;

typedef enum {
  START,
  IDLE,
  RECEIVING_COINS,
  VALIDATING,
  RETURNING_CHANGE,
  INVALID
}machine_states_t;

typedef struct {
    machine_states_t state;
    uint8_t receivedMoney;
    bool valid;
    uint8_t coinResevoir[MAX_COIN];
}state_controller_t;

static state_controller_t g_state = {
    .state = START,
    .receivedMoney = 0,
    .valid = false
};

clock_t last_interrupt_time = 0;

typedef struct {
    int pinNumber;
    clock_t time;
}btn_controller_t;

enum btn_state{
    PRESSED,
    RELEASED
};
QueueHandle_t handlerQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *arg){
    uint32_t gpio_num = (uint32_t) arg;
    if (g_state.state == IDLE || g_state.state == RECEIVING_COINS){
        last_interrupt_time = clock();
        g_state.state = VALIDATING;
        return;
    }
    if((clock() - last_interrupt_time) < DEBOUNCE_TIME){
        return;
    }
    if(gpio_get_level(gpio_num) == IDLE && g_state.state == VALIDATING){
        xQueueSendFromISR(handlerQueue,  &gpio_num, NULL);
        g_state.state = RECEIVING_COINS;
    }
    return;
}

void returnCoin(uint8_t coin){
    uint8_t ledN = 2;
    printf("Returned $%d\n", coin);
    if (coin == 20){
        ledN = LED_4;
    }else if (coin == 10){
        ledN = LED_3;
    }else if (coin == 5){
        ledN = LED_2;
    }else if (coin == 1){
        ledN = LED_1;
    }
    gpio_set_level(ledN, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(ledN, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void startOver(){
    g_state.receivedMoney = 0;
    g_state.state = IDLE;
    g_state.valid = false;
    gpio_set_level(LED_CONFIRM, 0);
}


void addToInventoryTask(void *params){
    int pinNumber, valor;
    while(true){
        if(xQueueReceive(handlerQueue, &pinNumber, portMAX_DELAY)){
            switch (pinNumber){
            case COIN_1:
                valor = 1;
                break;
            case COIN_5:
                valor = 5;
            break;
            case COIN_10:
                valor = 10;
            break;
            case COIN_20:
                valor = 20;
            break;
            default:
                valor= 0;
                break;
            }
            if (xSemaphoreTake(g_mutex, 0) == pdTRUE){
                g_state.receivedMoney += valor;
                g_state.coinResevoir[valor]++;
                printf("Inserted coin from btn %d. Received $%d. Total = %d\n", pinNumber, valor, g_state.receivedMoney);
                if (g_state.receivedMoney >= PARKING_COST){
                    g_state.valid = true;
                }
                //Give mutex to release resources
                xSemaphoreGive(g_mutex);
            }else{
               printf("Machine busy, returning inserted coin..."); 
               //return coin, symbolically
            }
            
        }
    }
}

void setupEverything(){
    g_mutex = xSemaphoreCreateMutex();
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << COIN_1) | (1ULL << COIN_5) | (1ULL << COIN_10) | (1ULL << COIN_20);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_1) | (1ULL << LED_2) | (1ULL << LED_3) | (1ULL << LED_4)| (1ULL << LED_CONFIRM);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);
    
    gpio_set_level(LED_1, 0);
    gpio_set_level(LED_2, 0);
    gpio_set_level(LED_3, 0);
    gpio_set_level(LED_4, 0);
    gpio_set_level(LED_CONFIRM, 0);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(COIN_1, gpio_interrupt_handler, (void *)COIN_1);
    gpio_isr_handler_add(COIN_5, gpio_interrupt_handler, (void *)COIN_5);
    gpio_isr_handler_add(COIN_10, gpio_interrupt_handler, (void *)COIN_10);
    gpio_isr_handler_add(COIN_20, gpio_interrupt_handler, (void *)COIN_20);

    g_state.state = IDLE;
    g_state.receivedMoney = 0;
    g_state.valid = false;
    //Initialize coin resevoir with 10 coins of each value
    //Kinda hash table
    for (int i = 0; i < MAX_COIN; i++){
        g_state.coinResevoir[i] = 0;
    }
    g_state.coinResevoir[1] = 10;
    g_state.coinResevoir[5] = 10;
    g_state.coinResevoir[10] = 10;
    g_state.coinResevoir[20] = 10;

}

void app_main(void){
    setupEverything();

    handlerQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(addToInventoryTask, "MoneyCatcherTask", 2048, NULL, 1, NULL);
  
    int devol = 0;
    while(true){
        if (g_state.valid){
            if(xSemaphoreTake(g_mutex,0) == pdTRUE){
                devol = g_state.receivedMoney - PARKING_COST;

                g_state.state = RETURNING_CHANGE;
                gpio_set_level(LED_CONFIRM, 1);
                while (devol > 0){
                        if (devol >= 20){
                            g_state.coinResevoir[20]--;
                            returnCoin(20);
                            devol -= 20;
                        }else if (devol >=10){
                            g_state.coinResevoir[10]--;
                            returnCoin(10);
                            devol -= 10;
                        }else if (devol >=5){
                            g_state.coinResevoir[5]--;
                            returnCoin(5);
                            devol -= 5;
                        }else if (devol >=1){
                            g_state.coinResevoir[1]--;
                            returnCoin(1);
                            devol -= 1;
                        }
                }
                printf("Gracias por visitarnos...\n");
                    vTaskDelay(1500 / portTICK_PERIOD_MS);
                startOver();
                xSemaphoreGive(g_mutex);
            }
        }else{
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

