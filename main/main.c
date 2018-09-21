#include <stdio.h>

#include "driver/uart.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "pjd7820hd/pjd7820hd.h"
#include "pjd7820hd/commands.h"

static const char* TAG = "pjd7820hd-controller";
static pjd7820hd_ctx_t* g_pjd7820hd_ctx = NULL;
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_err_t res;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_publish(client, CONFIG_MQTT_AVAILABILITY_TOPIC, CONFIG_MQTT_AVAILABILITY_ONLINE_MSG, sizeof(CONFIG_MQTT_AVAILABILITY_ONLINE_MSG) - 1, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_CMD_POWER_TOPIC, 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

            if (!strncmp(event->topic, CONFIG_MQTT_CMD_POWER_TOPIC, event->topic_len))
            {
                if (!strncmp(event->data, CONFIG_MQTT_CMD_POWER_ON_MSG, event->data_len))
                {
                    ESP_LOGI(TAG, "Turning on projector");

                    res = pjd7820hd_power_on(g_pjd7820hd_ctx);

                    if (res == ESP_OK)
                    {
                        esp_mqtt_client_publish(client, CONFIG_MQTT_STATUS_TOPIC, CONFIG_MQTT_STATUS_ON_MSG, sizeof(CONFIG_MQTT_STATUS_ON_MSG) - 1, 1, 1);
                    }
                }
                else if (!strncmp(event->data, CONFIG_MQTT_CMD_POWER_OFF_MSG, event->data_len))
                {
                    ESP_LOGI(TAG, "Turning off projector");

                    res = pjd7820hd_power_off(g_pjd7820hd_ctx);

                    esp_mqtt_client_publish(client, CONFIG_MQTT_STATUS_TOPIC, CONFIG_MQTT_STATUS_OFF_MSG, sizeof(CONFIG_MQTT_STATUS_OFF_MSG) - 1, 1, 1);
                }
            }

            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_init()
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(TAG, "Connecting to SSID: %s", CONFIG_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "pjd7820hd-controller");
    ESP_LOGI(TAG, "Waiting for WiFi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void mqtt_app_start()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .event_handle = mqtt_event_handler,
        .lwt_topic = CONFIG_MQTT_AVAILABILITY_TOPIC,
        .lwt_msg = CONFIG_MQTT_AVAILABILITY_OFFLINE_MSG,
        .lwt_msg_len = sizeof(CONFIG_MQTT_AVAILABILITY_OFFLINE_MSG) - 1,
        .lwt_retain = true,
        // .user_context = (void *)your_context
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    pjd7820hd_init_config_t pjd7820hd_config = PJD7820HD_INIT_CONFIG_DEFAULT();
    pjd7820hd_config.uart_num = UART_NUM_2;
    pjd7820hd_config.tx_io_num = GPIO_NUM_16;
    pjd7820hd_config.rx_io_num = GPIO_NUM_17;

    ESP_ERROR_CHECK(pjd7820hd_init(&pjd7820hd_config, &g_pjd7820hd_ctx));

    nvs_flash_init();
    wifi_init();
    mqtt_app_start();
}
