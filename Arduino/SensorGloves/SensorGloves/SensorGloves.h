/*
 * SensorGloves.h
 *
 * Created: 3/16/2015 12:28:04 PM
 *  Author: Jiaxin
 */ 


#ifndef SENSORGLOVES_H_
#define SENSORGLOVES_H_

class FlexSensor
{
	public:
		FlexSensor(int pin, int sensor_type);
		int read();
	private:
		int _pin;
};


#endif /* SENSORGLOVES_H_ */