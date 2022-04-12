#include <Arduino.h>
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include "secrets.h"
#include <ESP32TimerInterrupt.h>

// Pins
#define OLED_CLOCK 20
#define OLED_DATA 21

int dht_pin = 2;

int clock = 21;
int data = 20;

struct States{
  uint8_t led;
  uint8_t water;
  uint8_t fan;
  uint8_t pump;
};

States relay;
States pwm;

float temperature = 0;
float humidity = 0;
int DHTerror = SimpleDHTErrSuccess;

// konstruktory
SimpleDHT22 dht22(dht_pin);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK, OLED_DATA, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
}

void loop() {


}

int DHT_getTemp(void)
{
  if((DHTerror = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess){
    u8g2.setCursor(0, 10);
    u8g2.print(SimpleDHTErrCode(DHTerror));
    u8g2.sendBuffer();

  }
}

