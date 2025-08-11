# ESP32C3 + TB600B S02
### Used pin:
- 20 RX
- 21 TX
- 9 RX
- 10 TX

### Powerline
- 3.3v
- GND

### Defined PIN
```cpp
// UART0 for S02
#define SO2_UART_PORT UART_NUM_0
#define SO2_UART_RX_PIN 20
#define SO2_UART_TX_PIN 21

// UART1 for H2S
#define H2S_UART_PORT UART_NUM_1
#define H2S_UART_RX_PIN 9
#define H2S_UART_TX_PIN 10
```

### Toolchain
- ESP-IDF-V5.5

### ChangelogS:
- 20250811
    - Added new UART port for H2S and S02. Now can read 'stimulesly / bersamaan'.
- 20250808
    - Added get Sensor data. But some the variable still unused.
- 20250807
    - Added UART commands: Led Query, LED ON, LED OFF.

