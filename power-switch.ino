/* 
  Power switch with Ethernet interface

  https://github.com/nuess0r/

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
#define VERSION "2024-08-24"

// Required libs
#include <stdint.h> // uint8_t type variables
#include <SPI.h>
#include <Ethernet.h>

// Included libs
#include "settings.h"

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer Server(80);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

const int ethResetPin = 8;  // pin the Ethernet shield reset pin is attached to

// Define the pins used by the relay
const int relay[2][4] = {
  {A15, A14, A13, A12},
  {21, 20, 19, 18}
};

// State of the Relais after power-on
#define NO 0  // Normaly open = initially off
#define NC 1  // Normaly closed = initially on

int relayState[2][4] = {
  {NC, NC, NC, NC},
  {NC, NC, NC, NC}
};


// Client variables 
String readString;
String htmlString;

void setup() {
  // make the LED pins outputs
  for(uint8_t i=0;i<2;i++){
    for(uint8_t j=0;j<4;j++){
      pinMode(relay[i][j], OUTPUT);
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
  IPAddress ip(192, 168, 36, 77);
  Ethernet.begin(mac, ip);
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
      htmlString += " (";
      htmlString += "Text";
      htmlString += ")";
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
      htmlString += F("?off'\">");
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
          if(readString.indexOf("GET /relay1off") > 0){
            digitalWrite(relay[1][0], LOW);
            relayState[1][0] = 0;
          }
          if(readString.indexOf("GET /relay1on") > 0){
            digitalWrite(relay[1][0], HIGH);
            relayState[1][0] = 1;
          }

          if(readString.indexOf('2') >0)//checks for 2
          {
            digitalWrite(relay[1][1], HIGH);    // set pin 5 high
            Serial.println("Led 5 On");
          }
          if(readString.indexOf('3') >0)//checks for 3
          {
            digitalWrite(relay[1][1], LOW);    // set pin 5 low
            Serial.println("Led 5 Off");
          }
          
          if(readString.indexOf('4') >0)//checks for 4
          {
            digitalWrite(relay[1][2], HIGH);    // set pin 6 high
            Serial.println("Led 6 On");
          }
          if(readString.indexOf('5') >0)//checks for 5
          {
            digitalWrite(relay[1][2], LOW);    // set pin 6 low
            Serial.println("Led 6 Off");
          }
          
           if(readString.indexOf('6') >0)//checks for 6
          {
            digitalWrite(relay[0][0], HIGH);    // set pin 7 high
            Serial.println("Led 7 On");
          }
          if(readString.indexOf('7') >0)//checks for 7
          {
            digitalWrite(relay[0][0], LOW);    // set pin 7 low
            Serial.println("Led 7 Off");
          }     
          
            if(readString.indexOf('8') >0)//checks for 8
          {
            digitalWrite(relay[0][1], HIGH);    // set pin 8 high
            Serial.println("Led 8 On");
          }
          if(readString.indexOf('9') >0)//checks for 9
          {
            digitalWrite(relay[0][1], LOW);    // set pin 8 low
            Serial.println("Led 8 Off");
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
