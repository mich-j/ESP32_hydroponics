/** v1
 * Obecnie działa:
 * Broker MQTT: Mosquitto na Raspberry Pi
 * Lokalny serwer Thingsboard na Raspberry Pi  do wyświetlania wykresów i kontrolek
 * ESP32 publikuje wiadomości z telemetrią (pomiary z sensorów oraz stan wyjść) w formacie JSON
 * Program działa w układzie nieblokującym - pomiary i publikacja wywoływana jest przerwaniem timera, w pozostałym czasie wywoływana jest metoda mqtt_client.loop(), co umożliwia nasłuchiwanie żądań ze strony serwera
 *
 * https://github.com/mich-j/ESP32_hydroponics
 *
 */

#include <Arduino.h>

#include "ESP32TimerInterrupt.h"
#include <SimpleDHT.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "pass.h"
#include <WiFi.h>
#include <string>

#define WATER_OK 1
#define WATER_LOW 0

/// Ustawienia
#define OLED_CLOCK_PIN 22
#define OLED_DATA_PIN 21
#define WATER_PUMP_PIN 2
#define WATER_SENSOR_PIN 23
#define LED_PIN 21
#define LED_CHANNEL 0
#define FAN_PIN 15
#define AIR_PIN 36
#define DHT_PIN 18

#define TEMP_HOT_THRESHOLD 25.0
#define TEMP_COLD_THRESHOLD 20.0

#define TIMER_INTERVAL 120000 // miliseconds // czas pomiędzy kolejnymi przerwaniami timera. Określa, jak często publikowana jest telemetria

// Defines
#define MODE_AUTO 0
#define MODE_MANUAL 1

uint8_t wifi_status = WL_IDLE_STATUS;
uint8_t lineht;

uint8_t MODE = MODE_MANUAL;

const char ssid[] = WIFI_SSID;
const char pass[] = WIFI_PASSWORD;
const char mqtt_server[] = MQTT_SERVER;
const uint16_t mqtt_port = MQTT_PORT;
const char device_name[] = "esp32telemetry";
const char data_topic[] = "/sensor/data"; // MQTT topic
const char alarm_topic[] = "alarm";
const char status_topic[] = "status";
const char rpc_request_topic[] = "v1/devices/me/rpc/request/setValue/+";
const char rpc_response_topic[] = "v1/devices/me/rpc/response/";

volatile bool tim_flg = false;
void callback(char *topic, byte *payload, unsigned int length);

float_t temperature = 0;
float_t humidity = 0;
uint8_t water_level = 0;
uint8_t water_pump = 0;
uint8_t led_pwm = 0;
uint8_t fan_state = 0;

// konstruktory
SimpleDHT22 dht22(DHT_PIN);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_CLOCK_PIN, OLED_DATA_PIN, /* reset=*/U8X8_PIN_NONE);

WiFiClient esp32wifi;
PubSubClient mqtt_client(esp32wifi);

ESP32Timer ITImer0(0);

// Function declarations
void SetRelay(const char *comp, uint8_t state)
{
  uint8_t pin = 0;

  if (strcmp(comp, "fan") == 0)
  {
    pin = FAN_PIN;
    fan_state = state;
  }
  if (strcmp(comp, "water_pump") == 0)
  {
    pin = WATER_PUMP_PIN;
    water_pump = state;
  }

  digitalWrite(pin, state);
}

void setPWM(uint8_t channel, uint8_t dutyCycle)
{
  ledcWrite(channel, dutyCycle);
}

void GetWaterLevel(bool *pstate)
{
  *pstate = digitalRead(WATER_SENSOR_PIN);
}

void updateAttributes()
{
  char mqtt_msg[255];
  StaticJsonDocument<255> doc;
  doc["serialNumber"] = device_name;
  doc["sensorType"] = "Hydroponics";
  doc["sensorModel"] = "telemetry";
  doc["temp"] = temperature;
  doc["hum"] = humidity;
  doc["waterlvl"] = water_level;
  doc["led_pwm"] = led_pwm;
  doc["fan_state"] = fan_state;
  doc["water_pump_state"] = water_pump;

  serializeJson(doc, mqtt_msg);
  mqtt_client.publish(data_topic, mqtt_msg);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // Funkcja wywoływana w momencie odebrania wiadomości w subskrybowanym temacie
  // Serwer publikuje tam kolejne tematy, każdy ma inną nazwę - jest to ID żądania RPC
  // Odpowiedź klienta jest publikowana w temacie o takiej samej nazwie, jak żądanie. Dzięki temu możliwe jest potwierdzanie odczytu wiadomości.

  /* Odczytanie nazwy tematu, który stanowi jego unikalny numer. Działa pod warunkiem, że ilość podtematów, począwszy od v1, jest stała xD */
  // strtok działa tak, że pierwszy znak określony w argumencie, zastępuje NULL. Zwraca wskaźnik na początek łańcucha znaków, od którego zaczęło się poszukiwanie. Jeśli chcemy szukać dalej, wywołujemy ją z argumentem NULL.
  char *request_id;
  char *ptr;
  char *methodName;
  ptr = strtok(topic, "/");

  uint8_t cnt = 0;
  while (ptr != NULL)
  {
    ptr = strtok(NULL, "/");
    cnt++;

    if (cnt == 5)
    {
      methodName = ptr;
    }

    if (cnt >= 6)
    {
      request_id = ptr;
      ptr = NULL;
    }
  }

  /* odczytanie treści wiadomości */

  if (strcmp(methodName, "setValueLED") == 0)
  {
    payload[length] = '\0'; // przekształcenie w ciąg znaków poprzez doklejenie NULL z prawej strony
    uint8_t led_pwm = atoi((char *)payload);
    Serial.printf("LED PWM: %d \n", led_pwm);
    setPWM(LED_CHANNEL, led_pwm);
  }
  else
  {
    StaticJsonDocument<1000> doc;
    deserializeJson(doc, payload);
    char const *component = doc["component"];
    uint8_t state = doc["enabled"];
    Serial.printf("Set %s in state %d \n", component, state);
    SetRelay(component, state);
    serializeJsonPretty(doc, Serial);
    Serial.print("\n");
    doc.clear();
  }

  updateAttributes();

  // /* Sekcja obsługująca odpowiedź */
  // const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 100;

  // DynamicJsonDocument jsonBuffer(capacity);
  // // jsonBuffer["method"] = "rpc_call";
  // // JsonObject params = jsonBuffer.createNestedObject("params");
  // // odpowiedź
  // // params["status"] = "ok";
  // // Odpowiedź wygląda tak: {"method": "rpc_call", "params": {"status": "ok"}}

  // // jsonBuffer["method"] = methodName;
  // // JsonObject params = jsonBuffer.createNestedObject("params");
  // // params["enabled"] = 1;
  // jsonBuffer["enabled"] = 1;

  // char response[256];
  // serializeJson(jsonBuffer, response);

  // char response_topic[256];
  // // Skopiowanie const char z adresem kanału odpowiedzi do bufora
  // strcpy(response_topic, rpc_response_topic);
  // // doklejenie do bufora numeru żądania RPC
  // strcat(response_topic, methodName);
  // strcat(response_topic, "/");
  // strcat(response_topic, request_id);
  // mqtt_client.publish(response_topic, response);
  // Serial.printf("Resp topic: %s \n", response_topic);
  // Serial.printf("Resp message: %s \n\n", response);
}

void oledPrint(uint8_t cur_x, uint8_t cur_y, char *buffer)
{
  u8g2.setCursor(cur_x, cur_y);
  u8g2.print(buffer);
  u8g2.sendBuffer();

  Serial.println(buffer); // buffer to char z nazwą tematu w którym zostanie opublikowana odpowiedź
}

void setupWIFI(void)
{
  WiFi.mode(WIFI_STA);

  mqtt_client.setServer(mqtt_server, mqtt_port);

  oledPrint(0, 10, "connecting");

  uint8_t cnt = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.hostname("ESP-host");
    WiFi.begin(ssid, pass);
    WiFi.setSleep(false);
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

  while (!mqtt_client.connected())
  {
    mqtt_client.connect(client_id, client_username, client_pass);
    Serial.println("connecting to broker: ");
    Serial.println(mqtt_client.state());

    delay(5000);
  }

  char msg[255];

  // opublikowanie w odpowiednim temacie informacji o sukcesie połączenia.
  StaticJsonDocument<255> doc;
  doc["serialNumber"] = device_name;
  serializeJson(doc, msg);
  mqtt_client.publish("/sensor/connect", msg);
  mqtt_client.subscribe(rpc_request_topic);
  mqtt_client.subscribe("v1/devices/me/rpc/request/setValueLED/+");
  mqtt_client.setCallback(callback);
}

bool IRAM_ATTR TimerHandler(void *timerNumber)
{
  tim_flg = true;

  return true;
}

void setup()
{
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.clear();
  lineht = u8g2.getMaxCharHeight() + 10;

  Serial.begin(9600);
  Serial.println("Uruchamianie szklarni");
  Serial.printf("CPU speed: %d MHz\n", getCpuFrequencyMhz());

  setupWIFI();
  setupMQTT();

  // setup pinów
  ledcSetup(LED_CHANNEL, 1000, 8);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  // setup timera
  if (ITImer0.attachInterruptInterval(TIMER_INTERVAL * 1000, TimerHandler))
  {
    Serial.println("Konfiguracja timera poprawna :)");
  }
  else
  {
    Serial.println("timer nie dziala :C");
  }
}

void loop()
{
  if (tim_flg == true)
  {
    if (WiFi.status() != WL_CONNECTED || mqtt_client.connected() != true)
    {
      // jeśli brak połączenia z WiFi, lub utracono połączenie z brokerem, próbuj ponownie
      setupWIFI();
      setupMQTT();
    }
    if (MODE == MODE_MANUAL)
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

      // formatowanie dokumentu JSON z telemetrią do publikacji
      updateAttributes();

      doc.clear();

      GetWaterLevel(&water_level);
      Serial.printf("Poziom wody : %d \n", water_level);
    }

    if (MODE == MODE_AUTO)
    {
    }

    tim_flg = false;
  } // koniec zadania uruchamianego poprzez timer

  mqtt_client.loop();
}