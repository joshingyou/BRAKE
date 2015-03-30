//A way to tell _delay_ms() how fast your MCU is running. 
#define F_CPU 16000000UL 
#include "trtSettings.h" 
#include "trtkernel.c" 
#include "trtUart.c"
#include "trtUart.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>                  //watchdog timer handling 
#include <math.h>
#include <string.h>

#define BIT_0 0b00000001
#define BIT_1 0b00000010
#define BIT_2 0b00000100
#define BIT_3 0b00001000
#define BIT_4 0b00010000
#define BIT_5 0b00100000
#define BIT_6 0b01000000
#define BIT_7 0b10000000

//Serial communication library 
#define SEM_RX_ISR_SIGNAL 1
#define SEM_STRING_DONE   2   //user hit <enter>

//prepare a user-supplied FILE buffer for operation with stdio. 
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW); 
int args[2]; 

//IMU -- electronic device that measures and reports a 
//craft's velocity, orientation, and gravitational forces
#define DEV24c02      0b11010010    //device address of IMU
#define ACCEL_XOUT_H  0x3B
#define ACCEL_XOUT_L  0x3C
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_X  0x3F
#define ACCEL_ZOUT_L  0x40
#define GYRO_XOUT_H   0x43
#define GYRO_XOUT_L   0x44
#define GYRO_YOUT_H   0x45
#define GYRO_YOUT_L   0x46
#define GYRO_ZOUT_H   0x47
#define GYRO_ZOUT_L   0x48
#define PWR_MGMT_1    0x6B

#define readSensor(regH)  ((imuRead(regH) << 8) | imuRead(regH+1))

//--Init Functions -------------------------
void imuInit() {
  imuWrite (PWR_MGMT_1, 0x00); 
}

//--Need to change how it should read values through bluetooth 
//--But then again, I want to use a machine learning algorithm
//--Let's see what we can do...
void imuWrite (unsigned char reg, unsigned char value) {
  //set device address and write mode 
  i2c_start_wait(Dev24C02+I2C_WRITE); 
  i2c_write(reg); 
  i2c_write(value);
  i2c_stop(); 
}

//IMU Read/Write Methods
unsigned char imuRead (unsigned char reg) {
  unsigned char ret; 
  i2c_start_wait (Dev24C02+I2C_WRITE); 
  i2c_write(reg); 
  i2c_rep_start(Dev24C02+I2C_READ); 
  ret = i2c_readNak(); 
  i2c_stop(); 
  return ret; 
}

//IMU Functions
void imuReadAccelGyro(int * sensorArr) {
  sensorArr[0] = readSensor(ACCEL_XOUT_H);
  sensorArr[1] = readSensor(ACCEL_YOUT_H);
  sensorArr[2] = readSensor(ACCEL_ZOUT_H);
  sensorArr[3] = readSensor(GYRO_XOUT_H); 
  sensorArr[4] = readSensor(GYRO_YOUT_H); 
  sensorArr[5] = readSensor(GYRO_ZOUT_H); 
}

//ADC Functions =======================
//The result of the ADC conversion needs to be stored in 
//two registers: ADCH & ADCL 
//Initiate a conversion by setting the ADSC (ADC start conversion bit 
//in the ADCSR) 
void readADC(int * array) {
  int i; 
  for (i=0; i<5; i++) {
    //read sample
    //ADSC will stay high while the conversion is in progress
    //will automatically cleared when the conversion completes. 
    while (ADCSRA & (1 << ADSC)); 

    array[i] = ADCH; 
    //Tell the ADC which channel to convert by setting the ADMUX register. 
    //To use 8 bit resolution, set the ADLAR to HIGH(1) 
    //To use AVcc as the reference, set the REFS0 to (1) 
    if (i == 4) ADMUX = (1 << ADLAR) | (1 << REFS0); 
    else ADMUX = ((1 << ADLAR) | (1 << REFS0)) + i + 1; 
    //start the ADC conversion 
    ADCSRA |= (1 << ADSC); 
  }
}

//Read and Send sensors 
void prepareMsg (char * msg, unsigned int * sensors) {
  msg[0] = 0xA1; 
  msg[1] = 0xB2; 
  msg[2] = 0xC3; 
  msg[3] = 0xD4; 
  
  for (int i = 0; i < 11 ; i++) {
    msg[2*i + 4] = sensors[i]; 
    msg[2*i + 5] = sensors[i] >> 8; 
  }
  msg[4 + 22] = sensors[11] ; 
  msg[4 + 22 + 1] = sensors[12]; 
  
  //checksum 
  unsigned int sum = 0; 
  for (int i = 0; i < 24; i++) {
    sum += msg[4+i]; 
  }

  msg[4+24] = sum; 
  msg[4+24+1] sum >> 8; 
}

void readAndSendSensors (void * args) {
  uint32_t interval, nextSend; 
  char msg[4 + 22 + 2 + 2];       //header, data, checksum 
  int accelGyroSensors[16][6]; 
  unsigned int flexSensors[16][5]; 
  unsigned int touchSensors[16][2]; 
  unsigned int sensorsFiltered[13]; 

  interval = SECONDS2TICKS(.03);
  i2c_init(); 
  imuInit(); 
  initADC(); 

  DDRB = 0; 
  PORTB = 3; 
  
  while (1) {
    //watchdog reset 
    wdt_reset(); 
    //get 16 readings (still under the time constraint for 33Hz)
    for (int i = 0; i < 16; i++) {
      //read IMU data 
      imuReadAccelGyro(&accelGyroSensors[i]); 

      //read flex sensors
      readADC(&flexSensors[i]); 

      //Read touch sensors
      readTouch(&touchSensors[i]); 
    }
    //while not time to send 
    while (trtCurrentTime() < nextSend); 
    nextSend = trtCurrentTime() + interval; 
  
    //Concatenate data 
    for (int i = 0; i < 6 ; i ++) {
      sensorsFiltered[i] = 0; 
      for (int j = 0; j < 16; j++) {
        sensorsFiltered[i] += accelGyroSensors[j][i] >> 8; 
      }
    }
  
    for (int i = 0; i < 5; i++) {
      sensorsFiltered[6+i] = 0; 
      for (int j = 0; j < 16; j++) {
        sensorsFiltered[6+i] += flexSensors[j][i]; 
      }
    }

    for (int i = 0; i < 2; i++) {
      sensorsFiltered[11+i] = 0; 
      for (int j = 0; j < 16; j++) {
        sensorsFiltered[11+i] += touchSensors[j][i]; 
      } 
    }
    
    //send data encoded
    prepareMsg (msg, sensorFiltered); 
    fwrite(msg, 4+22+2+2, 1, stdout); 
  }
}

int main () {
  //watchdog timer 
  cli(); 
  wdt_reset();  //_watchdog_reset(); 
  WDTCSR |= (1 << WDCE) | (1 << WDE);   //setup WDT interrupt 
  WDTCSR |= (1 << WDE) | (1 << WDP2) | (1 << WDP0);     //prescaler 
  sei(); 

  //LED 
  DDRD |= BIT_2; 
  PORTD |= BIT_2; 
  blinkLED(); 

  //init the UART 
  trt_uart_init();  //in trtUart.c 
  stdout = stdin = stderr = &uart_str; 
  fprintf(stdout, "\n\r TRT Starting\n\r\n\r"); 

  //start TRT 
  trtInitKernel(80);    //80 bytes for the idle task stack 
  
  //create semaphores 
  trtCreateSemaphore (SEM_RX_ISR_SIGNAL, 0);  //uart receive ISR semaphore 
  trtCreateSemaphore (SEM_STRING_DONE, 0);    //user typed <enter> 
  
  trtCreateTask (readAndSendSensors, 2000, SECONDS2TICKS(0.0), SECONDS2TICKS(0.0), &(args[0])); 

  //idle task
  set_sleep_mode(SLEEP_MODE_IDLE); 
  sleep_enable(); 
  while(1) {
    sleep_cpu(); 
  }
}




