#include "my_main.h"

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
    printf("Returned $%d", coin);
    //Turn on leds
}

void startOver(){
    g_state.receivedMoney = 0;
    g_state.state = IDLE;
    g_state.valid = false;
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
    gpio_reset_pin(LED_1);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);

    gpio_set_direction(BTN1, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN1);
    gpio_pulldown_dis(BTN1);
    gpio_set_intr_type(BTN1, GPIO_INTR_ANYEDGE);

}

void app_main(void){
    setupEverything();

    handlerQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(addToInventoryTask, "MoneyCatcherTask", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN1, gpio_interrupt_handler, (void *)BTN1);
  
    int devol = 0;
    while(true){
        if (g_state.valid){
            if(xSemaphoreTake(g_mutex,0) == pdTRUE){
                devol = g_state.receivedMoney - PARKING_COST;
                if (devol > 0){
                    g_state.state = RETURNING_CHANGE;
                    while(devol > 0){
                        if (devol >= 20){
                            g_state.coinResevoir[20]--;
                            returnCoin(20);
                        }else if (devol >=10){
                            g_state.coinResevoir[10]--;
                            returnCoin(10);
                        }else if (devol >=5){
                            g_state.coinResevoir[5]--;
                            returnCoin(5);
                        }else if (devol >=1){
                            g_state.coinResevoir[1]--;
                            returnCoin(1);
                        }
                    }
                }
                printf("Gracias por visitarnos...\n");
                startOver();
                xSemaphoreGive(g_mutex);
            }
        }
    }
}