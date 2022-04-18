/*
 * Broker MQTT: Mosquitto na Raspberry Pi
 * Lokalny serwer Thingsboard/NODE-RED/Home Assistant, do wyświetlania wykresów i kontrolek
 *
 * ESP32 publikuje wiadomości z telemetrią w formacie JSON
 *
 *
 */

#include <Arduino.h>
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "pass.h"
#include <WiFi.h>

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
#define DHT_PIN 2

#define TEMP_HOT_THRESHOLD 25.0
#define TEMP_COLD_THRESHOLD 20.0

int wifi_status = WL_IDLE_STATUS;

const char ssid[] = WIFI_SSID;
const char pass[] = WIFI_PASSWORD;
const char mqtt_server[] = MQTT_SERVER;
const uint16_t wifi_port = WIFI_PORT;
const char data_topic[] = "data"; // MQTT topic
const char alarm_topic[] = "alarm";
const char to_device_topic[] = "cmd";

hw_timer_t *tim_sens = NULL;
volatile bool tim_sens_flg = false;

// konstruktory
SimpleDHT22 dht22(DHT_PIN);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK_PIN, OLED_DATA_PIN, /* reset=*/U8X8_PIN_NONE);

WiFiClient esp32wifi;
PubSubClient esp_client(esp32wifi);

// Function declarations
void SetRelay(uint8_t pin, bool state)
{
  digitalWrite(pin, state);
}

void GetWaterLevel(bool *pstate)
{
  *pstate = digitalRead(WATER_SENSOR_PIN);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);

  uint8_t led = doc["setled"];
}

void setupWIFIandMQTT(void)
{
  WiFi.mode(WIFI_STA);

  esp_client.setServer("192.168.1.30", 1883);

  while (wifi_status != WL_CONNECTED)
  {
    wifi_status = WiFi.begin(ssid, pass);
    Serial.println("connecting to wifi: ");
    Serial.println(WiFi.localIP());
    delay(5000);
  }

  Serial.println("Polaczono z siecia");
  const char *client_id = CLIENT_ID;
  const char *client_pass = CLIENT_PASS;

  // while (!client.connect("arduinoClient", client_id, client_pass))
  while (!esp_client.connected())
  {
    esp_client.connect("esp", client_id, client_pass);
    Serial.println("connecting to broker: ");
    Serial.println(esp_client.state());
    // client.subscribe(to_device_topic);
    delay(5000);
  }
}

void IRAM_ATTR tim_sens_isr(void){
  tim_sens_flg = true;
}

float temperature = 0;
float humidity = 0;
bool water_level = 0;

void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  Serial.begin(9600);
  Serial.println("Uruchamianie szklarni");
  Serial.printf("CPU speed: %d MHz\n", getCpuFrequencyMhz());
  esp_client.setCallback(callback);
  setupWIFIandMQTT();

  //tim_sens = timerBegin(0, )
}

void loop()
{
  if (int DHTerror = dht22.read2(&temperature, &humidity, NULL) != SimpleDHTErrSuccess)
  {
    u8g2.setCursor(0, 10);
    u8g2.print(SimpleDHTErrCode(DHTerror));
    u8g2.sendBuffer();
  }

  GetWaterLevel(&water_level);

  StaticJsonDocument<255> doc;
  StaticJsonDocument<255> alrm;

  if (temperature > TEMP_HOT_THRESHOLD)
  {
    alrm["temp_alrm"] = "HOT";
  }
  else if (temperature < TEMP_COLD_THRESHOLD)
  {
    alrm["temp_alrm"] = "COLD";
  }

  else
  {
    alrm["temp_alrm"] = "OK";
  }

  char buf[256];

  // data for publishing
  doc["temp"] = temperature;
  doc["hum"] = humidity;
  // doc["fan"] =
  //     doc["led"] =
  //         doc["air"] =
  //             doc["pump"] =
  //                 doc["waterlevel"] = waterlevel;

  serializeJson(doc, buf);
  esp_client.publish(data_topic, buf);

  esp_client.loop();

  delay(10000);
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
