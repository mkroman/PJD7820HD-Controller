#pragma once
#ifndef __PJD7820HD_H
#define __PJD7820HD_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/uart.h"

typedef struct pjd7820hd_ctx {
  uint8_t* buffer;
  int uart_num;
} pjd7820hd_ctx_t;


typedef struct {
  int tx_io_num;
  int rx_io_num;
  int rts_io_num;
  int cts_io_num;
  uart_port_t uart_num;
} pjd7820hd_init_config_t;

static const char PJD7820HD_CMD_ACK[]  = { 0x03, 0x14, 0x00, 0x00, 0x00, 0x14 };
static const char PJD7820HD_CMD_NACK[] = { 0x00, 0x14, 0x00, 0x00, 0x00, 0x14 };

#define PJD7820HD_INIT_CONFIG_DEFAULT() { \
    .tx_io_num = GPIO_NUM_17, \
    .rx_io_num = GPIO_NUM_16, \
    .rts_io_num = UART_PIN_NO_CHANGE, \
    .cts_io_num = UART_PIN_NO_CHANGE, \
    .uart_num = UART_NUM_1 \
};

/// @brief Allocates buffer memory and installs an UART driver on the UART number given
///   in config.
esp_err_t pjd7820hd_init(const pjd7820hd_init_config_t* config, pjd7820hd_ctx_t** ctx);

/**
  * @brief Cleans up the context and uninstalls the UART driver.
  */
void pjd7820hd_free(pjd7820hd_ctx_t* ctx);

/** @brief Signals the PJD7820HD to power on.
  *
  * This function will send a POWER ON command to the projector.
  */
esp_err_t pjd7820hd_power_on(const pjd7820hd_ctx_t* ctx);

/** @brief Signals the PJD7820HD to power off.
  *
  * This function will send a POWER OFF command to the projector.
  */
esp_err_t pjd7820hd_power_off(const pjd7820hd_ctx_t* ctx);

esp_err_t pjd7820hd_send_command(const pjd7820hd_ctx_t* ctx, int cmd);
esp_err_t pjd7820hd_read_command(const pjd7820hd_ctx_t* ctx, int* length);

#endif
