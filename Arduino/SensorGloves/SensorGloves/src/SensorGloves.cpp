/*
* SensorGloves.cpp
*
* Created: 4/13/2015 12:29:35 AM
*  Author: Jiaxin
*/

#include "SensorGloves.h"
extern "C" {
    #include "blink_leds.h"
}
#include "../lib/Arduino/Arduino.h"
#include "../lib/SparkFun_BLEMate2/SparkFun_BLEMate2.h"

static String fullBuffer;
static String inputBuffer;
static String sendBuffer;

BLEMate2 BTModu(&Serial);

#define LEFTFLEXPIN 1
#define RIGHTFLEXPIN 1
#define BPLGHEADER "BL"
#define BPRGHEADER "BR"

int counter = 1;
int i = 0;
int leftFlexReading = 0;
int rightFlexReading = 0;
int turnSignalSent = 0;

void setupPeripheralExample();

void setup()
{
    Serial.begin(9600);           // This is the BC118 default baud rate.
    #ifdef DEBUG
    Serial.println("Serial has started...");
    #endif
    delay(1000);

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
    for (i = 0; i < 10; i++) {
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
    
    #ifdef DEBUG
    Serial.println("Reset/Restore/Write Config DONE!");
    #endif
    delay(1000);
}

int find_text(String needle, String haystack) {
    int foundpos = -1;
    for (unsigned int i = 0; (i < (haystack.length() - needle.length())); i++) {
        if (haystack.substring(i,needle.length()+i) == needle) {
            foundpos = i;
        }
    }
    return foundpos;
}

int main()
{
    // Since I'm going to be reporting strings back over serial to the PC, I want
    //  to make sure that I'm (probably) not going to be looking away from the BLE
    //  device during a data receive period. I'll *guess* that, if more than 1000
    //  milliseconds has elapsed since my last receive, that I'm in a quiet zone
    //  and I can switch over to the PC to report what I've heard.
    setup();

    while(1) {
        // This is the peripheral example code.

        // When a remote module connects to us, we'll start to see a bunch of stuff.
        //  Most of that is just overhead; we don't really care about it. All we
        //  *really* care about is data, and data looks like this:
        // RCV=20 char max msg\n\r

        // The state machine for capturing that can be pretty easy: when we've read
        //  in \n\r, check to see if the string began with "RCV=". If yes, do
        //  something. If no, discard it.
        
        boolean done = false;
        while (!done && Serial.available() > 0)
        {
            digitalWrite(11,HIGH);
            inputBuffer.concat((char)Serial.read());
            delay(200);
            digitalWrite(11,LOW);
            if (find_text(String("\n\r"), inputBuffer) != -1) {
                done = true;
            }
        }

        // We'll probably see a lot of lines that end with \n\r- that's the default
        //  line ending for all the connect info messages, for instance. We can
        //  ignore all of them that don't start with "RCV=". Remember to clear your
        //  String object after you find \n\r!!!
        int line_end_pos = find_text(String("\n\r"), inputBuffer);
        inputBuffer.remove((unsigned int)line_end_pos);
        int rcv_pos = find_text(String("RCV="), inputBuffer);
        if (rcv_pos == -1) {
            inputBuffer = "";
        } else {
            inputBuffer.remove(rcv_pos,4); // Remove RCV= from front.
            #ifdef DEBUG
            Serial.println(inputBuffer);
            #endif
            
            if (inputBuffer == "LP1") {
                    Serial.println("NAV SAYS LEFT");
                    blink_left_arrow(10);
            } else if (inputBuffer == "RP2") {
                    Serial.println("NAV SAYS RIGHT");
                    blink_right_arrow(10);
            } else if (inputBuffer == "LP3") {
                    Serial.println("NAV SAYS STRAIGHT");
                    blink_top_arrow(10);
                    delay(1000);
            }
        
            fullBuffer += inputBuffer;
            inputBuffer = "";
            
        }
        
        rightFlexReading = analogRead(RIGHTFLEXPIN);
        //Serial.print("Right Flex Reading: ");
        //Serial.println(rightFlexReading);
        if (rightFlexReading > 530 && !turnSignalSent) {
            Serial.flush();
            sendBuffer.concat(BPRGHEADER);
            sendBuffer.concat("2");
            BTModu.sendData(sendBuffer);
            sendBuffer = "";
            turnSignalSent = 1;
        } else if (rightFlexReading < 530 && turnSignalSent) {
            turnSignalSent = 0;
        }
        delay(500);
    }
    return 0;
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