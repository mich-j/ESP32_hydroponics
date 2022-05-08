# ESP32_hydroponics
IoT hydroponics setup with telemetry, built inside IKEA SAMLA box. 

ESP32 Devkit module was used to process data from sensors and publish messages with telemetry via MQTT.

Raspberry Pi 3B+ hosts Thingsboard server, which allows to store sensors' output and display a dashboard with charts and gauges, accessible within local network. Raspberry also works as a MQTT broker and gateway for communication with Thingsboard.

Code written in Arduino Framework for ESP32.

### Features
- Air temperature and humidity measurement
- Built as a Nutrient Film Technique (NFT) system
- Remote control via MQTT protocol
- Dashboard with gauges and charts
- PWM Pump flow control
- Low water level alert
- Fan for controlling humidity and supplying oxygen
- OLED screen for displaying system status

<skrin dashboardu>
  
### Used software:
- Mosquitto MQTT broker: https://mosquitto.org/
- Thingsboard Community server: https://thingsboard.io/

### Used libraries:
- SimpleDHT: https://github.com/winlinvip/SimpleDHT 
- u8g2: https://github.com/olikraus/u8g2
- ESP32 Timer Interrupt: https://github.com/khoih-prog/ESP32TimerInterrupt
- PubSub Client: https://github.com/knolleary/pubsubclient
- ArduinoJson: https://github.com/bblanchon/ArduinoJson

