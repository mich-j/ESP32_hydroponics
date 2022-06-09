/* WAŻNE
* Podłączając urządzenie do nowej sieci WiFi, jeśli zostanie zmieniony adres IP Raspberry, to należy go zaktualizować:
- w tym pliku
- w pliku tb_gateway.yaml: sudo nano /etc/thingsboard-gateway/config/tb_gateway.yaml
- w pliku mqtt.json: sudo nano /etc/thingsboard-gateway/config/mqtt.json
*/

// #define WIFI_SSID "Ecuador"
// #define WIFI_PASSWORD "slodkiekotki69"
#define WIFI_SSID "getto"
#define WIFI_PASSWORD "7387842399"
#define MQTT_SERVER "192.168.1.81"
#define MQTT_PORT 1884
#define CLIENT_USERNAME "esp32"
#define CLIENT_ID "esp32id"
#define CLIENT_PASS "hydroponika"