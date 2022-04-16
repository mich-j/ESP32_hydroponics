#define _TIMERINTERRUPT_LOGLEVEL_     4
#define TIMER_INTERRUPT_DEBUG      0

#include <Arduino.h>
#include "ESP32_New_TimerInterrupt.h"
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include "esp32-mqtt.h"
#include <ArduinoJson.h>
#include "ESP32_New_ISR_Timer.h"


//#define _TIMERINTERRUPT_LOGLEVEL_ 4
#define WATER_OK 1
#define WATER_LOW 0

// Pins
#define OLED_CLOCK_PIN 20
#define OLED_DATA_PIN 21
#define WATER_PUMP_PIN 34
#define WATER_SENSOR_PIN 14
#define LED_PIN 5
#define FAN_PIN 35
#define AIR_PIN 36

#define TIMER0_INTERVAL_MS 3000
#define TIMER0_DURATION_MS 5000


int dht_pin = 2;

struct Device
{
  struct relays
  {
    uint8_t pin;
    uint8_t state;
  };
  relays led;
  relays air;
  relays fan;
  relays pump;
  relays water_level;

};



Device dev;

float temperature = 0;
float humidity = 0;
int DHTerror = SimpleDHTErrSuccess;

char buf[100];

// konstruktory
SimpleDHT22 dht22(dht_pin);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK_PIN, OLED_DATA_PIN, /* reset=*/U8X8_PIN_NONE);

ESP32Timer ITimer0(0);

StaticJsonDocument<200> doc;

// Function declarations
void DHT_getTempHum(void);


bool IRAM_ATTR TimerHandler0(void *timerNo)
{

  return true;
}

void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  setupCloudIoT();

  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS, TimerHandler0);

  
}

void loop()
{
  DHT_getTempHum();
  GetWaterLevel();
  

  //data for publishing
  doc["temp"] = temperature;
  doc["hum"] = humidity;
  doc["fan"] = dev.fan.state;
  doc["led"] = dev.led.state;
  doc["air"] = dev.air.state;
  doc["pump"] = dev.pump.state;
  doc["waterlevel"] = dev.water_level.state;
  
  serializeJson(doc, buf);
  publishTelemetry(buf);
}



void DHT_getTempHum(void)
{
  if ((DHTerror = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
  {
    u8g2.setCursor(0, 10);
    u8g2.print(SimpleDHTErrCode(DHTerror));
    u8g2.sendBuffer();
  }

}

// bool CheckStatesDiff(States curr, States prevy)
// {
//   int *curr_ptr;
//   int *prevy_ptr;

//   bool is_diff = false;

//   for (int i = 0; i < 3; i++)
//   {
//     if (*(curr_ptr + i) != *(prevy_ptr + i))
//     {
//       is_diff = true;
//     }
//   }

//   return is_diff;
// }

void SetRelay(uint8_t pin, bool state)
{
  digitalWrite(pin, state);

}

uint8_t GetWaterLevel(void){
  dev.water_level.state = digitalRead(WATER_SENSOR_PIN);
}