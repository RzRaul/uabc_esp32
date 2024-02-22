#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_vfs.h"
#include "esp_vfs_dev.h"



#define DEFAULT_SCAN_LIST_SIZE 7
#define TIMES_AVG_RSSI 5
#define UART_NUM UART_NUM_0
#define MAX_BUFFER 100
#define LED_1 GPIO_NUM_25
#define LED_2 GPIO_NUM_33
#define LED_3 GPIO_NUM_32

void connect_to_wifi(const char*,const char*);
void displaySignalLeds(int level);
int askAgain();
int getWorstRSSI();
int getBestRSSI();
int get_level_among_aps();
int avg_current_rssi();
int get_actual_rssi();
char getChar();
void init_uart();
void getstr();
char *getline();
void configure_leds();
void showLeds();
static void print_auth_mode(int authmode);

// Globals

static const char *TAG = "scan";
static const char *TAG_WIFI = "wifi";
wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
char readerBuffer[MAX_BUFFER] = {0};
char *line;
gpio_config_t configIO;



/* Initialize Wi-Fi as sta and set scan method */
static void wifi_activate(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

}

static void wifi_scan(){
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    uint16_t ap_count = 0;
    int selected_network = 0;
    memset(ap_info, 0, sizeof(ap_info));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    printf("Redes encontradas = %u\n", ap_count);

    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        printf("%d.- SSID %s con RSSI: %d ", i+1,ap_info[i].ssid, ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        printf("\n");
    }
    printf("\nSelecciona la red a la que te quieres conectar: \n");
    selected_network = (int)getChar() - '0';
    fflush(stdin);
    ESP_LOGI(TAG, "Seleccionaste la red %d\n", selected_network);
    if (selected_network > 0 && selected_network <= ap_count) {
        char ssid[32]={0};
        char password[MAX_BUFFER]={0};
        strcpy(ssid, (char*)ap_info[selected_network-1].ssid);
        printf("SSID: %s\n", ssid);
        printf("Password: \n");
        line = getline();
        strcpy(password, line);
        connect_to_wifi(ssid, password);
    } else {
        ESP_LOGE(TAG, "Red no encontrada\n");
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    init_uart();
    wifi_activate();
    wifi_scan();
    configure_leds();
    //create a task to show the leds running in parallel
    xTaskCreate(showLeds, "showLeds", 2048, NULL, 5, NULL);
    while(askAgain()){
        wifi_scan();
    }
}
void configure_leds(){
    configIO.pin_bit_mask = (1ULL<<LED_1) | (1ULL<<LED_2) | (1ULL<<LED_3);
    configIO.mode = GPIO_MODE_OUTPUT;
    configIO.pull_up_en = GPIO_PULLUP_ENABLE;
    configIO.pull_down_en = GPIO_PULLDOWN_DISABLE;
    configIO.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&configIO);
}
void showLeds(){
    while(1){
    int level = get_level_among_aps();
    displaySignalLeds(level);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
int askAgain(){
    char answer = 0;
    printf("Mostrar de nuevo la lista? (y/n): \n");
    answer = getChar();
    if (answer == 'y' || answer == 'Y') {
        return 1;
    } else {
        return 0;
    }
}
void connect_to_wifi(const char *ssid, const char *password) {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    printf("\nConectando a %s con password: %s\n", wifi_config.sta.ssid, wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
}
static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        printf("Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_OWE:
        printf("Authmode \tWIFI_AUTH_OWE");
        break;
    case WIFI_AUTH_WEP:
        printf("Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        printf("Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        printf("Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        printf("Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_ENTERPRISE:
        printf("Authmode \tWIFI_AUTH_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        printf("Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        printf("Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        printf("Authmode \tWIFI_AUTH_WPA3_ENT_192");
        break;
    default:
        printf("Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}
int get_actual_rssi() {
    wifi_ap_record_t aux_ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&aux_ap_info);
    if (ret == ESP_OK) {
        return (aux_ap_info.rssi);
    } else {
        return 0;
    }
}
int avg_current_rssi() {
    int rssi = 0;
    for (int i = 0; i < TIMES_AVG_RSSI; i++) {
        rssi += get_actual_rssi();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return rssi / TIMES_AVG_RSSI;
}
int get_level_among_aps(){
    int rssi = avg_current_rssi();
    int best_rssi = getBestRSSI();
    int worst_rssi = getWorstRSSI();
    int steps = (best_rssi - worst_rssi) / 3;
    if (rssi > best_rssi - steps) {
        return 3;
    } else if (rssi > best_rssi - 2 * steps) {
        return 2;
    } else {
        return 1;
    }
}
int getBestRSSI() {
    int best_rssi = -100;
    for (int i = 0; i < DEFAULT_SCAN_LIST_SIZE; i++) {
        if (ap_info[i].rssi > best_rssi) {
            best_rssi = ap_info[i].rssi;
        }
    }
    return best_rssi;
}
int getWorstRSSI() {
    int worst_rssi = 0;
    for (int i = 0; i < DEFAULT_SCAN_LIST_SIZE; i++) {
        if (ap_info[i].rssi < worst_rssi) {
            worst_rssi = ap_info[i].rssi;
        }
    }
    return worst_rssi;
}
//shows the level of the wifi signal with 3 leds via gpio
void displaySignalLeds(int level){
    if (level == 1) {
        //turn on led 1
        ESP_LOGI(TAG_WIFI, "Led 1 encendido\n");
        gpio_set_level(LED_1, 1);
        gpio_set_level(LED_2, 0);
        gpio_set_level(LED_3, 0);
    } else if (level == 2) {
        //turn on led 1 and 2
        ESP_LOGI(TAG_WIFI, "Led 1 y 2 encendidos\n");
        gpio_set_level(LED_1, 1);
        gpio_set_level(LED_2, 1);
        gpio_set_level(LED_3, 0);
    } else {
        ESP_LOGI(TAG_WIFI, "Led 1, 2 y 3 encendidos\n");
        //turn on led 1, 2 and 3
        gpio_set_level(LED_1, 1);
        gpio_set_level(LED_2, 1);
        gpio_set_level(LED_3, 1);
    }
}
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, MAX_BUFFER * 2, 0, 0, NULL, 0);
}
char getChar() {
    uint8_t data = 0;
    while (1) {
        int len = uart_read_bytes(UART_NUM, &data, 1, portMAX_DELAY);
        if (len == 1) {
            return (char)data;
        }
    }
}
char* getline() {
    static char buffer[MAX_BUFFER];
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    int index = 0;
    while (1) {
        char c = getChar();
        if (c == '\n' || c == '\r') {
            printf("\n"); // Print newline
            break;
        } else if (c == '\b' || c == 127) { // Backspace or delete
            if (index > 0) {
                index--;
                printf("\b \b"); // Move cursor back, erase character, move cursor back again
            }
        } else if (index < MAX_BUFFER - 1) {
            buffer[index++] = c;
            printf("%c", c); // Echo character
        }
    }
    fflush(stdout);
    buffer[index] = '\0'; // Null-terminate the string
    return buffer;
}




