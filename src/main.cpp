#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "capsule.h"
#include "PacketDefinition_Firehorn.h"
#include "test_packets_data.h"

SPIClass SPI_LoRa(HSPI);
Adafruit_NeoPixel led(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
CapsuleStatic LoRaCapsule(nullptr);

static unsigned long lastPacketSent = 0;

// -----------------------------------------------------------------------------
// 1) CONFIGURATION: toggles and intervals
// -----------------------------------------------------------------------------
bool send_gse_cmd_status  = false;  // Enable/disable GSE Command Status
bool send_gse_down        = false;  // Enable/disable GSE Downlink
bool send_av_engine_state = false;  // Enable/disable AV Engine State
bool send_av_up           = true;  // Enable/disable AV Uplink
bool send_av_down         = true;  // Enable/disable AV Downlink

// Sending intervals (ms)
unsigned long interval_gse_cmd_status = 1000;
unsigned long interval_gse_down    = 2000;
unsigned long interval_av_engine_state = 3000;
unsigned long interval_av_up   = 3000;
unsigned long interval_av_down = 6000;


// Last time each packet type was sent
unsigned long last_gse_cmd_status_sent = 0;
unsigned long last_gse_down_sent    = 0;
unsigned long last_av_engine_state_sent = 0;
unsigned long last_av_up_sent   = 0;
unsigned long last_av_down_sent = 0;
//------------------------------------------------------------------------------

//forward declarations
void handleLoRaPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len);
void sendPacket(uint8_t packetId, void* packet, size_t packetSize, const char* packetName);
//

void setup() {
  // Initialize LED
  led.begin();
  SERIAL_TO_PC.begin(SERIAL_TO_PC_BAUD);
  SERIAL_TO_PC.println("Initializing LoRa...");

  // Initialize LoRa SPI interface
  SPI_LoRa.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setSPI(SPI_LoRa);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_INT0);

  // Initialize LoRa module
  if (!LoRa.begin(LORA_FREQ)) {
    SERIAL_TO_PC.println("LoRa init failed!");
    while (1);
  }

  LoRa.setTxPower(LORA_POWER, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
  LoRa.enableCrc();

  SERIAL_TO_PC.println("LoRa init succeeded.");

  // Set initial LED color to Red
  led.fill(0xFF0000);
  led.show();
}

// -----------------------------------------------------------------------------
// 2) Main Loop
// -----------------------------------------------------------------------------
void loop() {
  unsigned long now = millis();

  //-------------------------------------------------------------------------
  // (A) GSE Downlink
  //-------------------------------------------------------------------------
  if (send_gse_cmd_status && (now - last_gse_cmd_status_sent >= interval_gse_cmd_status)) {
    last_gse_cmd_status_sent = now;
    sendPacket(0x02, &gse_cmd_status_packet, sizeof(GSE_cmd_status), "GSE Command Status");
  }

  if (send_gse_down && (now - last_gse_down_sent >= interval_gse_down)) {
    last_gse_down_sent = now;
    sendPacket(0x01, &gse_down_packet, sizeof(PacketGSE_downlink), "GSE Downlink");
  }

  //-------------------------------------------------------------------------
  // (B) AV Uplink (Example: "ignition" command)
  //-------------------------------------------------------------------------
  if (send_av_engine_state && (now - last_av_engine_state_sent >= interval_av_engine_state)) {
    last_av_engine_state_sent = now;
    sendPacket(0x0B, &av_engine_state_down_packet, sizeof(engine_state_t), "AV Engine State");
  }

  if (send_av_up && (now - last_av_up_sent >= interval_av_up)) {
    last_av_up_sent = now;
    sendPacket(0x0A, &av_up_packet, sizeof(av_uplink_t), "AV Uplink");
  }

  //-------------------------------------------------------------------------
  // (C) AV Downlink
  //-------------------------------------------------------------------------
  if (send_av_down && (now - last_av_down_sent >= interval_av_down)) {
    last_av_down_sent = now;
    sendPacket(0x08, &av_down_packet, sizeof(av_downlink_t), "AV Downlink");
  }

  // Small delay to avoid spamming the loop (not strictly needed)
  delay(10);
}




// -----------------------------------------------------------------------------
// 3) Encapsulation & LoRa Transmission
// -----------------------------------------------------------------------------
void handleLoRaPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
  lastPacketSent = millis();
  led.setPixelColor(0, led.Color(255, 255, 0)); // Yellow
  led.show();

  // Debug packet ID and length before encapsulation
  SERIAL_TO_PC.print("Packet ID: 0x");
  SERIAL_TO_PC.println(packetId, HEX);
  SERIAL_TO_PC.print("Packet Length: ");
  SERIAL_TO_PC.println(len);

  // Encode the packet using LoRaCapsule
  uint8_t* packetToSend = LoRaCapsule.encode(packetId, dataIn, len);
  uint32_t codedLen = LoRaCapsule.getCodedLen(len);

  SERIAL_TO_PC.print("Encapsulated Packet: ");
  for (uint32_t i = 0; i < codedLen; i++) {
      SERIAL_TO_PC.printf("%02X ", packetToSend[i]);
  }
  SERIAL_TO_PC.println();

  // Transmit the encoded packet over LoRa
  LoRa.beginPacket();
  LoRa.write(packetToSend, codedLen);
  LoRa.endPacket();
  delete[] packetToSend;

  // Set LED to green after send
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
}


// Generic packet sending function
void sendPacket(uint8_t packetId, void* packet, size_t packetSize, const char* packetName) {
  uint8_t* packetData = reinterpret_cast<uint8_t*>(packet);

  SERIAL_TO_PC.printf("----- Sending %s -----\n", packetName);
  SERIAL_TO_PC.printf("Raw Packet Data (%s): ", packetName);
  for (uint32_t i = 0; i < packetSize; i++) {
    SERIAL_TO_PC.printf("%02X ", packetData[i]);
  }
  SERIAL_TO_PC.println();

  handleLoRaPacket(packetId, packetData, packetSize);
}
