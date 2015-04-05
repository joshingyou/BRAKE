
/* some includes */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 115200
#include <util/setbaud.h>

#define WHITE 300
#define RED 250
#define YELLOW 225
#define GREEN 200
#define TURQUOISE 175
#define BLUE 150
#define PURPLE 125


void uart_init(void) {
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;

#if USE_2X
   UCSR0A |= _BV(U2X0);
#else
   UCSR0A &= ~(_BV(U2X0));
#endif

   UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
   UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uart_putchar(char c) {
   loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
   UDR0 = c;
}

char uart_getchar(void) {
   loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
   return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);


int main(void)
{

   /* Setup serial port */
   uart_init();
   stdout = &uart_output;
   stdin  = &uart_input;

   char input;
   int adc_value;

   // Setup ports
   DDRB |= (1<<1) | (1<<0);
   PORTB |= (1<<0);
   PORTB &= ~(1<<1);

   DDRC &= (0<<0); //temp sensor input
   DDRD |= (1<<3) | (1<<5) | (1<<6); //3, 5, 6 LED OUTPUT

   //enable ADC
   ADCSRA |= (1<<ADEN);
   //start converting
   ADCSRA |= (1<<ADSC);
   //auto trigger
   ADCSRA |= (1<<ADATE);
   //clock prescalar
   ADCSRA |= ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
   //Avcc as voltage ref
   ADMUX |= (1<<REFS0);
   ADMUX &= ~(1<<REFS1);   
   ADCSRB &= ~((1<<ADTS2) | (1<<ADTS1) | (1<<ADTS0));


   /* 
      Print hello and then display color on LED based on 
      keypress or temperature sensor readings 
   */
   printf("Hello world!\r\n");
   while(1) {
      input = getchar();
      printf("You wrote %c\r\n", input);
      PORTB ^= 0x01;

      PORTD &= 0x00;
      switch (input) {
         case 'r':      //red
            PORTD |= (1<<5);
            break;
         case 'g':      //green
            PORTD |= (1<<6);
            break;
         case 'b':      //blue
            PORTD |= (1<<3);
            break;
         case 'p':      //purple
            PORTD |= (1<<3) | (1<<5);
            break;
         case 't':      //turquoise
            PORTD |= (1<<3) | (1<<6);
            break;
         case 'y':      //yellow
            PORTD |= (1<<5) | (1<<6);
            break;
         case 'w':      //white
            PORTD |= (1<<3) | (1<<5) | (1<<6);
            break;
         case 'a':
            printf("Auto mode\r\n");
            while (1) {
               adc_value = ADCW;
               printf("Temperature value is %d\r\n", adc_value);
               PORTD &= 0x00;
               if (adc_value < PURPLE) {
                  PORTD |= (1<<3) | (1<<5);  //make purple
               } else if (adc_value >= PURPLE && adc_value < BLUE) {
                  PORTD |= (1<<3);  //make blue
               } else if (adc_value >= BLUE && adc_value < TURQUOISE) {
                  PORTD |= (1<<3) | (1<<6);  //make turquoise
               } else if (adc_value >= TURQUOISE && adc_value < GREEN) {
                  PORTD |= (1<<6);  //make green
               } else if (adc_value >= GREEN && adc_value < YELLOW) {
                  PORTD |= (1<<5) | (1<<6);  //make yellow
               } else if (adc_value >= YELLOW && adc_value < RED) {
                  PORTD |= (1<<5);  //make red
               } else {
                  PORTD |= (1<<3) | (1<<5) | (1<<6);  //make white
               }
               _delay_ms(1000);  
            }
            break;
      }
      _delay_ms(500);

   }

}
