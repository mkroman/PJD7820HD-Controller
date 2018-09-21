/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "pjd7820hd/pjd7820hd.h"
#include "pjd7820hd/commands.h"

void app_main()
{
    printf("Hello world!\n");

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

    assert(pjd7820hd_commands[0].cmd == CMD_POWER_TURN_ON);
    assert(pjd7820hd_commands[159].cmd == CMD_GAMMA_READ_STATUS);

    ESP_LOGI("main", "CMD_GAMMA_READ_STATUS command:");
    ESP_LOG_BUFFER_HEX("main", pjd7820hd_commands[CMD_GAMMA_READ_STATUS].command, pjd7820hd_commands[CMD_GAMMA_READ_STATUS].command_len);

    ESP_ERROR_CHECK(pjd7820hd_init(&pjd7820hd_config));
}
