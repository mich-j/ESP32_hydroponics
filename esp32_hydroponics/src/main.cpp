/** v1
 * Obecnie działa:
 * Broker MQTT: Mosquitto na Raspberry Pi
 * Lokalny serwer Thingsboard na Raspberry Pi  do wyświetlania wykresów i kontrolek
 * ESP32 publikuje wiadomości z telemetrią (pomiary z sensorów oraz stan wyjść) w formacie JSON
 * 
 * https://github.com/mich-j/ESP32_hydroponics
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
#define OLED_CLOCK_PIN 22
#define OLED_DATA_PIN 21
#define WATER_PUMP_PIN 34
#define WATER_SENSOR_PIN 23
#define LED_PIN 5
#define FAN_PIN 35
#define AIR_PIN 36
#define DHT_PIN 18

#define TEMP_HOT_THRESHOLD 25.0
#define TEMP_COLD_THRESHOLD 20.0

uint8_t wifi_status = WL_IDLE_STATUS;
uint8_t lineht;

const char ssid[] = WIFI_SSID;
const char pass[] = WIFI_PASSWORD;
const char mqtt_server[] = MQTT_SERVER;
const uint16_t mqtt_port = MQTT_PORT;
const char device_name[] = "esp32telemetry";
const char data_topic[] = "/sensor/data"; // MQTT topic
const char alarm_topic[] = "alarm";
const char status_topic[] = "status";
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

void oledPrint(uint8_t cur_x, uint8_t cur_y, char *buffer)
{
  u8g2.setCursor(cur_x, cur_y);
  u8g2.print(buffer);
  u8g2.sendBuffer();

  Serial.println(buffer);
}

void setupWIFI(void)
{
  WiFi.mode(WIFI_STA);

  esp_client.setServer(mqtt_server, mqtt_port);

  oledPrint(0, 10, "connecting");

  uint8_t cnt = 0;
  while (wifi_status != WL_CONNECTED)
  {

    wifi_status = WiFi.begin(ssid, pass);
    oledPrint(cnt * 5, lineht, ".");

    delay(5000);
    cnt++;
  }
  u8g2.clear();
  Serial.print("Polaczono z siecia: ");
  Serial.println(WiFi.SSID());
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT(void)
{
  const char *client_id = CLIENT_ID;
  const char *client_username = CLIENT_USERNAME;
  const char *client_pass = CLIENT_PASS;

  // while (!client.connect("arduinoClient", client_id, client_pass))
  while (!esp_client.connected())
  {
    esp_client.connect(client_id, client_username, client_pass);
    Serial.println("connecting to broker: ");
    Serial.println(esp_client.state());
    // client.subscribe(to_device_topic);
    delay(5000);

    char msg[255];

    StaticJsonDocument<255> doc;
    doc["serialNumber"] = device_name;
    serializeJson(doc, msg);
    esp_client.publish("/sensor/connect", msg);

  }
}

void IRAM_ATTR tim_sens_isr(void)
{
  tim_sens_flg = true;
}

float temperature = 0;
float humidity = 0;
bool water_level = 0;
uint8_t pump_pwm = 0;
uint8_t led_pwm = 0;
bool fan = 0;

void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.clear();
  lineht = u8g2.getMaxCharHeight() + 10;
  Serial.begin(9600);
  Serial.println("Uruchamianie szklarni");
  Serial.printf("CPU speed: %d MHz\n", getCpuFrequencyMhz());
  esp_client.setCallback(callback);
  setupWIFI();
  setupMQTT();

  // tim_sens = timerBegin(0, )

  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);
}

void loop()
{
  char *buf = new char[255];

  if (int DHTerror = dht22.read2(&temperature, &humidity, NULL) != SimpleDHTErrSuccess)
  {
    u8g2.setCursor(0, 10);
    u8g2.print(SimpleDHTErrCode(DHTerror));
    u8g2.sendBuffer();
  }

  u8g2.clear();
  sprintf(buf, "Temp: %.1f *C", temperature);
  oledPrint(0, 10, buf);
  sprintf(buf, "Hum: %d RH", (int)humidity);
  oledPrint(0, 10 + lineht, buf);

  GetWaterLevel(&water_level);
  Serial.println(water_level);

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

  // data for publishing
  doc["serialNumber"] = device_name;
  doc["sensorType"] = "Hydroponics";
  doc["sensorModel"] = "telemetry";
  doc["temp"] = temperature;
  doc["hum"] = humidity;
  doc["led"] = led_pwm;
  doc["fan"] = fan;
  doc["waterlvl"] = water_level;
  doc["pump_pwm"] = pump_pwm;

  char mqtt_msg[255];

  serializeJson(doc, mqtt_msg);
  esp_client.publish(data_topic, mqtt_msg);

  doc.clear();


  esp_client.loop();

  delay(60000);
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
