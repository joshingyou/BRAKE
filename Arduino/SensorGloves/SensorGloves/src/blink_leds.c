/*
 * blink_leds.c
 *
 * Created: 4/24/2015 10:55:09 PM
 *  Author: Jiaxin
 */ 

#include "../lib/Arduino/Arduino.h"
#include "blink_leds.h"

boolean left_arrow_on;
boolean right_arrow_on;
boolean top_arrow_on;
boolean status_led_on;
unsigned long timer_left_arrow;
unsigned long timer_top_arrow;
unsigned long timer_right_arrow;
unsigned long timer_status_led;

void setup_leds()
{
    for (int i = 2; i < 12; i++) {
        pinMode(i, OUTPUT);
    }
    for (int i = 2; i < 12; i++) {
        digitalWrite(i, HIGH);
        delay(20);
        digitalWrite(i, LOW);
    }
}

void blink_left_arrow ()
{
    if (!left_arrow_on && (millis() - timer_left_arrow > 500)) {
            digitalWrite(7, HIGH);
            digitalWrite(3, HIGH);
            digitalWrite(2, HIGH);
            timer_left_arrow = millis();
            left_arrow_on = true;
    }
    if (left_arrow_on && (millis() - timer_left_arrow > 500)) {
            digitalWrite(7, LOW);
            digitalWrite(3, LOW);
            digitalWrite(2, LOW);
            timer_left_arrow = millis();
            left_arrow_on = false;
    }
}

void blink_right_arrow ()
{
    if (!right_arrow_on && (millis() - timer_right_arrow > 500)) {
        digitalWrite(8, HIGH);
        digitalWrite(10, HIGH);
        digitalWrite(5, HIGH);
        timer_right_arrow = millis();
        right_arrow_on = true;
    }
    if (right_arrow_on && (millis() - timer_right_arrow > 500)) {
        digitalWrite(8, LOW);
        digitalWrite(10, LOW);
        digitalWrite(5, LOW);
        timer_right_arrow = millis();
        right_arrow_on = false;
    }
}

void blink_top_arrow ()
{
    if (!top_arrow_on && (millis() - timer_top_arrow > 500)) {
        digitalWrite(4, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(6, HIGH);
        timer_top_arrow = millis();
        top_arrow_on = true;
    }
    if (top_arrow_on && (millis() - timer_top_arrow > 500)) {
        digitalWrite(4, LOW);
        digitalWrite(9, LOW);
        digitalWrite(6, LOW);
        timer_top_arrow = millis();
        top_arrow_on = false;
    }
}

void blink_status_led()
{

    if (!status_led_on && (millis() - timer_status_led > 500)) {
        digitalWrite(11, HIGH);
        timer_status_led = millis();
        status_led_on = true;
    }
    if (status_led_on && (millis() - timer_status_led > 500)) {
        digitalWrite(11, LOW);
        timer_status_led = millis();
        status_led_on = false;
    }
      
}