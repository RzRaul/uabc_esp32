# Sistemas Embebidos
Unidad 1. Fundamentos de sistemas embebidos, sistemas operativos y microcontroladores
El lenguaje C se considera un lenguaje de 
- Evaluaciones parciales ... 50%
- Prácticas ... 30%
- Proyecto ... 20%


### Sistemas embebidos
Los sitemas embebidos tienen tareas específicas y limitadas, que pertenecen a un sistema mayor.

Por ejemplo, dentro de un auto existen muchos sistemas embebidos, cada sistema aislado representa uno, como el control de frenos, sistema neumático, etc.

La organización de los componentes es parecida a una de propósito general

### Tema 2 

Here's the table for GPIOs availability in the ESP32 microcontroller:
- Input only pins: 34, 35, 36, 39 (also they do not have internal pull-up or pull-down resistors)
- Output only pins: 0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
- Input/Output pins: 1, 3, 6, 7, 8, 9, 10, 11, 20, 24, 28, 29, 30, 31

The ADC2 pins cannot be used while Wi-Fi is enabled. The ADC1 pins can be used at any time.

Dont use interruptions with the GPIO36 and GPIO39 pins. They are used for the flash memory.

Configuraación de pines en ESP32:
```c
gpio_reset_pin(GPIO_NUM_2);
gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);

uint8_t input_state = gpio_get_level(GPIO_NUM_2);
gpio_set_level(GPIO_NUM_2, 1);
gpio_set_level(GPIO_NUM_2, 0);

//Example
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#define LED GPIO_NUM_2
#define DELAY 1000

void app_main(){
  gpio_reset_pin(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);

  while(true){
    gpio_set_level(LED, 1);
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
    gpio_set_level(LED, 0);
    vTaskDelay(DELAY / portTICK_PERIOD_MS);
  }
}

```
```c
//we  have different types of interruptions for a GPIO
//For example, we can use the interruption when the signal is rising, falling, or both
//Here the example code:
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

app_main(){
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  io_conf.pin_bit_mask = 1 << GPIO_NUM_2;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(GPIO_NUM_2, isr_handler, (void*) GPIO_NUM_2);
}
```

















