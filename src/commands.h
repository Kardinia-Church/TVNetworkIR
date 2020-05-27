/**

TV Network IR By Kardinia Church 2020
A ESP8266 project for controlling a TV using an IR signal

http://github.com/kardinia-church/TVNetworkIR


commands.h
Used to define the commands that are supported
*/

#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "settings.h"

IRsend irsend(IR_PIN);

//Setup pins required
void setupCommands() {
    pinMode(POWER_STATE_PIN, STATE_MODE);
    irsend.begin();
}

//Flash the debug led to show it did something
void flashDebug() {
    digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
    delay(100);
    digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
    delay(100);
    digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
    delay(100);
    digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
    delay(100);
}

//Turn the TV on or off
unsigned long lastPowerCommandSent = millis();
bool setPower(bool state) {
    Serial.print("Set power to "); Serial.print(state ? "on" : "off");
    flashDebug();
    
    //Timeout
    if(lastPowerCommandSent + (POWER_TIMEOUT_SEC * 1000) > millis()){Serial.println(" failed in timeout"); return false;}

    #ifdef INVERT_STATE
        if(!digitalRead(POWER_STATE_PIN) == state){Serial.println(" tv is already in that state"); return true;}
    #else
        if(digitalRead(POWER_STATE_PIN) == state){Serial.println(" tv is already in that state"); return true;}
    #endif

    //Send the IR
    for (int i = 0; i < 3; i++) {
        irsend.sendNEC(POWER_TOGGLE_IR, 32);
        delay(40);
    }
    lastPowerCommandSent = millis();
    Serial.println(" sent!");
    return true;
}

String processCommand(String command) {
    //Power on
    if(command == "PON"){
        return setPower(true) ? "OK" : "FAIL";
    }
    //Power off
    else if(command == "POF") {
        return setPower(false) ? "OK" : "FAIL";
    }
    //Current power
    else if(command == "CPW") {
        #ifdef INVERT_STATE
            return !digitalRead(POWER_STATE_PIN) ? "ON":"OFF";
        #else
            return digitalRead(POWER_STATE_PIN) ? "ON":"OFF";
        #endif
    }

    else {
        return "IRTVFAIL";
    }
}

#endif