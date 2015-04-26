/*
 * BackpackLED.h
 *
 * Created: 4/12/2015 1:48:43 PM
 *  Author: Jiaxin
 */ 


#ifndef BACKPACKLED_H_
#define BACKPACKLED_H_

#define CLK 11  // MUST be on PORTB! (Use pin 11 on Mega)
#define OE  9
#define LAT 12
#define A   A0
#define B   A1
#define C   A2
#define D   A3

void setup();
void doCentralExample();


#endif /* BACKPACKLED_H_ */