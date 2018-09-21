#include "esp_log.h"
#include "esp_task.h"

#include "pjd7820hd/pjd7820hd.h"
#include "pjd7820hd/commands.h"

// #define BUF_SIZE (127)

// static const char CMD_READ_POWER_STATUS[] = {
//     0x07, 0x14, 0x00, 0x05, 0x00, 0x34, 0x00, 0x00, 0x11, 0x00, 0x5E
// };

// static const char CMD_POWER_ON[] = {
//     0x06, 0x14, 0x00, 0x04, 0x00, 0x34, 0x11, 0x00, 0x00, 0x5D,
// };

// static const char CMD_POWER_OFF[] = {
//     0x06, 0x14, 0x00, 0x04, 0x00, 0x34, 0x11, 0x01, 0x00, 0x5E,
// };


// static void uart_task(void *parameters) {
//     // Configure the UART driver.
//     uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity    = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
//     };

//     uart_param_config(UART_NUM_2, &uart_config);
//     uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//     uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);

//     // Configure a temporary buffer for the incoming data
//     uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

//     uart_write_bytes(UART_NUM_2, CMD_POWER_ON, sizeof(CMD_POWER_OFF));

//     while (true) {
//         // off status: 0x5 0x14 0 0x3 0 0 0 0 0x17
//         uart_write_bytes(UART_NUM_2, CMD_READ_POWER_STATUS, sizeof(CMD_READ_POWER_STATUS));

//         int len = uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 20 / portTICK_RATE_MS);

//         if (len > 0) {
//             printf("read %d bytes\n", len);

//             for (int i = 0; i < len; i++) {
//                 printf("%#x ", data[i]);
//             }

//             printf("\n");
//             vTaskDelay(500 / portTICK_RATE_MS);
//         }
//     }
// }

static const char* TAG = "pjd7820hd";

esp_err_t pjd7820hd_init(const pjd7820hd_init_config_t* config) {
  ESP_LOGI(TAG, "Initialized PJD7820HD driver");

  pjd7820hd_ctx_t* pjd7820hd_ctx = malloc(sizeof(pjd7820hd_ctx_t));

  if (pjd7820hd_ctx == NULL) {
    ESP_LOGE(TAG, "Not enough memory to allocate pj7820hd_ctx_t");

    return NULL;
  }

  QueueHandle_t rx_queue = xQueueCreate(10, sizeof(pjd7820hd_cmd)); // fixme: response

  if (rx_queue == NULL) {
    ESP_LOGE(TAG, "xQueueCreate for rx_queue failed");

    return NULL;
  }

  QueueHandle_t tx_queue = xQueueCreate(10, sizeof(pjd7820hd_cmd));

  if (tx_queue == NULL) {
    ESP_LOGE(TAG, "xQueueCreate for tx_queue failed");

    return NULL;
  }

  TaskHandle_t rx_task, tx_task;
  BaseType_t x_returned;

  x_returned = xTaskCreate(pjd7820hd_recv_task, "pj7820hd_recv", 1024, pjd7820hd_ctx, ESP_TASK_MAIN_PRIO, &rx_task);

  if (x_returned != pdPASS) {
    ESP_LOGE(TAG, "xTaskCreate failed");

    return NULL;
  }

  x_returned = xTaskCreate(pjd7820hd_send_task, "pj7820hd_send", 1024, pjd7820hd_ctx, ESP_TASK_MAIN_PRIO, &tx_task);

  if (x_returned != pdPASS) {
    ESP_LOGE(TAG, "xTaskCreate failed");

    return NULL;
  }

  return ESP_OK;
}


esp_err_t pjd7820hd_send_command(const pjd7820hd_ctx_t* ctx, int cmd) {
  return ESP_OK;
}

void pjd7820hd_recv_task(void* ctx) {
  while (1) {
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

void pjd7820hd_send_task(void* ctx) {
  while (1) {
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}
