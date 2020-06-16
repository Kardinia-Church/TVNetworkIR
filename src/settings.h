/**

TV Network IR By Kardinia Church 2020
A ESP8266 project for controlling a TV using an IR signal

http://github.com/kardinia-church/TVNetworkIR


Settings.h
Used to define the global settings for the project
*/

#define SERIAL_BAUD 115200
#define CONFIG_SSID "ESP8266"

#define WIFI_SSID ""                                    //Default WiFi ssid
#define WIFI_PASS ""                                    //Default WiFi password

#define TV_TYPE TV_TYPE_PANASONIC
#define DEFAULT_UDP_PASSWORD ""                         //The default password for the UDP packet listener
#define DEFAULT_ID 1                                    //The tv id
#define LISTEN_PORT 2390                                //Port to listen on (this is listening to broadcast messages)
#define ANSWER_PORT 2391                                //Port to send on (this will reply on this port to the ip address that a message was sent from)

//Pins
#define DEBUG_LED LED_BUILTIN                           //Debug led pin
#define DEBUG_LED_ON_STATE LOW                          //Debug led on state
#define POWER_STATE_PIN 5                               //Power state input pin (D1)
#define INVERT_STATE                                    //Uncomment if you require the power status input to be inverted
#define STATE_MODE INPUT_PULLUP                         //Change depending if you require a internal pullup or not
#define IR_PIN 4                                        //IR output pin (D2)
//#define DISABLE_POWER_DETECT                          //Uncomment to disable the power dection. Assumes the TV is OFF when power is applied

//TV Types
#define TV_TYPE_HISENSE 1
#define TV_TYPE_PANASONIC 2

//IR commands
#if TV_TYPE == TV_TYPE_HISENSE
    #define POWER_TIMEOUT_SEC 30
    #define SEND_IR irsend.sendNEC(0xFDB04F, 32)
    #define LOOP_TIMES 3
#elif TV_TYPE == TV_TYPE_PANASONIC
    #define POWER_TIMEOUT_SEC 30
    #define SEND_IR irsend.sendPanasonic(0x4004, 0x100BCBD)
    #define LOOP_TIMES 20
#else
    #error Invalid TV Type Specified
#endif

