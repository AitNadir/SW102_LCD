#pragma once

#include <stdint.h>

typedef enum {
  MOTOR_NONE = 0,
  MOTOR_TSDZ2,
  MOTOR_TSDZ8
} motor_type_t;

void uart_init(void);
void uart_switch_config(void);
motor_type_t uart_get_motor_type(void);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
void uart_send_tx_buffer(uint8_t *tx_buffer, uint8_t ui8_len);

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE       29
#define UART_NUMBER_DATA_BYTES_TO_SEND          88

