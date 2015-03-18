/*
 * SensorGloves.cpp
 *
 * Created: 3/16/2015 12:25:35 PM
 *  Author: Jiaxin
 */ 


#include <avr/io.h>
#include "SensorGloves.h"

int _Analog_pin;

FlexSensor::FlexSensor(int pin, int sensor_type)
{
    pinMode(pin, INPUT);
    _Analog_pin = pin;
}

int FlexSensor::read()
{
    analogRead(_Analog_pin);
}
