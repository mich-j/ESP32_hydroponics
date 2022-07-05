#include "sensors.h"

Sensor::Sensor() {}

void Sensor::Setup(int pin, int value_min, int value_max, std::string name)
{
    this->pin = pin;
    this->value_min = value_min;
    this->value_max = value_max;
    this->name = name;
    this->io=io;
}

 void Sensor::Setup(int pin, int value_min, int value_max, std::string name, int alarm_value)
 {
    this->pin = pin;
    this->value_min = value_min;
    this->value_max = value_max;
    this->name = name;
    this->alarm_value=alarm_value;
 }


int Sensor::getValue()
{
    return val;
}

void Sensor::setValue(int val)
{
    this->val = val;
}

Sensor::~Sensor()
{
}

Sensor::JsonDocument Sensor::MakeJson(){
   StaticJsonDocument<255> doc;
   doc[name]=val;
   
   return doc;
}


