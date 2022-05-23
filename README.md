# ESP32_hydroponics: IoT hydroponics setup with telemetry

System designed for easy crop cultivation at home, especially herbs and fast-growing small plants like lettuce. Three channels, with continous flow of a nutrient solution, are available, allowing for up to 12 seedlings to be grown at the same time.

![schematic](/images/system_schematic_en.png)

ESP32 Devkit module was used to process data from sensors and publish messages with telemetry via MQTT.

Code was written using Arduino Framework for ESP32.

### Features
- Air temperature and humidity measurement
- Built as a Nutrient Film Technique (NFT) system
- Remote control via MQTT protocol
- Dashboard with gauges and charts
- PWM Pump flow control
- Low water level alert
- Fan for controlling humidity and supplying oxygen
- OLED screen for easy operation control

![administrator's dashboard](/images/dashboard_home_v2.png)
_Dashboard for system's administrator_
  
 ### Communication 

Raspberry Pi 3B+ hosts Thingsboard server, which allows to store sensors' output in a database and display a dashboard with charts and gauges, accessible within local network. User can turn on/off water pump and fan. Moreover, LED intensity can be controlled with a dimmer. Raspberry also works as a MQTT broker and a gateway for communication with Thingsboard.

 ![communication schema](/images/communication_flowchart_en.png)
  
 
### Used software:
- [Mosquitto MQTT broker](https://mosquitto.org/)
- [Thingsboard Community server](https://thingsboard.io/)

### Used libraries:
- [SimpleDHT](https://github.com/winlinvip/SimpleDHT) 
- [u8g2](https://github.com/olikraus/u8g2)
- [ESP32 Timer Interrupt](https://github.com/khoih-prog/ESP32TimerInterrupt)
- [PubSub Client](https://github.com/knolleary/pubsubclient)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

### Built prototype

stay tuned

## To-do
- [x] Setting up MQTT broker on RPi 
- [x] Setting up Thingsboard server and Gateway on RPi 
- [x] Sending telemetry to Thingsboard server
- [x] RPC implementation - controlling GPIO and PWM from dashboard
- [ ] Building and running actual prototype
