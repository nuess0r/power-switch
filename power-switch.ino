/* 
  Power switch with Ethernet interface

  https://github.com/nuess0r/power-switch

  Additional contributions from:
  - 

  CONNECTION for Arduino Mega:
    Pins D21        Relais left 0
    Pins D20        Relais left 1
    Pins D19        Relais left 2
    Pins D18        Relais left 3
    Pins A15        Relais right 0
    Pins A14        Relais right 1
    Pins A13        Relais right 2
    Pins A12        Relais right 3

*/
#define VERSION "2025-03-10"

// Required libs
#include <stdint.h> // uint8_t type variables
#include <SPI.h>
#include <Ethernet.h>

// Included libs
#include "settings.h"

//---------------------------------------------------------------------------
// Network configuration

//the IP address is dependent on your network
IPAddress ip(192, 168, 36, 77);

// the router's gateway address:
IPAddress gateway(192, 168, 36, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

// the dns server ip
IPAddress dnServer(192, 168, 36, 1);

// Port you want to use (port 80 is default for HTTP):
EthernetServer Server(80);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

String relayText(uint8_t i, uint8_t j) {
  //---------------------------------------------------------------------------
  // Output descriptions
  //   20 characters per output, fill the remaining with space
  String outputText =
  //|      String 1     |      String 2     |      String 3     |      String 4     |      String 5     |      String 6     |      String 7     |      String 8     |
  F("Text 1              Text 2              Text 3              Text 4              Text 5              Text 6              Text 7              Text 8              ");

  return outputText.substring((i+1)*j*20, (i+1)*j*20+19);
}

//---------------------------------------------------------------------------
// Pin configuration

// Define the pins used by the relay
const uint8_t relay[2][4] = {
  {21, 20, 19, 18},
  {A15, A14, A13, A12}
};

// State of the Relais after power-on
#define NO 0  // Normaly open = initially off
#define NC 1  // Normaly closed = initially on

const uint8_t relayInit[2][4] = {
  {NC, NC, NC, NO},
  {NC, NC, NC, NO}
};

// Pin the Ethernet shield reset pin is attached to
const uint8_t ethResetPin = 8;

//---------------------------------------------------------------------------
// Code

// Holds the relay state during runtime
uint8_t relayState[2][4];


// Client variables 
String readString;
String htmlString;

void relayOff(uint8_t i, uint8_t j) {
  if(NO == relayInit[i][j]) {
    digitalWrite(relay[i][j], LOW);
  } else { // NC
    digitalWrite(relay[i][j], HIGH);
  }
  relayState[i][j] = 0;
}

void relayOn(uint8_t i, uint8_t j) {
  if(NO == relayInit[i][j]) {
    digitalWrite(relay[i][j], HIGH);
  } else { // NC
    digitalWrite(relay[i][j], LOW);
  }
  relayState[i][j] = 1;
}

void setup() {
  // make the LED pins outputs
  for(uint8_t i=0;i<2;i++){
    for(uint8_t j=0;j<4;j++){
      pinMode(relay[i][j], OUTPUT);
      relayState[i][j] = relayInit[i][j];
    }
  }

  // disable SD card otherwise the Ethernet shield doesn't work on the Mega
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  // Manually reset the Ethernet shield
  pinMode(ethResetPin, OUTPUT);
  digitalWrite(ethResetPin, HIGH);
  delay(50);
  digitalWrite(ethResetPin, LOW);
  delay(50);
  digitalWrite(ethResetPin, HIGH);
  delay(100);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  settingsEepromRead();
  showSettingsMenu();

  // start the Ethernet connection and the server:
  //server(user_settings.server_port);
  //Ethernet.begin(user_settings.server_mac, user_settings.server_ip);
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  Server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());

}

// Display dashboard page with on/off button for relay

void dashboardPage(EthernetClient &Client) {
  // HTML header
  Client.println(F("<!DOCTYPE HTML><html><head>"));
  Client.println(F("<meta name='apple-mobile-web-app-capable' content='yes' /> <meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' /> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
  Client.println(F("<title>Ethernet Power Switch</title>"));
  // CSS styles
  Client.println(F("<style>html, body {height: 100%; font-family: -apple-system, system-ui, system-ui, \"Segoe UI\", Roboto, \"Helvetica Neue\", \"Fira Sans\", Ubuntu, Oxygen, \"Oxygen Sans\", Cantarell, \"Droid Sans\", \"Apple Color Emoji\", \"Segoe UI Emoji\", \"Segoe UI Symbol\", \"Lucida Grande\", Helvetica, Arial, sans-serif; font-size: 14px;}"));
  Client.println(F(".bt { align-items: center; border: 0; border-radius: 100px; box-sizing: border-box; color: #ffffff; cursor: pointer; display: inline-flex;font-size: 14px; font-weight: 600; justify-content: center; line-height: 20px; max-width: 480px; min-height: 30px; min-width: 0px; overflow: hidden;"));
  Client.println(F("padding: 0px; padding-left: 20px; padding-right: 20px; text-align: center; touch-action: manipulation;transition: background-color 0.167s cubic-bezier(0.4, 0, 0.2, 1) 0s, box-shadow 0.167s cubic-bezier(0.4, 0, 0.2, 1) 0s, color 0.167s cubic-bezier(0.4, 0, 0.2, 1) 0s;user-select: none; -webkit-user-select: none; vertical-align: middle;}"));
  Client.println(F(".bt{background-color: #ccc;} .bt:hover, .bt:focus { background-color: #aaa; color: #ffffff;}"));
  Client.println(F(".bt.tg{background-color: #0A66C2;} .bt.tg:hover, .bt.tg:focus { background-color: #16437E; color: #ffffff;} .bt.tg:active { background: #09223b; color: rgb(255, 255, 255, .7);} .bt.tg:disabled { cursor: not-allowed; background: rgba(0, 0, 0, .08); color: rgba(0, 0, 0, .3);}"));
  Client.println(F(".sl {--s: 25px; height: var(--s); aspect-ratio: 2.5; width: auto; border-radius: var(--s); padding: calc(var(--s)/10); margin: calc(var(--s)/4); cursor: pointer; background: radial-gradient(farthest-side,#fff 96%,#0000) var(--_p,0%)/var(--s) content-box no-repeat, var(--_c,#ccc); box-sizing: content-box; appearance: none;"));
  Client.println(F("-moz-appearance: none; -webkit-appearance: none; vertical-align: middle; transform-origin: calc(3*var(--s)/5) 50%; transition: transform cubic-bezier(0,300,1,300) .5s,background .3s .1s ease-in;} .sl:checked {--_c: #85ff7a;--_p: 100%;}"));
  Client.println(F(".st {width: 20px; display: inline-block;}</style></head><body>"));
  // HTML body
  Client.println(F("<H1>Ethernet Power Switch</H1><hr /><input type=button class=bt value=Refresh onmousedown=\"location.href='/'\">"));
  for(uint8_t i=0;i<2;i++){
    // Generates the section for a phase
    htmlString = F("<hr /><H2>Phase ");
    htmlString += i + 1;
    Client.println(htmlString + "</H2>");
    
    for(uint8_t j=0;j<4;j++){
      // Generates buttons to control the relay
      Client.println(F(""));
      htmlString = F("<h3>Relay ");
      htmlString += j + 1;
      htmlString += " - ";
      htmlString += relayText(i, j);
      Client.println(htmlString);
      Client.println(F("</h3><span class=st>"));
      if(1 == relayState[i][j]) {
        Client.println("On");
      } else {
        Client.println("Off");
      }
      Client.println(F("</span><input type=\"checkbox\" "));
      if(1 == relayState[i][j]) {
        Client.println("checked");
      }
      Client.println(F(" class=sl "));
      htmlString = F("onmousedown=\"location.href='/relay");
      htmlString += i;
      htmlString += j;
      if(1 == relayState[i][j]) {
        htmlString += F("?off'\">");
      } else {
        htmlString += F("?on'\">");
      }
      Client.println(htmlString);
      Client.println(F("<input type=button class=\"bt tg\" value=TOGGLE "));
      htmlString = F("onmousedown=\"location.href='/relay");
      htmlString += i;
      htmlString += j;
      htmlString += F("?toggle'\"><br />");
      Client.println(htmlString);
    }
  }

  Client.println(F("<hr /><a href=https://github.com/nuess0r/power-switch >Code on Github</a></body></html>")); 
}

void loop() {
  while(Serial.available()) {  // see if there are someone is trying to edit settings via serial port
    processSerial(Serial.read());
  }
  
  // listen for incoming clients
  EthernetClient Client = Server.available();
  if (Client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (Client.connected()) {
      if (Client.available()) {
        char c = Client.read();
        Serial.write(c);
       //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string 
          readString += c;
        } 
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          dashboardPage(Client);
          break;
        }
        if (c == '\n') {
          // Check if we have a valid request
          //example: GET /relay02?off'
          for(uint8_t i=0;i<2;i++){
            for(uint8_t j=0;j<4;j++){
              htmlString = F("GET /relay");
              htmlString += i;
              htmlString += j;
              // TODO distinguish between NC and NO wiring
              if(readString.indexOf(htmlString + "?off") > 0){
                relayOff(i,j);
              }
              if(readString.indexOf(htmlString + "?on") > 0){
                relayOn(i,j);
              }
              if(readString.indexOf(htmlString + "?toggle") > 0){
                // TODO proper toggle with timer
                // what to do, if relay was off?
                relayOff(i,j);
                delay(100);
                relayOn(i,j);
              }
            }
          }

          // you're starting a new line
          currentLineIsBlank = true;

          //clearing string for next read
          readString="";
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    Client.stop();
    Serial.println("Client disconnected");
  }
}
