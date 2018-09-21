#pragma once
#ifndef __PJD7820HD_H
#define __PJD7820HD_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

typedef struct pjd7820hd_ctx {
  QueueHandle_t rx_queue;
  QueueHandle_t tx_queue;
  TaskHandle_t rx_task;
  TaskHandle_t tx_task;
  int uart_num;
} pjd7820hd_ctx_t;


typedef struct {
  int tx_io_num;
  int rx_io_num;
  int rts_io_num;
  int cts_io_num;
  int uart_num;
} pjd7820hd_init_config_t;

typedef int pjd7820hd_cmd;

#define PJD7820HD_INIT_CONFIG_DEFAULT() { \
  .tx_io_num = GPIO_NUM_17, \
  .rx_io_num = GPIO_NUM_16, \
  .rts_io_num = UART_PIN_NO_CHANGE, \
  .cts_io_num = UART_PIN_NO_CHANGE, \
  .uart_num = UART_NUM_1 \
};

void pjd7820hd_recv_task(void* ctx);

void pjd7820hd_send_task(void* ctx);

/// @brief
esp_err_t pjd7820hd_init(const pjd7820hd_init_config_t* config);
esp_err_t pjd7820hd_send_command(const pjd7820hd_ctx_t* ctx, int cmd);

#endif
