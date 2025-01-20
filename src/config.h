#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG false

#define MODE_STANDALONE true
#define SEND_TO_DB false

// In standalone mode, the board will always send data to the PC
#if MODE_STANDALONE
#define UART_PORT USBSerial
#else
#define UART_PORT Serial1
#endif

#define UART_BAUD 115200

#define SERIAL_TO_PC USBSerial
#define SERIAL_TO_PC_BAUD 115200

// PIN/GPIO Definition on Radio Module ERT
#define LORA_SCK 42
#define LORA_MOSI 44
#define LORA_MISO 43
#define LORA_CS 41
#define LORA_INT0 21
#define LORA_RST -1
#define NEOPIXEL_PIN 18

// LoRa Configuration
#define LORA_FREQ 915E6
#define LORA_POWER 20
#define LORA_BW 125E3
#define LORA_SF 7
#define LORA_CR 5
#define LORA_PREAMBLE_LEN 8
#define LORA_CRC true
#define LORA_INVERSE_IQ false

#define INITIAL_LED_COLOR 0

#endif // CONFIG_H