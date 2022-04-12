#include <Arduino.h>
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include "esp32-mqtt.h"

#define _TIMERINTERRUPT_LOGLEVEL_     4

#include <ESP32TimerInterrupt.h>

// Pins
#define OLED_CLOCK 20
#define OLED_DATA 21

#define TIM_INTERVAL 3000

int dht_pin = 2;

int clock = 21;
int data = 20;

struct States{
  uint8_t led;
  uint8_t water;
  uint8_t fan;
  uint8_t pump;
};

States relay; //struktura przechowująca stany przekaźników
States pwm;

float temperature = 0;
float humidity = 0;
int DHTerror = SimpleDHTErrSuccess;

// konstruktory
SimpleDHT22 dht22(dht_pin);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK, OLED_DATA, /* reset=*/ U8X8_PIN_NONE);

ESP32Timer ITimer0(0);


void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  setupCloudIoT();

ITimer0.attachInterruptInterval(TIM_INTERVAL, TimerHandler);
  
  
}

void loop() {

  DHT_getTempHum();
}

bool IRAM_ATTR TimerHandler(void * timerNo){

  return true;
}

int DHT_getTempHum(void)
{
  if((DHTerror = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess){
    u8g2.setCursor(0, 10);
    u8g2.print(SimpleDHTErrCode(DHTerror));
    u8g2.sendBuffer();

  }
}

