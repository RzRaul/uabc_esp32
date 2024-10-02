#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <time.h>

#define COIN_1 GPIO_NUM_23
#define COIN_5 GPIO_NUM_19
#define COIN_10 GPIO_NUM_25
#define COIN_20 GPIO_NUM_26
#define LED_1 GPIO_NUM_2
#define LED_2 GPIO_NUM_3
#define LED_3 GPIO_NUM_4
#define LED_4 GPIO_NUM_5
#define LED_RECEIPT GPIO_NUM_2
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

typedef enum {
    BAR_LOW,
    BAR_MID,
    BAR_HIGH
}bar_level_t;

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
    IDLE,
    RELEASED
};
QueueHandle_t handlerQueue;