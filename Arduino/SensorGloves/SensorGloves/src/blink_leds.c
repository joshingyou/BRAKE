/*
 * blink_leds.c
 *
 * Created: 4/24/2015 10:55:09 PM
 *  Author: Jiaxin
 */ 

#include "../lib/Arduino/Arduino.h"
#include "blink_leds.h"

void blink_left_arrow (unsigned int times)
{
    for (unsigned int loop = 0; loop < times; loop++) {
        digitalWrite(7, HIGH);
        digitalWrite(3, HIGH);
        digitalWrite(2, HIGH);
        delay(500);
        digitalWrite(7, LOW);
        digitalWrite(3, LOW);
        digitalWrite(2, LOW);
        delay(500);
    }
}

void blink_right_arrow (unsigned int times)
{
    for (unsigned int loop = 0; loop < times; loop++) {
        digitalWrite(8, HIGH);
        digitalWrite(10, HIGH);
        digitalWrite(5, HIGH);
        delay(500);
        digitalWrite(8, LOW);
        digitalWrite(10, LOW);
        digitalWrite(5, LOW);
        delay(500);
    }
}

void blink_top_arrow (unsigned int times)
{
    for (unsigned int loop = 0; loop < times; loop++) {
        digitalWrite(4, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(6, HIGH);
        delay(500);
        digitalWrite(4, LOW);
        digitalWrite(9, LOW);
        digitalWrite(6, LOW);
        delay(500);
    }
}