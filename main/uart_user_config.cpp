#include "uart_user_config.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/uart_types.h"
#include "soc/clk_tree_defs.h"

void init_uart(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate, const char *tag) {
  const uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  }; // Missing `rx_flow_ctrl_thresh` initializer

  ESP_ERROR_CHECK(uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  ESP_LOGI(tag, "Initialize...");
  vTaskDelay(pdMS_TO_TICKS(100));
}