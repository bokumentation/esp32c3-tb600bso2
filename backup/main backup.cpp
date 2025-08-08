/*
Last update - 20250807

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
#include <cstdint>

#define LED_PIN 8
#define UART_PORT UART_NUM_1
#define UART_RX_PIN 20
#define UART_TX_PIN 21
#define UART_BAUD_RATE 9600
#define BUF_SIZE 128

static const char *TAG_UART_SENSOR = "[UART_SENSOR]"; // Tag for ESP_LOG
static const char *TAG_START = "[START]";
static const char *TAG_TASK = "[TASK]";

void readConfirmation() {
  const int responseLength = 2;
  uint8_t responseData[responseLength];
  int bytesRead = uart_read_bytes(UART_PORT, responseData, responseLength,
                                  pdMS_TO_TICKS(1000));

  if (bytesRead == responseLength) {
    if (responseData[0] == 0x4F && responseData[1] == 0x4B) {
      ESP_LOGI(TAG_UART_SENSOR, "Received: 'OK' confirmation.");
    } else {
      ESP_LOGW(TAG_UART_SENSOR,
               "Received unexpected response for confirmation.");
      ESP_LOG_BUFFER_HEXDUMP(TAG_UART_SENSOR, responseData, bytesRead,
                             ESP_LOG_INFO);
    }
  } else {
    ESP_LOGE(TAG_UART_SENSOR,
             "Failed to receive 'OK' confirmation within timeout.");
  }
}

void readStatusResponse() {
  const int responseLength = 9;
  uint8_t responseData[responseLength];
  int bytesRead = uart_read_bytes(UART_PORT, responseData, responseLength,
                                  pdMS_TO_TICKS(1000));

  if (bytesRead == responseLength) {
    ESP_LOGI(TAG_UART_SENSOR, "Received Status Response:");
    ESP_LOG_BUFFER_HEXDUMP(TAG_UART_SENSOR, responseData, responseLength,
                           ESP_LOG_INFO);

    if (responseData[2] == 0x01) {
      ESP_LOGI(TAG_UART_SENSOR, "Light Status: ON (0x01)");
    } else if (responseData[2] == 0x00) {
      ESP_LOGI(TAG_UART_SENSOR, "Light Status: OFF (0x00)");
    } else {
      ESP_LOGW(TAG_UART_SENSOR, "Light Status: Unknown");
    }
  } else {
    ESP_LOGE(TAG_UART_SENSOR,
             "Failed to receive status response within timeout.");
  }
}

void readCombinedData() {
  // Command to get combined gas concentration, temperature, and humidity
  uint8_t combinedReadCommand[] = {0xFF, 0x01, 0x87, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x78};
  const int responseLength = 13;
  uint8_t responseData[responseLength];

  ESP_LOGI(TAG_TASK, "--- [SENDING] command to get combined data ---");
  uart_write_bytes(UART_PORT, combinedReadCommand, sizeof(combinedReadCommand));

  // Wait for the response from the sensor
  int bytesRead = uart_read_bytes(UART_PORT, responseData, responseLength,
                                  pdMS_TO_TICKS(1000));

  if (bytesRead == responseLength) {
    ESP_LOGI(TAG_TASK, "Received Combined Data Response:");
    ESP_LOG_BUFFER_HEXDUMP(TAG_TASK, responseData, responseLength,
                           ESP_LOG_INFO);

    // Check if the response packet starts with the correct header
    if (responseData[0] == 0xFF && responseData[1] == 0x87) {
      // Parse temperature from bytes 8 and 9
      int16_t rawTemperature =
          (int16_t)((responseData[8] << 8) | responseData[9]);
      float temperature = (float)rawTemperature / 100.0;

      // Parse humidity from bytes 10 and 11
      uint16_t rawHumidity =
          (uint16_t)((responseData[10] << 8) | responseData[11]);
      float humidity = (float)rawHumidity / 100.0;

      // Parse gas concentration (ug/m³) from bytes 2 and 3
      uint16_t rawGasUg = (uint16_t)((responseData[2] << 8) | responseData[3]);
      float gasUg = (float)rawGasUg;

      ESP_LOGI(TAG_TASK, "Temperature: %.2f °C", temperature);
      ESP_LOGI(TAG_TASK, "Humidity: %.2f %%", humidity);
      ESP_LOGI(TAG_TASK, "Gas Concentration: %.2f ug/m^3", gasUg);
    } else {
      ESP_LOGW(TAG_TASK, "Received malformed response header.");
    }
  } else {
    ESP_LOGE(
        TAG_TASK,
        "Failed to receive complete combined data response within timeout.");
  }
}

void getLedStatus() {
  // Command to CHECK light status
  const uint8_t cmdQueryLightStatus[] = {0xFF, 0x01, 0x8A, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x75};
  vTaskDelay(pdMS_TO_TICKS(100));
  uart_write_bytes(UART_PORT, cmdQueryLightStatus, sizeof(cmdQueryLightStatus));
  readStatusResponse();
}

void turnOffLed() {
  // Command to turn OFF the running lights
  const uint8_t cmdTurnOffLights[] = {0xFF, 0x01, 0x88, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x77};
  ESP_LOGI(TAG_UART_SENSOR,
           "--- [SENDING] command to TURN OFF the running lights.");

  uart_write_bytes(UART_PORT, cmdTurnOffLights, sizeof(cmdTurnOffLights));
  readConfirmation();
  getLedStatus();
}

void turnOnLed() {
  // Command to turn ON the running lights
  const uint8_t cmdTurnOnLights[] = {0xFF, 0x01, 0x89, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x76};
  ESP_LOGI(TAG_UART_SENSOR,
           "--- [SENDING] command to TURN ON the running lights.");

  uart_write_bytes(UART_PORT, cmdTurnOnLights, sizeof(cmdTurnOnLights));
  readConfirmation();
  getLedStatus();
}

void uart_communication_task(void *pvParameters) {

  /* --- COMMANDS: MODE SWITCHING ---- */
  // Switches to active upload mode (sensor sends data continuously)
  const uint8_t cmdSwitchToActiveUpload[] = {0xFF, 0x01, 0x78, 0x40, 0x00,
                                             0x00, 0x00, 0x00, 0x47};
  // Switches to passive upload (query) mode (sensor waits for commands)
  const uint8_t cmdSwitchToPassiveUpload[] = {0xFF, 0x01, 0x78, 0x41, 0x00,
                                              0x00, 0x00, 0x00, 0x46};
  /* --- COMMANDS: MODE SWITCHING ---- */

  /* --- COMMAND: DATA QUERY MODE --- */
  // Command to get sensor type, max range, and unit
  const uint8_t commandGetSensorInfo[] = {0xFF, 0x01, 0xD1, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x35};

  // Command to get sensor type, max range, unit, and decimal places (Command 4
  // format)
  const uint8_t commandGetSensorInfo2[] = {0xFF, 0x07, 0x24, 0x00, 0xC8,
                                           0x02, 0x01, 0x00, 0x3A};

  // Command to actively read gas concentration value
  const uint8_t commandGetGasConcentration[] = {0xFF, 0x01, 0x86, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x79};

  // Command to get combined gas concentration, temperature, and humidity
  const uint8_t commandGetCombinedData[] = {0xFF, 0x01, 0x87, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x78};

  // Command to get current version number
  // Note: The datasheet does not show the full command packet, only the return
  // value. The command byte is 0x19, so the command structure would likely be
  // similar to others.
  const uint8_t commandGetVersion[] = {
      0xFF, 0x01, 0x19, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00}; // Checksum is assumed
  /* --- COMMAND: DATA QUERY MODE --- */

  /* ########################## USER CODE BEGIN ########################## */
  ESP_LOGI(TAG_TASK, "--- UART COMMUNICATION TASK BEGIN ---");
  ESP_LOGI(TAG_TASK, "[DELAY] Wait for the delay...");
  vTaskDelay(pdMS_TO_TICKS(3000));

  // --- Send command to QnA mode first
  ESP_LOGI(TAG_UART_SENSOR, "[MODE] Switching to QnA Mode ---");
  uart_write_bytes(UART_PORT, cmdSwitchToPassiveUpload,
                   sizeof(cmdSwitchToPassiveUpload));

  ESP_LOGI(TAG_UART_SENSOR, "[WAIT] Wait for the delay ---");
  vTaskDelay(pdMS_TO_TICKS(5000));

  while (1) {
    // ---
  }
}

extern "C" void app_main(void) {
  // UART configuration
  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  // Install UART driver
  uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(UART_PORT, &uart_config);
  uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);

  ESP_LOGI(TAG_START, "UART driver initialized.");
  ESP_LOGI(TAG_START, "Wait for the delay...");
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Create the communication task
  xTaskCreate(uart_communication_task, "uart_comm", 4096, NULL, 10, NULL);
}
