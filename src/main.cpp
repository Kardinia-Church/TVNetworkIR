/**

TV Network IR By Kardinia Church 2020
A ESP8266 project for controlling a TV using an IR signal

http://github.com/kardinia-church/TVNetworkIR


main.cpp
Main functionality
*/

#include <Arduino.h>

#ifdef BOARD_ESP266
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#else
  #include <UIPEthernet.h>
#endif

#include "settings.h"
#include "commands.h"

#define VERSION "2.1"
#define PACKET_SIZE 16

#ifdef BOARD_ESP266
  WiFiUDP udp;
#else
  byte mac[] = {0x1C, 0x39, 0x47, 0x00, 0x0A, 0x8F};
  EthernetUDP udp;
  void(* resetFunc) (void) = 0;
#endif

String udpPassword = DEFAULT_UDP_PASSWORD;
bool foundServer = false;
IPAddress serverIP(192, 168, 0, 5);
int id = DEFAULT_ID;

void setup() { 
  Serial.begin(SERIAL_BAUD);
  pinMode(DEBUG_LED, OUTPUT);
  digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
  Serial.println("TV Network IR By Kardinia Church");
  Serial.println("Version: " + String(VERSION));
  Serial.println("Build Date: " + String(__DATE__));
  Serial.println("");
  setupCommands();


  //Attempt to connect
  #ifdef BOARD_ESP266
    Serial.print("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      attempts++;
      if(attempts > 30) {
        Serial.println(" Failed");
        ESP.restart();
        break;
      }
    }

    if(WiFi.isConnected()) {
      Serial.print(" Connected IP: ");
      Serial.print(WiFi.localIP());

      //If we're connected to wifi open the UDP listener
      udp.begin(LISTEN_PORT);
      Serial.print(", Listening on port ");
      Serial.println(udp.localPort());
    }
  #else
    Serial.print("Connecting to Ethernet..");
    if(Ethernet.begin(mac) == 0) {
      Serial.println(" Failed to connect. Rebooting");
      delay(1000);
      resetFunc();
    }
    else {
      Serial.print(" Connected IP: ");
      Serial.print(Ethernet.localIP());
      udp.begin(LISTEN_PORT);
      Serial.print(", Listening on port ");
      Serial.println(LISTEN_PORT);
    }
  #endif

    Serial.print("This device ID: "); Serial.println(id);
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
}

//Processing the incoming UDP packets
char packetBuffer[PACKET_SIZE];
bool processIncoming() {
  int incoming = udp.parsePacket();
  //Got packet?
  if (incoming) {
    memset(packetBuffer, 0, sizeof(packetBuffer));
    udp.read(packetBuffer, PACKET_SIZE);
    IPAddress remoteIP = udp.remoteIP();

    //Check IRTV flag
    if(packetBuffer[0] != 0x49){return false;}
    if(packetBuffer[1] != 0x52){return false;}
    if(packetBuffer[2] != 0x54){return false;}
    if(packetBuffer[3] != 0x56){return false;}
    if(packetBuffer[12] != id){return false;}

    serverIP = udp.remoteIP();
    foundServer = true;

    //Check password
    for(int i = 0; i < udpPassword.length(); i++) {
      if(packetBuffer[i + 4] != udpPassword[i]) {
        //Password incorrect
        udp.beginPacket(remoteIP, ANSWER_PORT);
        udp.print("IRTV" + String(id) + "AUTHERR");
        udp.endPacket();
        return false;
      }
    }
    
    //Get the action
    String command = "";
    for(int i = 0; i < 3; i++) {command += (char)packetBuffer[i + 13];}

    Serial.println("Request incoming: " + command);

    //Process the command and send a reply
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("IRTV" + String(id) + processCommand(command));
    udp.endPacket();

    return true;
  }
}


void loop() {
  processIncoming();
  
  String status = processStatus();
  if(status != "" && foundServer == true) {
    //Send update
    Serial.println("State changed externally sending: " + status);
    udp.beginPacket(serverIP, ANSWER_PORT);
    udp.print("IRTV" + String(id) + status);
    udp.endPacket();
  }

  //If disconnected from wifi reboot
  #ifdef BOARD_ESP266
    if(WiFi.status() != WL_CONNECTED) {
      ESP.restart();
    }
  #else
    if(Ethernet.linkStatus() != LinkON) {
      resetFunc();
    }
  #endif
}
