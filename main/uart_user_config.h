#ifndef UART_USER_CONFIG_H
#define UART_USER_CONFIG_H

#include "hal/uart_types.h"

// Define default baud rate 9600
#define UART_BAUD_RATE 9600

// UART0 for S02
#define SO2_UART_PORT UART_NUM_0
#define SO2_UART_RX_PIN 20
#define SO2_UART_TX_PIN 21

// UART1 for H2S
#define H2S_UART_PORT UART_NUM_1
#define H2S_UART_RX_PIN 9
#define H2S_UART_TX_PIN 10

static const int RX_BUF_SIZE = 128;
static const char *TAG_UART_HEADER = "UART_INIT";

void init_uart(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate, const char *tag);

#endif // UART_USER_CONFIG_H