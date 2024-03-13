#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include <esp_https_server.h>
#include "esp_tls.h"
#include <string.h>
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "io_handler.h"
//includes the i2c driver
#include "driver/i2c.h"

#define HISTORY_LEN 10
#define SAMPLING_AVG 5
#define MY_ADC_CHANNEL ADC_CHANNEL_0
#define MY_ADC_ATTEN ADC_ATTEN_DB_0
#define MY_ADC_WIDTH ADC_BITWIDTH_10

int8_t adc_r_value = 0;
int8_t adc_g_value = 0;
int8_t adc_b_value = 0;

int8_t adc_r_values[HISTORY_LEN];

adc_oneshot_unit_handle_t adc_one_shot;



static const char *TAG = "main";
adc_oneshot_unit_handle_t adc1_handle;
t_adc_config adc_config = {
    .quant = 2,
    .channels = (adc_channel_t[]){MY_ADC_CHANNEL, ADC_CHANNEL_3},
    .atten = (adc_atten_t[]){MY_ADC_ATTEN, MY_ADC_ATTEN},
    .bitwidth = (adc_bitwidth_t[]){MY_ADC_WIDTH, MY_ADC_WIDTH},
    .unit_config = &(adc_oneshot_unit_init_cfg_t){
        .unit_id = ADC_UNIT_1,
    },
    .handle = &adc1_handle,
};



/* An HTTP GET handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    extern unsigned char view_start[] asm("_binary_view_html_start");
    extern unsigned char view_end[] asm("_binary_view_html_end");
    size_t view_len = view_end - view_start;
    char viewHtml[view_len];
    memcpy(viewHtml, view_start, view_len);
    ESP_LOGI(TAG, "URI: %s", req->uri);

    adc_g_value = readAvgADC(adc1_handle, MY_ADC_CHANNEL, SAMPLING_AVG);
    // adc_b_value = readAvgADC(adc1_handle, ADC_CHANNEL_3, SAMPLING_AVG);
    //prints ESP_LOG both values
    ESP_LOGI(TAG, "ADC1_CHANNEL_0: %i", adc_g_value);
    // ESP_LOGI(TAG, "ADC1_CHANNEL_3: %i", adc_b_value);
    
    char *viewHtmlUpdated;
    int formattedStrResult = asprintf(&viewHtmlUpdated, viewHtml, adc_r_value,adc_r_value,adc_g_value,adc_g_value,adc_b_value,adc_b_value);
    
    int8_t offset = 0;
    if (adc_r_value < 10)
    {
        offset +=2;
    }
    if (adc_g_value < 10)
    {
        offset +=2;
    }
    if (adc_b_value < 10)
    {
        offset +=2;
    }
    
    ESP_LOGI(TAG, "offset %i", offset);
    httpd_resp_set_type(req, "text/html");

    if (formattedStrResult > 0)
    {
        httpd_resp_send(req, viewHtmlUpdated, (view_len - offset));
        free(viewHtmlUpdated);
    }
    else
    {
        ESP_LOGE(TAG, "Error updating variables");
        httpd_resp_send(req, viewHtml, view_len);
    }

    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler};

static const httpd_uri_t history = {
    .uri = "/history",
    .method = HTTP_GET,
    .handler = root_get_handler};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;
    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &history);
    return server;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_ssl_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}


void app_main(void)
{   
    static httpd_handle_t server = NULL;
    setupADC(&adc_config);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    ESP_ERROR_CHECK(example_connect());
}
