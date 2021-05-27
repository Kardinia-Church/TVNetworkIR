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
#include "settings.h"

#ifdef BOARD_ESP266
    #include <IRremoteESP8266.h>
    #include <IRsend.h>
    IRsend irsend(4);
#else
    #include <IRremote.h>
    IRsend irsend;
#endif


//Setup pins required
void setupCommands() {
    #ifdef DISABLE_POWER_DETECT
        pinMode(POWER_STATE_PIN, OUTPUT);
        //digitalWrite(POWER_STATE_PIN, LOW);
    #else
        pinMode(POWER_STATE_PIN, STATE_MODE);
    #endif

    #ifdef BOARD_ESP266
        irsend.begin();
    #endif
}

//Turn the TV on or off
unsigned long lastPowerCommandSent = -(POWER_TIMEOUT_SEC * 1000);
bool setPower(bool state) {
    Serial.print("Set power to "); Serial.print(state ? "on" : "off");
    digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
    
    #ifdef INVERT_STATE
        if(!digitalRead(POWER_STATE_PIN) == state){Serial.println(" tv is already in that state"); return true;}
    #else
        if(digitalRead(POWER_STATE_PIN) == state){Serial.println(" tv is already in that state"); return true;}
    #endif
    
    #ifdef DISABLE_POWER_DETECT
        #ifdef INVERT_STATE
            digitalWrite(POWER_STATE_PIN, !state);
            storePersistState(!state);
        #else
            digitalWrite(POWER_STATE_PIN, state);
            storePersistState(state);
        #endif
    #endif

    //Timeout
    if(lastPowerCommandSent + (POWER_TIMEOUT_SEC * 1000) > millis()) {
        Serial.print(" in timeout, waiting... "); 
        digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
        delay(lastPowerCommandSent + (POWER_TIMEOUT_SEC * 1000) - millis());
        digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
    }

    #ifndef OVERRIDE_IRSENDER
        //Send the IR
        for (int i = 0; i < LOOP_TIMES; i++) {
            SEND_IR;
            delay(40);
        }
    #else
        OVERRIDE_IRSENDER
    #endif
    lastPowerCommandSent = millis();
    Serial.print(" sent... ");
    
    #ifdef INVERT_STATE
        int wait = 0;
        while(!digitalRead(POWER_STATE_PIN) != state){wait++; delay(1000); if(wait > POWER_TIMEOUT_SEC){Serial.println("Failed."); digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE); return false;}}
        Serial.println("Success!");
        digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
        return true;
    #else
        int wait = 0;
        while(digitalRead(POWER_STATE_PIN) != state){wait++; delay(1000); if(wait > POWER_TIMEOUT_SEC){Serial.println("Failed."); digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE); return false;}}
        Serial.println("Success!");
        digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
        return true;
    #endif
}

//Continuously loop to check state
bool previousPowerState = false;

String processStatus() {
    if(digitalRead(POWER_STATE_PIN) != previousPowerState) {
        previousPowerState = digitalRead(POWER_STATE_PIN);

        #ifdef INVERT_STATE
            if(!digitalRead(POWER_STATE_PIN)){return "PON";} else{return "POF";}
        #else
            if(digitalRead(POWER_STATE_PIN)){return "PON";} else{return "POF";}
        #endif
    }
    
    return "";
}

String processCommand(String command) {
    //Power on
    if(command == "PON"){
        return setPower(true) ? command : "FAIL";
    }
    //Power off
    else if(command == "POF") {
        return setPower(false) ? command : "FAIL";
    }
    //Current power
    else if(command == "CPW") {
        #ifdef INVERT_STATE
            return !digitalRead(POWER_STATE_PIN) ? "PON":"POF";
        #else
            return digitalRead(POWER_STATE_PIN) ? "PON":"POF";
        #endif
    }
    //Force switch the stored state
    else if(command == "PSW") {
        storePersistState(!digitalRead(POWER_STATE_PIN));
        digitalWrite(POWER_STATE_PIN, !digitalRead(POWER_STATE_PIN));

        #ifdef INVERT_STATE
            return !digitalRead(POWER_STATE_PIN) ? "PON":"POF";
        #else
            return digitalRead(POWER_STATE_PIN) ? "PON":"POF";
        #endif
    }

    else {
        return "FAIL";
    }
}

#endif