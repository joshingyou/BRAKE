/* Name: main.c
 * Author: Ji Hye Lee 
 * Hardware: Atmega328 
 */

#include <avr/io.h>
#include <util/delay.h>     //for _delay_ms() 

//These are artifical numbers 
#define FLEX_THUMB  0x3B
#define FLEX_INDEX  0x3C
#define FLEX_MIDDLE 0x3D

int main(void)
{
    /* insert your hardware initialization here */
    //PORTD (Arduino digital pins 0 to 7) 
    
    /* DDRD: The Port D Data Direction Register */ 
    //1 (output). 0 (input) 

    //for(;;){
        /* insert your main loop code here */
    //}


    //Testing - Try to make LED lit up 
    DDRD = 1 << 4; 
    for (;;) {
      char i; 
      for (i=0; i<10; i++) {
        _delay_ms(30);    //max is 262.14 ms 
      }
      PORTD ^= 1 << 4;    //Toggle the led 
    }

    return 0;   /* never reached */
}



/*  Reads sensor data from flex sensor. 
 */ 
//int readSensor() {}

//Sensor Read Functions
void ReadFlexSensors (int * sensorArr) {
  sensorArr[0] = readSensor(FLEX_THUMB); 
  sensorArr[1] = readSensor(FLEX_INDEX); 
  sensorArr[2] = readSensor(FLEX_MIDDLE); 
}
