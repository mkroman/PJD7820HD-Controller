#include <string.h>

#include "esp_log.h"
#include "esp_task.h"

#include "pjd7820hd/pjd7820hd.h"
#include "pjd7820hd/commands.h"

#define BUF_SIZE (127)

static const char* TAG = "pjd7820hd";

esp_err_t pjd7820hd_init(const pjd7820hd_init_config_t* config, pjd7820hd_ctx_t** ctx) {
    esp_err_t res = ESP_FAIL;
    pjd7820hd_ctx_t* ctx_p;

    ctx_p = (pjd7820hd_ctx_t*)malloc(sizeof(pjd7820hd_ctx_t));

    if (ctx_p == NULL)
        return ESP_ERR_NO_MEM;

    *ctx = ctx_p;

    // Configure the UART driver.
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    res = uart_param_config(config->uart_num, &uart_config);

    if (res != ESP_OK) {
        goto uart_fail;
    }

    res = uart_set_pin(config->uart_num, config->tx_io_num, config->rx_io_num, config->rts_io_num, config->cts_io_num);

    if (res != ESP_OK) {
        goto uart_fail;
    }

    res = uart_driver_install(config->uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);

    if (res != ESP_OK) {
        goto uart_fail;
    }

    ctx_p->uart_num = config->uart_num;

    // Allocate a read/write buffer.
    ctx_p->buffer = (uint8_t*)malloc(BUF_SIZE);

    uart_read_bytes(ctx_p->uart_num, ctx_p->buffer, BUF_SIZE, 200 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "Initialized PJD7820HD driver on UART %d", ctx_p->uart_num);

    return ESP_OK;

uart_fail:
    ESP_LOGE(TAG, "UART init failed");

    return res;
}


void pjd7820hd_free(pjd7820hd_ctx_t* ctx)
{
    if (ctx == NULL)
        return;

    if (ctx->buffer)
        free(ctx->buffer);

    ESP_ERROR_CHECK(uart_driver_delete(ctx->uart_num));

    free(ctx);
}

esp_err_t pjd7820hd_send_command(const pjd7820hd_ctx_t* ctx, int cmd)
{
    int res;
    pjd7820hd_command_t* command = (pjd7820hd_command_t*)&pjd7820hd_commands[cmd];

    ESP_LOGI(TAG, "Sending command %d to UART %d", cmd, ctx->uart_num);

    res = uart_write_bytes(ctx->uart_num, command->command, command->command_len);

    if (res == -1) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Wrote %d bytes to UART FIFO", res);

    return ESP_OK;
}

bool pjd7820hd_cmd_is_ack(uint8_t* buf, int len)
{
    int res;

    if (len < sizeof(PJD7820HD_CMD_ACK))
        return false;

    res = memcmp(buf, &PJD7820HD_CMD_ACK, sizeof(PJD7820HD_CMD_ACK));

    return res == 0;
}

esp_err_t pjd7820hd_read_command(const pjd7820hd_ctx_t* ctx, int* length)
{
    int res;

    res = uart_read_bytes(ctx->uart_num, ctx->buffer, BUF_SIZE, 20 / portTICK_RATE_MS);

    if (res == -1) {
        return ESP_ERR_INVALID_STATE;
    }

    *length = res;

    ESP_LOGI(TAG, "Read %d bytes from UART %d", res, ctx->uart_num);
    ESP_LOG_BUFFER_HEX(TAG, ctx->buffer, res);

    return ESP_OK;
}

esp_err_t pjd7820hd_power_on(const pjd7820hd_ctx_t* ctx)
{
    int len;
    esp_err_t res = ESP_FAIL;

    res = pjd7820hd_send_command(ctx, (int)CMD_POWER_TURN_ON);

    if (res != ESP_OK) {
        goto fail;
    }

    res = pjd7820hd_read_command(ctx, &len);

    if (res != ESP_OK || len == 0) {
        goto fail;
    }

    if (!pjd7820hd_cmd_is_ack(ctx->buffer, len)) {
        ESP_LOGE(TAG, "power on fail: expected ack");

        return ESP_FAIL;
    }

    return ESP_OK;

fail:
    ESP_LOGE(TAG, "power on fail");
    return res;
}

esp_err_t pjd7820hd_power_off(const pjd7820hd_ctx_t* ctx)
{
    int len;
    esp_err_t res = ESP_FAIL;

    res = pjd7820hd_send_command(ctx, (int)CMD_POWER_TURN_OFF);

    if (res != ESP_OK) {
        goto fail;
    }

    res = pjd7820hd_read_command(ctx, &len);

    if (res != ESP_OK || len == 0) {
        goto fail;
    }

    if (memcmp(ctx->buffer, "\x00\x03\x14\x00\x00\x00\x14", 7) != 0)
    {
        ESP_LOGE(TAG, "power off fail: expected ack");

        return ESP_FAIL;
    }

    return ESP_OK;

fail:
    ESP_LOGE(TAG, "power off fail");
    return res;
}
