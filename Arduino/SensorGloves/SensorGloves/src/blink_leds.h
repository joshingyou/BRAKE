/*
 * blink_leds.h
 *
 * Created: 4/24/2015 10:56:42 PM
 *  Author: Jiaxin
 */ 


#ifndef BLINK_LEDS_H_
#define BLINK_LEDS_H_

void setup_leds();

void blink_left_arrow();
void turn_off_left_arrow();

void blink_right_arrow();
void turn_off_right_arrow();

void blink_top_arrow();
void turn_off_top_arrow();

void blink_status_led();
void turn_off_status_led();

#endif /* BLINK_LEDS_H_ */