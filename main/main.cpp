/*
Last update - 20250811

Used esp32-c3 pins:
- 21 TX
- 20 RX
*/

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_log_buffer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "portmacro.h"

// USER INCLUDE
#include "tb600b_so2.h"
#include "uart_user_config.h"

static const char *SO2_UART_TAG = "[SO2]";
static const char *H2S_UART_TAG = "[H2S]";
static const char *TAG_LOOP = "[LOOP]";

// MAIN CODE
extern "C" void app_main(void) {

  // Initialize UART
  init_uart(SO2_UART_PORT, SO2_UART_TX_PIN, SO2_UART_RX_PIN, UART_BAUD_RATE, SO2_UART_TAG);
  init_uart(H2S_UART_PORT, H2S_UART_TX_PIN, H2S_UART_RX_PIN, UART_BAUD_RATE, H2S_UART_TAG);

  // Set passive mode
  tb600b::set_passive_mode(SO2_UART_PORT);
  tb600b::set_passive_mode(H2S_UART_PORT);
  vTaskDelay(pdMS_TO_TICKS(1000));

  while (1) {
    /* CODE LOOP BEGIN */
    ESP_LOGI(TAG_LOOP, "--- START FIRST LINE OF LOOP ---");
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(SO2_UART_TAG, "Get Combined Data");
    tb600b::get_combined_data(SO2_UART_PORT, CMD_GET_COMBINED_DATA, sizeof(CMD_GET_COMBINED_DATA), SO2_UART_TAG);
    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(H2S_UART_TAG, "Get Combined Data");
    tb600b::get_combined_data(H2S_UART_PORT, CMD_GET_COMBINED_DATA, sizeof(CMD_GET_COMBINED_DATA), H2S_UART_TAG);
    vTaskDelay(pdMS_TO_TICKS(3000));

    /* Send command to TURN OFF the LED */
    ESP_LOGI(SO2_UART_TAG, "Turn OFF the LED");
    tb600b::led::turn_off_led(SO2_UART_PORT);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(H2S_UART_TAG, "Turn OFF the LED");
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::turn_off_led(H2S_UART_PORT);

    /* Get LED info */
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::get_led_status(SO2_UART_PORT);
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::get_led_status(H2S_UART_PORT);

    /* Send command to TURN ON the LED */
    ESP_LOGI(SO2_UART_TAG, "Turn ON the LED");
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::turn_on_led(SO2_UART_PORT);
    ESP_LOGI(H2S_UART_TAG, "Turn ON the LED");
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::turn_on_led(H2S_UART_PORT);

    /* Get LED info */
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::get_led_status(SO2_UART_PORT);
    vTaskDelay(pdMS_TO_TICKS(1000));
    tb600b::led::get_led_status(H2S_UART_PORT);

    /* CODE LOOP END */
    ESP_LOGI(TAG_LOOP, "--- BACK TO THE FIRST LINE OF LOOP ---");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
