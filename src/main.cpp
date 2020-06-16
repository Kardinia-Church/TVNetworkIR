/**

TV Network IR By Kardinia Church 2020
A ESP8266 project for controlling a TV using an IR signal

http://github.com/kardinia-church/TVNetworkIR


main.cpp
Main functionality
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include "settings.h"
#include "commands.h"

#define VERSION "1.3"
#define PACKET_SIZE 16

WiFiUDP udp;
String udpPassword = DEFAULT_UDP_PASSWORD;
bool foundServer = false;
IPAddress serverIP(192, 168, 0, 5);
int id = DEFAULT_ID;

//Open an AP to allow for configuration using the web ui
void openAP() {
  Serial.print("Opening AP on SSID: " + String(CONFIG_SSID) + "... ");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(CONFIG_SSID);
  //dnsServer.start(53, "*", IPAddress(192, 168, 1, 1));
  Serial.println(" Ready");
}

void setup() { 
  Serial.begin(SERIAL_BAUD);
  pinMode(DEBUG_LED, OUTPUT);
  digitalWrite(DEBUG_LED, DEBUG_LED_ON_STATE);
  Serial.println("TV Network IR By Kardinia Church");
  Serial.println("Version: " + String(VERSION));
  Serial.println("Build Date: " + String(__DATE__));
  Serial.println("");
  setupCommands();

  //Attempt to connect to wifi
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
      openAP();
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
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(DEBUG_LED, !DEBUG_LED_ON_STATE);
}

//Processing the incoming UDP packets
char packetBuffer[PACKET_SIZE];
bool processIncoming() {
  int incoming = udp.parsePacket();
  IPAddress remoteIP = udp.remoteIP();
  //Got packet?
  if (incoming) {
    memset(packetBuffer, 0, sizeof(packetBuffer));
    udp.read(packetBuffer, PACKET_SIZE);

    //Check IRTV flag
    if(packetBuffer[0] != 0x49){return false;}
    if(packetBuffer[1] != 0x52){return false;}
    if(packetBuffer[2] != 0x54){return false;}
    if(packetBuffer[3] != 0x56){return false;}

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

    //Check the TV id
    if(packetBuffer[12] != id){return false;}
    
    //Get the action
    String command = "";
    for(int i = 0; i < 3; i++) {command += (char)packetBuffer[i + 13];}

    Serial.println("Request incoming: " + command);

    //Process the command and send a reply
    udp.beginPacket(remoteIP, ANSWER_PORT);
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
  if(WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }
}
