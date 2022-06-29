/*
  LoRa Transmitter (with Low Power Mode)

  Description:
  Sketch to Recurrently Transmit a Packet Every Predefined Interval.
  Uses Low Power Mode After Transmitting.

  Product Page:
  https://heltec.org/project/htcc-ab01/
*/

#include <Arduino.h>
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

double txNumber;

int16_t rssi, rxSize;
void DoubleToString(char *str, double double_num, unsigned int len);

#define timetillsleep 10000
#define timetillwakeup 10000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower = 1;
unsigned long start_time = 0;

#define WORKING_MODE_START 0
#define WORKING_MODE_SLEEP 1
#define WORKING_MODE_TRANSMIT 2
#define WORKING_MODE_TRANSMITTING 3
#define WORKING_MODE_SLEEP 4

uint8_t working_mode = WORKING_MODE_START;
bool packet_transmitted = false;

void onSleep();
void onWakeUp();
void OnTxDone(void);

void setup()
{
  Serial.begin(115200);

  TimerInit(&sleep, onSleep);
  TimerInit(&wakeUp, onWakeUp);

  txNumber = 0;
  rssi = 0;

  RadioEvents.TxDone = OnTxDone;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void loop()
{
  if (working_mode == WORKING_MODE_START)
  {
    Serial.println();
    Serial.println("[WM] START");

    working_mode = WORKING_MODE_TRANSMIT;
  }
  else if (working_mode == WORKING_MODE_TRANSMIT)
  {
    Serial.println("[WM] TX");

    start_time = millis();
    txNumber = millis() / 1000.0;

    sprintf(txpacket, "%s", "Hello ");     // start a package
    DoubleToString(txpacket, txNumber, 3); // add to the end of package
    Serial.printf("Sending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));

    packet_transmitted = false;

    turnOnRGB(COLOR_SEND, 0);                          // change rgb color
    Radio.Send((uint8_t *)txpacket, strlen(txpacket)); // send the package out

    working_mode = WORKING_MODE_TRANSMITTING;
  }
  else if (working_mode == WORKING_MODE_TRANSMITTING)
  {
    if (packet_transmitted)
    {
      working_mode = WORKING_MODE_SLEEP;
    }
  }
  else if (working_mode == WORKING_MODE_SLEEP)
  {
    Serial.println("[WM] SLEEP");

    onSleep();
    Serial.print("Time: ");
    Serial.print(millis());
    Serial.print("ms");
    Serial.print(", Run Time: ");
    Serial.print(millis() - start_time);
    Serial.print("ms");
    Serial.println();
    Serial.println();
    delay(20);

    working_mode = WORKING_MODE_START; // Reset Process
    Radio.Sleep();
    lowPowerHandler();
  }
}

/**
 * @brief  Double To String
 * @param  str: Array or pointer for storing strings
 * @param  double_num: Number to be converted
 * @param  len: Fractional length to keep
 * @retval None
 */
void DoubleToString(char *str, double double_num, unsigned int len)
{
  double fractpart, intpart;
  fractpart = modf(double_num, &intpart);
  fractpart = fractpart * (pow(10, len));
  sprintf(str + strlen(str), "%d", (int)(intpart));    // Integer part
  sprintf(str + strlen(str), ".%d", (int)(fractpart)); // Decimal part
}

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n", timetillwakeup);
  lowpower = 1;

  // timetillwakeup ms later wake up;
  TimerSetValue(&wakeUp, timetillwakeup);
  TimerStart(&wakeUp);
}

void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n", timetillsleep);
  lowpower = 0;

  // timetillsleep ms later into lowpower mode;
  TimerSetValue(&sleep, timetillsleep);
  TimerStart(&sleep);
}

void OnTxDone(void)
{
  Serial.println("TX Done");
  packet_transmitted = true;
  turnOffRGB();
}