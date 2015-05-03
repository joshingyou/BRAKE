/*
 * blink_leds.c
 *
 * Created: 4/24/2015 10:55:09 PM
 *  Author: Jiaxin
 */ 

#include "../lib/Arduino/Arduino.h"
#include "blink_leds.h"

boolean left_arrow_is_on;
boolean right_arrow_is_on;
boolean top_arrow_is_on;
boolean status_led_is_on;

unsigned long timer_left_arrow = 0;
unsigned long timer_top_arrow = 0;
unsigned long timer_right_arrow = 0;
unsigned long timer_status_led = 0;

void setup_leds()
{
    for (int i = 2; i < 12; i++) {
        pinMode(i, OUTPUT);
    }
    for (int i = 2; i < 12; i++) {
        digitalWrite(i, HIGH);
        delay(500);
        digitalWrite(i, LOW);
    }
}

void blink_left_arrow ()
{ 
    if ((!left_arrow_is_on) && (millis() - timer_left_arrow > 500)) {
            digitalWrite(7, HIGH);
            digitalWrite(3, HIGH);
            digitalWrite(2, HIGH);
            timer_left_arrow = millis();
            left_arrow_is_on = true;
    }
    if (left_arrow_is_on && (millis() - timer_left_arrow > 500)) {
            digitalWrite(7, LOW);
            digitalWrite(3, LOW);
            digitalWrite(2, LOW);
            timer_left_arrow = millis();
            left_arrow_is_on = false;
    }
}

void turn_off_left_arrow()
{
    digitalWrite(7, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    timer_left_arrow = 0;
    left_arrow_is_on = false;
}

void blink_right_arrow ()
{
    if ((!right_arrow_is_on) && (millis() - timer_right_arrow > 500)) {
        digitalWrite(8, HIGH);
        digitalWrite(10, HIGH);
        digitalWrite(5, HIGH);
        timer_right_arrow = millis();
        right_arrow_is_on = true;
    }
    if (right_arrow_is_on && (millis() - timer_right_arrow > 500)) {
        digitalWrite(8, LOW);
        digitalWrite(10, LOW);
        digitalWrite(5, LOW);
        timer_right_arrow = millis();
        right_arrow_is_on = false;
    }
}

void turn_off_right_arrow()
{
    digitalWrite(8, LOW);
    digitalWrite(10, LOW);
    digitalWrite(5, LOW);
    timer_right_arrow = 0;
    right_arrow_is_on = false;
}

void blink_top_arrow ()
{
    if ((!top_arrow_is_on) && (millis() - timer_top_arrow > 500)) {
        digitalWrite(4, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(6, HIGH);
        timer_top_arrow = millis();
        top_arrow_is_on = true;
    }
    if (top_arrow_is_on && (millis() - timer_top_arrow > 500)) {
        digitalWrite(4, LOW);
        digitalWrite(9, LOW);
        digitalWrite(6, LOW);
        timer_top_arrow = millis();
        top_arrow_is_on = false;
    }
}

void turn_off_top_arrow()
{
    digitalWrite(4, LOW);
    digitalWrite(9, LOW);
    digitalWrite(6, LOW);
    timer_top_arrow = 0;
    top_arrow_is_on = false;
}

void turn_on_status_led()
{

    digitalWrite(11, HIGH);
    status_led_is_on = true;
    /*
    if ((!status_led_is_on) && (millis() - timer_status_led > 500)) {
        digitalWrite(11, HIGH);
        timer_status_led = millis();
        status_led_is_on = true;
    }
    if (status_led_is_on && (millis() - timer_status_led > 500)) {
        digitalWrite(11, LOW);
        timer_status_led = millis();
        status_led_is_on = false;
    }*/
      
}

void turn_off_status_led()
{
    digitalWrite(11, LOW);
    timer_status_led = 0;
    status_led_is_on = false;
}
