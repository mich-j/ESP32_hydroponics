#include <Arduino.h>
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include "esp32-mqtt.h"

#define _TIMERINTERRUPT_LOGLEVEL_ 4


#include <ESP32TimerInterrupt.h>

// Pins
#define OLED_CLOCK_PIN 20
#define OLED_DATA_PIN 21
#define WATER_PUMP_PIN 34
#define FAN_PIN 35
#define AIR_PIN 36

#define TIM0_INTERVAL 3000

int dht_pin = 2;

struct States
{
  uint8_t led;
  uint8_t air;
  uint8_t fan;
  uint8_t pump;
};

States relay; // struktura przechowująca stany przekaźników
States pwm;

float temperature = 0;
float humidity = 0;
int DHTerror = SimpleDHTErrSuccess;

// konstruktory
SimpleDHT22 dht22(dht_pin);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK_PIN, OLED_DATA_PIN, /* reset=*/U8X8_PIN_NONE);

ESP32Timer ITimer0(0);

// Function declarations
void DHT_getTempHum(void);
bool IRAM_ATTR TimerHandler0(void *timerNo);

void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  setupCloudIoT();

  ITimer0.attachInterruptInterval(TIM0_INTERVAL, TimerHandler0);
}

void loop()
{

  DHT_getTempHum();
}

bool IRAM_ATTR TimerHandler0(void *timerNo)
{

  return true;
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

bool CheckStatesDiff(States curr, States prevy){
  int *curr_ptr;
  int *prevy_ptr;

  bool is_diff = false;

  for(int i = 0; i<3; i++){
    if(*(curr_ptr+i) != *(prevy_ptr+i)){
      is_diff=true;
    }
  }

  return is_diff;
}
