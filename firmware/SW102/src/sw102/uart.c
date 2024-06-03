/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#include <string.h>
#include "common.h"
#include "nrf_drv_uart.h"
#include "uart.h"
#include "utils.h"
#include "assert.h"
#include "app_util_platform.h"
#include "app_uart.h"

extern uint32_t _app_uart_init(const app_uart_comm_params_t * p_comm_params,
    app_uart_buffers_t *     p_buffers,
    app_uart_event_handler_t event_handler,
    app_irq_priority_t       irq_priority);
extern uint8_t app_uart_get(void);

#define UART_IRQ_PRIORITY                       APP_IRQ_PRIORITY_LOW

#define FRAME_MAX_BYTES                         9u

#define TSDZ2_CONFIG()                          (motor_state.current_config == &tsdz2_comm_params)
#define TSDZ8_CONFIG()                          (motor_state.current_config == &tsdz8_comm_params)

/**
 *@breif UART configuration structure
 */
static const app_uart_comm_params_t tsdz2_comm_params =
{
    .rx_pin_no  = UART_RX__PIN,
    .tx_pin_no  = UART_TX__PIN,
    .rts_pin_no = RTS_PIN_NUMBER,
    .cts_pin_no = CTS_PIN_NUMBER,
    //Below values are defined in ser_config.h common for application and connectivity
    .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
    .use_parity   = false,
    .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud9600
};

static const app_uart_comm_params_t tsdz8_comm_params =
{
    .rx_pin_no  = UART_RX__PIN,
    .tx_pin_no  = UART_TX__PIN,
    .rts_pin_no = RTS_PIN_NUMBER,
    .cts_pin_no = CTS_PIN_NUMBER,
    //Below values are defined in ser_config.h common for application and connectivity
    .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
    .use_parity   = false,
    .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud19200
};

typedef struct {
  volatile motor_type_t type;
  const app_uart_comm_params_t *current_config;
} motor_state_s;

uint8_t ui8_rx_buffer[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
uint8_t ui8_tx_buffer[UART_NUMBER_DATA_BYTES_TO_SEND];
volatile uint8_t ui8_received_package_flag = 0;
motor_state_s motor_state = {
  .type = MOTOR_NONE,
  .current_config = &tsdz2_comm_params
};

uint8_t* uart_get_tx_buffer(void)
{
  return ui8_tx_buffer;
}

uint8_t* usart1_get_rx_buffer(void)
{
  return ui8_rx_buffer;
}

uint8_t usart1_received_package(void)
{
  return ui8_received_package_flag;
}

void usart1_reset_received_package(void)
{
  ui8_received_package_flag = 0;
}

static void uart_tsdz8_recv_handler(uint8_t byte)
{
  static uint8_t ui8_state_machine = 0;
  static uint8_t ui8_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
  static uint8_t ui8_rx_cnt = 0;
  uint8_t ui8_i;
  uint16_t checksum = 0;

  switch (ui8_state_machine)
  {
    case 0:
      if (ui8_byte_received == 0x43) { // see if we get start package byte
        ui8_rx[0] = ui8_byte_received;
        ui8_state_machine = 1;
      }
      else {
        ui8_state_machine = 0;
      }

      ui8_rx_cnt = 1;
    break;

    case 1:
      ui8_rx[ui8_rx_cnt++] = ui8_byte_received;

      // reset if it is the last byte of the package and index is out of bounds
      if (ui8_rx_cnt >= FRAME_MAX_BYTES)
      {
        ui8_state_machine = 0;

        for (ui8_i = 0; ui8_i < FRAME_MAX_BYTES - 1; ui8_i++)
        {
          checksum += ui8_rx[ui8_i];
        }

        // if Checksum is correct read the package
        if ((checksum & 0xFF) == ui8_rx[FRAME_MAX_BYTES - 1])
        {
          // copy to the other buffer only if we processed already the last package
          if(!ui8_received_package_flag)
          {
            ui8_received_package_flag = 1;

            // store the received data to rx_buffer
            memcpy(ui8_rx_buffer, ui8_rx, FRAME_MAX_BYTES);

            motor_state.type = MOTOR_TSDZ8;
          }
        }
      }
    break;

    default:
      ui8_state_machine = 0;
      break;
  }
}

static void uart_tsdz2_recv_handler(uint8_t byte)
{
  static uint8_t ui8_state_machine = 0;
  static uint8_t ui8_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
  static uint8_t ui8_rx_cnt = 0;
  uint8_t ui8_i;
  uint16_t ui16_crc_rx;

  switch (ui8_state_machine)
  {
    case 0:
    if (ui8_byte_received == 0x43) { // see if we get start package byte
      ui8_rx[0] = ui8_byte_received;
      ui8_state_machine = 1;
    }
    else {
      ui8_state_machine = 0;
    }

    ui8_rx_cnt = 0;
    break;

    case 1:
      ui8_rx[1] = ui8_byte_received;
      ui8_state_machine = 2;
    break;

    case 2:
    ui8_rx[ui8_rx_cnt + 2] = ui8_byte_received;
    ++ui8_rx_cnt;

    // reset if it is the last byte of the package and index is out of bounds
    if (ui8_rx_cnt >= ui8_rx[1])
    {
      ui8_state_machine = 0;

      // just to make easy next calculations
      ui16_crc_rx = 0xffff;
      for (ui8_i = 0; ui8_i < ui8_rx[1]; ui8_i++)
      {
        crc16(ui8_rx[ui8_i], &ui16_crc_rx);
      }

      // if CRC is correct read the package
      if (((((uint16_t) ui8_rx[ui8_rx[1] + 1]) << 8) +
            ((uint16_t) ui8_rx[ui8_rx[1]])) == ui16_crc_rx)
      {
        // copy to the other buffer only if we processed already the last package
        if(!ui8_received_package_flag)
        {
          ui8_received_package_flag = 1;

          // store the received data to rx_buffer
          memcpy(ui8_rx_buffer, ui8_rx, ui8_rx[1] + 2);

          motor_state.type = MOTOR_TSDZ2;
        }
      }
    }
    break;

    default:
      ui8_state_machine = 0;
      break;
  }
}

void uart_evt_callback(app_uart_evt_t * uart_evt)
{
  uint8_t ui8_byte_received;

  switch (uart_evt->evt_type)
  {
    case APP_UART_DATA:
      //Data is ready on the UART
      ui8_byte_received = app_uart_get();
      if(TSDZ2_CONFIG()) {
        uart_tsdz2_recv_handler(ui8_byte_received);
      }
      else if(TSDZ8_CONFIG()) {
        uart_tsdz8_recv_handler(ui8_byte_received);
      }
    break;

    case APP_UART_TX_EMPTY:
      //Data has been successfully transmitted on the UART
      break;

    case APP_UART_COMMUNICATION_ERROR:
        ui8_state_machine = 0;
      break;

    default:
      break;
  }
}

/**
 * @brief Init UART peripheral
 */
void uart_init(void)
{
  uint32_t err_code;
  app_uart_buffers_t buffers;
  static uint8_t tx_buf[128]; // must be equal or higher than UART_NUMBER_DATA_BYTES_TO_SEND and power of 2

  buffers.tx_buf = tx_buf;
  buffers.tx_buf_size = sizeof (tx_buf);
  err_code = _app_uart_init(current_config, &buffers, uart_evt_callback, UART_IRQ_PRIORITY);

  APP_ERROR_CHECK(err_code);
}

/**
 * @brief Change the uart config
*/
void uart_switch_config(void)
{
  if(TSDZ2_CONFIG())
  {
    motor_state.current_config = &tsdz8_comm_params;
  }
  else if(TSDZ8_CONFIG())
  {
    motor_state.current_config = &tsdz2_comm_params;
  }

  err_code = _app_uart_init(motor_state.current_config, &buffers, uart_evt_callback, UART_IRQ_PRIORITY);

  APP_ERROR_CHECK(err_code);  
}

/**
 * @brief Get the current motor detected
*/
motor_type_t uart_get_motor_type(void)
{
  return motor_state.type;
}

/**
 * @brief Returns pointer to RX buffer ready for parsing or NULL
 */
const uint8_t* uart_get_rx_buffer_rdy(void)
{
  if(!usart1_received_package()) {
    return NULL;
  }

  uint8_t *r = usart1_get_rx_buffer();
  usart1_reset_received_package();
  return r;
}

/**
 * @brief Send TX buffer over UART.
 */
void uart_send_tx_buffer(uint8_t *tx_buffer, uint8_t ui8_len)
{
  uint32_t err_code;

  for (uint8_t i = 0; i < ui8_len; i++)
  {
    err_code = app_uart_put(tx_buffer[i]);
// assume that buffer will never get full, like for instance when we are debugging
//    if (err_code != 0)
//      APP_ERROR_CHECK(err_code);
  }
}
