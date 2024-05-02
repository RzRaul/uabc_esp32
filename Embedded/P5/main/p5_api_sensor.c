#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "bme280.h"
#include "bme280_defs.h"
#include <string.h>


#define PIN_SPI_SCK 18
#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_CS 5
#define spi_MASTER_FREQ_HZ 1000000
#define BME280_ADDR 0x78     
#define BME280_ADDR_ID 0xD0               
#define SAMPLE_COUNT  UINT8_C(5)

static esp_err_t spi_init();
static esp_err_t spi_master_read(uint8_t reg, uint8_t *data, size_t len);
static esp_err_t spi_master_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
static void bme280_delay_us(uint32_t period, void *intf_ptr);
void bme280_initialize();
static void bme280_task(void *pvParameters);
void printRegs();

int8_t rslt;
uint32_t period;
struct bme280_dev dev;
struct bme280_settings settings;
typedef struct {
    /*! Compensated pressure */
    double pressure;

    /*! Compensated temperature */
    double temperature;

    /*! Compensated humidity */
    double humidity;
}bme280_data;

bme280_data data;
spi_device_handle_t spi;

static const char *TAG = "BME280";

static void bme280_task(void *pvParameters) {
    float temperature, pressure, humidity;
    uint8_t who_am_i=0;
    rslt = spi_master_read(BME280_ADDR_ID, &who_am_i, 1);
    ESP_LOGI(TAG, "BME280 WHO AM I?: 0x%x", who_am_i);
    //Reads TEMP_LOW, TEMP_HIGH, and TEMP_XLSB registers
    printRegs();
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &dev);
    rslt = bme280_get_sensor_data(BME280_ALL, &data, &dev);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printRegs();

    vTaskDelay(4000 / portTICK_PERIOD_MS);

    while (1) {

        rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &dev);
        rslt = bme280_get_sensor_data(BME280_ALL, &data, &dev);
        temperature = data.temperature;
        pressure = data.pressure;
        humidity = data.humidity;

        ESP_LOGI(TAG, "Temp: %.2f C, Presión: %.2f hPa, Humedad: %.2f %%", temperature, pressure, humidity);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    ESP_ERROR_CHECK(spi_init());
    bme280_initialize();
    xTaskCreate(bme280_task, "bme280_task", 4096, NULL, 5, NULL);
}

static esp_err_t spi_init() {

    spi_bus_config_t bus_config = {
        .mosi_io_num = PIN_SPI_MOSI,
        .miso_io_num = PIN_SPI_MISO,
        .sclk_io_num = PIN_SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t dev_config = {
        .mode = 0,  // SPI mode 0
        .clock_speed_hz = 1000000,  // 1 MHz clock
        .spics_io_num = PIN_SPI_CS,
        .queue_size = 1,
    };

    spi_bus_initialize(HSPI_HOST, &bus_config, 1);
    return spi_bus_add_device(HSPI_HOST, &dev_config, &spi);
}


static esp_err_t spi_master_read(uint8_t reg_addr, uint8_t *data, size_t len) {
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8+8*len;
    t.rxlength = 8*len;
    t.tx_buffer = &reg_addr;

    char * buf = malloc(len+1);
    memset(buf, 0, len+1);
    t.rx_buffer = buf;

    rslt = spi_device_polling_transmit(spi, &t);
   
    memmove(data, &buf[1], len);
    free(buf);
    
    if (rslt ==  ESP_OK)
        return 0;
    return rslt;
}

static esp_err_t spi_master_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8*len+8;

    char * buf = malloc(len+1);
    buf[0] = reg_addr;
    memcpy(&buf[1], data, len);
    t.tx_buffer = buf;
    rslt = spi_device_polling_transmit(spi, &t);
    free(buf);   
    //had to add this delay, without it the bme280_init(&dev) return -6 BME280_E_NVM_COPY_FAILED
    //you can set  uint8_t try_run = 50; into bme280_soft_reset function in bme280.c and remove this delay
    vTaskDelay(pdMS_TO_TICKS(10)); 
    
    if (rslt ==  ESP_OK)
        return 0;
    return rslt;
}

//create a delay_us function using vTaskDelay
static void bme280_delay_us(uint32_t period, void *intf_ptr)
{
    vTaskDelay(period / portTICK_PERIOD_MS);
}

void bme280_initialize(){
    rslt = BME280_OK;
    
    dev.chip_id = BME280_ADDR;
    dev.intf = BME280_SPI_INTF;
    dev.read = (bme280_read_fptr_t) spi_master_read;
    dev.write = (bme280_write_fptr_t)spi_master_write;
    dev.delay_us = (bme280_delay_us_fptr_t)bme280_delay_us;
    rslt = bme280_init(&dev);
    ESP_LOGI(TAG, "bme280_init() result: %d", rslt);

    /* Recommended mode of operation: Indoor navigation */
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_16X;
    settings.osr_t = BME280_OVERSAMPLING_2X;
    settings.filter = BME280_FILTER_COEFF_16;
    settings.standby_time = BME280_STANDBY_TIME_0_5_MS;
    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);
    ESP_LOGI(TAG, "bme280_set_sensor_settings() result: %d", rslt);
    ESP_LOGI(TAG, "bme280_set_sensor_mode() result: %d", rslt);


}
void printRegs(){
    uint8_t readings[3];
    rslt = spi_master_read(0xFA, readings, 3);
    ESP_LOGI(TAG, "\nTEMP_LOW: 0x%x\nTEMP_HIGH: 0x%x\nTEMP_XLSB: 0x%x", readings[0], readings[1], readings[2]);
    //Reads PRESSURE_LOW, PRESSURE_HIGH, and PRESSURE_XLSB registers
    rslt = spi_master_read(0xF7, readings, 3);
    ESP_LOGI(TAG, "\n\nPRESSURE_LOW: 0x%x\nPRESSURE_HIGH: 0x%x\nPRESSURE_XLSB: 0x%x", readings[0], readings[1], readings[2]);
    //Reads HUMIDITY_LOW and HUMIDITY_HIGH registers
    rslt = spi_master_read(0xFD, readings, 2);
    ESP_LOGI(TAG, "\n\nHUMIDITY_LOW: 0x%x\nHUMIDITY_HIGH: 0x%x\n", readings[0], readings[1]);
}