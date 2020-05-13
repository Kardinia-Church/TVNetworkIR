/*
 * Basic Network IR Tv Controller 2020
 * Connect IR LED to pin 3
 * 
 * Will send and keep track of the state of the tv by sending the IR power toggle command below
 * For a Hisense TV
 * 
 * Please connect both this unit and the tv on the same power source to ensure they stay in sync!
 * 
 * Commands:
 *  ipAddress/action=<command>
 * 
 *  on = Turns the TV on
 *  off = Turns the TV off
 *  toggle = Toggles the TV power
 *  get = Returns the state of the TV power
 *  forceon = Forces the TV on
 *  forceoff = Forces the TV off
 *  changestateoff = Sets the internal flag to the TV is off
 *  changestateon = Sets the internal flag to the TV is on
 */

#include <Arduino.h>
#include <UIPEthernet.h>
#include <IRremote.h>

#define POWER_TOGGLE_IR 0xFDB04F
#define DELAY_BETWEEN_COMMANDS_SEC 30

byte mac[] = { 0x90, 0xA2, 0xFA, 0x0D, 0x78, 0xEE  };                                  
IPAddress ip(10, 1, 10, 50);                  
EthernetServer server(80);
bool state = false;

void setup() { 
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  Serial.print("MAC Address: ");
  for(int i = 0; i < 6; i++){Serial.print(mac[i], HEX); Serial.print(" ");}
  Serial.println("");
}

unsigned long lastCmdSent = -(DELAY_BETWEEN_COMMANDS_SEC * 1000);
bool sendIRPower(bool power, bool force=false) {
  if(state != power || force) {
      if(lastCmdSent + (DELAY_BETWEEN_COMMANDS_SEC * 1000) > millis()) {
        return false;
      }
      
      Serial.print("Sent IR.");
      lastCmdSent = millis();
      IRsend irsend;
      state = power;
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(POWER_TOGGLE_IR, 32);
        delay(40);
      }
      Serial.println(state);
      return true;
  }
  return true;
}

void loop() {
  //IRsend irsend;
  EthernetClient client = server.available();
  if(client) {
    String incoming = "";
    while(client.connected()) {
      if(client.available()) {
        char c = client.read();
        if(c == '\n') {
          //We only care about processing the first line in the HTTP request
          if(incoming.indexOf("GET") == 0) {
            String url = incoming.substring(incoming.indexOf("/") + 1, incoming.indexOf("HTTP") - 1);

            //Perform an action
            if(url.indexOf("action=") != -1) {
              
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");

              
              String action = url.substring(url.indexOf("=") + 1, url.length());

              if(action == "on") {
                if(sendIRPower(true)) {
                  client.println("OK");
                }
                else {
                  client.println("FAIL");
                  client.println("TOOMANYCOMMANDS");
                }
              }
              else if(action == "off") {
                if(sendIRPower(false)) {
                  client.println("OK");
                }
                else {
                  client.println("FAIL");
                  client.println("TOOMANYCOMMANDS");
                }
              }
              else if(action == "toggle") {
                if(sendIRPower(!state)) {
                  client.println("OK");
                }
                else {
                  client.println("FAIL");
                  client.println("TOOMANYCOMMANDS");
                }
              }
              else if(action == "get") {
                client.println(state ? "ON":"OFF");
              }
              else if(action == "forceon") {
                if(sendIRPower(true, true)) {
                  client.println("OK");
                }
                else {
                  client.println("FAIL");
                  client.println("TOOMANYCOMMANDS");
                }
              }
              else if(action == "forceoff") {
                if(sendIRPower(false, true)) {
                  client.println("OK");
                }
                else {
                  client.println("FAIL");
                  client.println("TOOMANYCOMMANDS");
                }
              }
              else if(action == "changestateoff") {
                client.println("OK");
                state = false;
              }
              else if(action == "changestateon") {
                client.println("OK");
                state = true;
              }
              else {
                client.println("ERROR");
              }
              client.println("</html>");
            }
            client.stop();
          }
          break;
        }
        else {
          incoming += c;
        }
      }
    }
  }
}
