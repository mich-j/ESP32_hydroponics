#ifndef sensors_h
#define sensors_h

#include<string>
#include<ArduinoJson.h>

#define OUTPUT 0x02
#define INPUT_PULLUP 0x05

class Sensor
{
private:
    int pin;
    int value_min;
    int value_max;
    std::string name;
    int val;
    int alarm_value;
    int io;

public:
    Sensor(){}
    void Setup(int pin, int io, int value_min, int value_max, std::string name);
    void Setup(int pin, int value_min, int value_max, std::string name, int alarm_value);
    int getValue();
    void setValue(int val);
    ~Sensor();
    using JsonDocument = StaticJsonDocument<255>;

    JsonDocument MakeJson();
};


#endif