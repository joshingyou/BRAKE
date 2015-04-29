/*
 * BackpackLED.cpp
 * 
 * This is the main control code for the controller microprocessor on the
 * backpack. 
 *
 * Created: 4/12/2015 1:39:40 PM
 *  Author: Josh Yu
 */ 

#include "BackpackLED.h"
#include "../lib/Arduino/Arduino.h"
#include "../lib/SparkFun_BLEMate2/SparkFun_BLEMate2.h"    // BLE Library
#include "../lib/Adafruit_GFX/Adafruit_GFX.h"              // Core graphics library
#include "../lib/Adafruit_RGBMatrix/RGBmatrixPanel.h"      // LED hardware library

// Private functions
void initMatrix();
void initBluetooth();
void drawMatrix(int msgNum);

static String fullBuffer = "";
static String inputBuffer;
static String sendBuffer;
boolean central = true;

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

BLEMate2 BTModu(&Serial1);

unsigned long serial_task_last_run = 0;
unsigned long matrix_draw_last_run = 0;

#define SERIAL_TASK_PERIOD 1000
#define MATRIX_DRAW_PERIOD 1000
void do_serial_task();
void do_matrix_draw_task();

boolean show_left_arrow = false;
boolean show_right_arrow = false;
boolean show_stop_sign = false;

void setup()
{
    Serial.begin(9600);     // This is the PC Communication Rate.
    Serial1.begin(9600);    // This is the BC118 default baud rate.

    #ifdef DEBUG
    Serial.println("Serial has started...");
    #endif
    delay(1000);

    initMatrix();
    initBluetooth();
    
    #ifdef DEBUG
    Serial.println("Reset/Restore/Write Config DONE!");
    #endif
    delay(1000);
}

void loop()
{
    if (millis() > (serial_task_last_run + SERIAL_TASK_PERIOD)) {
        //Serial.println("Serial runs");
        do_serial_task();
        serial_task_last_run = millis();
    }

    if (millis() > (matrix_draw_last_run + MATRIX_DRAW_PERIOD)) {
        //Serial.println("Flex read runs");
        do_matrix_draw_task();
        matrix_draw_last_run = millis();
    }
}

void do_serial_task()
{
    while (Serial1.available() > 0)
    {
        inputBuffer.concat((char)Serial1.read());
        delay(50);
    }
        
    // When a remote module connects to us, we'll start to see a bunch of stuff.
    //  Most of that is just overhead; we don't really care about it. All we
    //  *really* care about is data, and data looks like this:
    // RCV=20 char max msg\n\r

    // The state machine for capturing that can be pretty easy: when we've read
    //  in \n\r, check to see if the string began with "RCV=". If yes, do
    //  something. If no, discard it.
    if (inputBuffer.endsWith("\n\r")) {
        if (inputBuffer.startsWith("RCV=")) {
            inputBuffer.trim();
            inputBuffer.remove(0, 4);
            //Serial.println(inputBuffer);
            //left_arrow_on = true;
            //inputBuffer = "";
            goto parse_message;
            } else {
            inputBuffer = "";
        }
        } else {
        inputBuffer = "";
    }
    
    parse_message:

    // what kind of messages go to glove? navigation, ack from backpack,
    // check if the input buffer is intended for the left or right glove
    // check if the message is coming from the phone or backpack
    // then check flex sensors for any special readings for sending to backpack

    //for acknowledgement: if there's already been an acknowledgment for
    
    if (inputBuffer.startsWith("BL1")) {
        Serial.println("TO BACKPACK: SHOW LEFT TURN SIGNAL");
        show_left_arrow = true;
        show_right_arrow = false;
        show_stop_sign = false;
        //do some ack stuff here
        Serial.flush();
        sendBuffer.concat("LB1");
        BTModu.sendData(sendBuffer);
        sendBuffer = "";
        } else if (inputBuffer.startsWith("BR2")) {
        Serial.println("TO BACKPACK: SHOW RIGHT TURN SIGNAL");
        show_right_arrow = true;
        show_left_arrow = false;
        show_stop_sign = false;
        //do some ack stuff here
        Serial.flush();
        sendBuffer.concat("RB2");
        BTModu.sendData(sendBuffer);
        sendBuffer = "";
        } else if (inputBuffer.startsWith("BL3")) {
        Serial.println("TO BACKPACK: SHOW STOP SIGN");
        show_left_arrow = false;
        show_right_arrow = false;
        show_stop_sign = true;
        //do some ack stuff here
        Serial.flush();
        sendBuffer.concat("LB3");
        BTModu.sendData(sendBuffer);
        sendBuffer = "";
    }
}


void initMatrix()
{
    matrix.begin();
    matrix.setCursor(7, 0);
    matrix.setTextSize(1);
    matrix.setTextColor(matrix.Color333(7, 7, 7));
    
    matrix.print("H");
    matrix.print("I");
    matrix.print("!");

    delay(3000);

    matrix.fillRect(0, 0, 31, 31, matrix.Color333(0, 0, 0));
    delay(500);
}

void initBluetooth()
{
    // Regarding function return values: most functions that interact with the
    //  BC118 will return BLEMate2::opResult values. The possible values here
    //  are:
    //  REMOTE_ERROR - No remote devices exist.
    //  INVALID_PARAM - You've called the function with an invalid parameter.
    //  TIMEOUT_ERROR - The BC118 failed to respond to the command in a timely
    //                   manner; timely is redefined for each command.
    //  MODULE_ERROR - The BC118 didn't like the command string it received.
    //                  This will probably only occur when you attempt to send
    //                  commands and parameters outside the built-ins.
    //  SUCCESS - What it says.


    boolean firstResetSuccess = false;
    boolean restoreSuccess = false;
    boolean writeConfigSuccess = false;
    boolean secondResetSuccess = false;
    for (int i = 0; i < 10; i++) {
        // Reset is a blocking function which gives the BC118 a few seconds to reset.
        //  After a reset, the module will return to whatever settings are in
        //  non-volatile memory. One other *super* important thing it does is issue
        //  the "SCN OFF" command after the reset is completed. Why is this important?
        //  Because if the device is in central mode, it *will* be scanning on reset.
        //  No way to change that. The text traffic generated by the scanning will
        //  interfere with the firmware on the Arduino properly identifying response
        //  strings from the BC118.
        if (BTModu.reset() == BLEMate2::SUCCESS)
        {
            firstResetSuccess = true;
        } else
        {
            goto setup_try_again;
        }
        // restore() resets the module to factory defaults; you'll need to perform
        //  a writeConfig() and reset() to make those settings take effect. We don't
        //  do that automatically because there may be things the user wants to
        //  change before committing the settings to non-volatile memory and
        //  resetting.
        if (BTModu.restore() == BLEMate2::SUCCESS)
        {
            restoreSuccess = true;
        } else
        {
            goto setup_try_again;
        }
        // writeConfig() stores the current settings in non-volatile memory, so they
        //  will be in place on the next reboot of the module. Note that some, but
        //  not all, settings changes require a reboot. It's probably in general best
        //  to write/reset when changing anything.
        if (BTModu.writeConfig() == BLEMate2::SUCCESS)
        {
            writeConfigSuccess = true;
        } else
        {
            goto setup_try_again;
        }

        // One more reset, to make the changes take effect.
        // NB!!!!!!!!!!!!! This write/reset thing is *really* important.
        //  The status command (STS) and the LEDs *will* lie to you and tell you that
        //  you are e.g. advertising or in central mode when in fact that is not the
        //  case and the module still needs to be reset before that is actually true.
        if (BTModu.reset() == BLEMate2::SUCCESS)
        {
            secondResetSuccess = true;
        } else
        {
            goto setup_try_again;
        }

        setup_try_again:
        #ifdef DEBUG
        if (!firstResetSuccess) {
            Serial.println("Module reset error!");
            } else if (!restoreSuccess) {
            Serial.println("Module restore error!");
            } else if (!writeConfigSuccess) {
            Serial.println("Module write config error!");
            } else if (!secondResetSuccess) {
            Serial.println("Second module reset error!");
            } else {
            Serial.println("Reset/Restore/Write Config PASSED...");
            break;
        }
        #endif
    }
    if (!(firstResetSuccess && restoreSuccess && writeConfigSuccess && secondResetSuccess)) {
        // Setup failed after 10 tries
        #ifdef DEBUG
        Serial.println("Reset/Restore/Write Config FAILED 10 TIMES. STOP EXECUTION...");
        #endif
        while(1) {}
    }
}

void do_matrix_draw_task()
{
    if (show_left_arrow) {
        drawMatrix(1);
    } else if (show_right_arrow) {
        drawMatrix(2);
    } else if (show_stop_sign) {
        drawMatrix(3);
    } else {        
        drawMatrix(-1);
    }
}

void drawMatrix(int msgNum)
{
    static unsigned long matrix_last_updated = 0;
    static boolean matrix_on = false;
    static boolean done_drawing = false;

    if ((!matrix_on) && ((millis() - matrix_last_updated) > 500)) {
        if (!done_drawing) {
            switch (msgNum) {
                case 1:
                //turn left, draw a left arrow and a horizontal line
                    matrix.drawLine(27, 15, 5, 15, matrix.Color333(4, 7, 5));
                    matrix.drawLine(16, 4, 6, 14, matrix.Color333(4, 7, 5));
                    matrix.drawLine(16, 26, 6, 16, matrix.Color333(4, 7, 5));
                    done_drawing = true;
                    break;
                case 2:
                //turn right, draw a right arrow and a horizontal line
                    matrix.drawLine(5, 15, 27, 15, matrix.Color333(4, 7, 5));
                    matrix.drawLine(16, 4, 26, 14, matrix.Color333(4, 7, 5));
                    matrix.drawLine(16, 26, 26, 16, matrix.Color333(4, 7, 5));
                    done_drawing = true;
                    break;
                case 3:
                //draw a stop sign
                    matrix.fillCircle(15, 15, 15, matrix.Color333(7, 0, 0));
        
                    //STOP text
                    matrix.setCursor(4, 12);
                    matrix.setTextSize(1);
                    matrix.setTextColor(matrix.Color333(0, 0, 0));
        
                    //matrix.print("S"); matrix.print("T"); matrix.print("O"); matrix.print("P");
                    matrix.print("STOP");
                    done_drawing = true;
                    break;
                default:
                    matrix.fillRect(0, 0, 31, 31, matrix.Color333(0, 0, 0));
                    done_drawing = true;
                    break;
            }
        }        
        matrix_on = true;
        matrix_last_updated = millis();
    }
        
    if (matrix_on && ((millis() - matrix_last_updated) > 200)) {
        matrix.fillRect(0, 0, 31, 31, matrix.Color333(0, 0, 0));
        done_drawing = false;
        matrix_on = false;
        matrix_last_updated = millis();
    }
}

// The default settings are good enough for the peripheral example; just to
//  be on the safe side, we'll check the amICentral() function and do a r/w/r
//  if we're in central mode instead of peripheral mode.
void setupPeripheralExample()
{
    boolean inCentralMode = false;
    // A word here on amCentral: amCentral's parameter is passed by reference, so
    //  the answer to the question "am I in central mode" is handed back as the
    //  value in the boolean passed to it when it is called. The reason for this
    //  is the allow the user to check the return value and determine if a module
    //  error occurred: should I trust the answer or is there something larger
    //  wrong than merely being in the wrong mode?
    BTModu.amCentral(inCentralMode);
    if (inCentralMode)
    {
        BTModu.BLEPeripheral();
        BTModu.BLEAdvertise();
    }

    // There are a few more advance settings we'll probably, but not definitely,
    //  want to tweak before we reset the device.

    // The CCON parameter will enable advertising immediately after a disconnect.
    BTModu.stdSetParam("CCON", "ON");
    // The ADVP parameter controls the advertising rate. Can be FAST or SLOW.
    BTModu.stdSetParam("ADVP", "FAST");
    // The ADVT parameter controls the timeout before advertising stops. Can be
    //  0 (for never) to 4260 (71min); integer value, in seconds.
    BTModu.stdSetParam("ADVT", "0");
    // The ADDR parameter controls the devices we'll allow to connect to us.
    //  All zeros is "anyone".
    BTModu.stdSetParam("ADDR", "000000000000");

    BTModu.writeConfig();
    BTModu.reset();

    // We're set up to allow anything to connect to us now.
}

