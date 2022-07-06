/*
  LoRa Receiver

  Description:
  Received LoRa Packets and Print them to Serial.

  Product Page:
  https://heltec.org/project/htcc-ab01/
*/

#include "LoRaWan_APP.h"
#include "Arduino.h"

/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY 866000000  // Hz
#define TX_OUTPUT_POWER 22      // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz,  1: 250 kHz,  2: 500 kHz,  3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6,  3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000

#define BUFFER_SIZE 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

int16_t txNumber;
int16_t rssi, rxSize, snr;
bool received_packet = false;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t _rssi, int8_t _snr);

void setup()
{
  Serial.begin(115200);

  txNumber = 0;
  rssi = 0;

  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  turnOnRGB(COLOR_SEND, 0); // change rgb color
  Serial.println("into RX mode");
}

void loop()
{
  Radio.Rx(0);
  delay(250);
  Radio.IrqProcess();

  if (received_packet)
  {
     Radio.Sleep();
    // Radio.Standby();

    Serial.print("[RX]");
    Serial.print("[RSSI]");
    Serial.print(rssi);
    Serial.print("dBm [SNR]");
    Serial.print(snr);
    Serial.print("[DATA][");
    Serial.print(rxSize);
    Serial.print("] "); 
    Serial.print(rxpacket);
    Serial.println();

    received_packet = false;
    turnOffRGB();
  }
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t _rssi, int8_t _snr)
{
  turnOnRGB(COLOR_RECEIVED, 0);

  rssi = _rssi;
  snr = _snr;
  rxSize = size;
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';

  received_packet = true;
}
