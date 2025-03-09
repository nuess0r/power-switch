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

// Client variables 
String readString;

const int redLed = A15;     // pin the piezo is attached to
const int greenLed = 21;    // pin the switch is attached to
const int yellowLed = 20;   // pin the yellow LED is attached to
const int ethResetPin = 8;  // pin the Ethernet shield reset pin is attached to

String relay1State = "Off";

// Define the pins used by the relais
const int relais[2][4] = {
  {A15, A14, A13, A12},
  {21, 20, 19, 18}
};

void setup() {
  // make the LED pins outputs
  for(uint8_t i=0;i<2;i++){
    for(uint8_t j=0;j<4;j++){
      pinMode(relais[i][j], OUTPUT);
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

  // turn the green LED on
  digitalWrite(relais[1][0], HIGH);
  digitalWrite(relais[0][2], HIGH);
  digitalWrite(relais[0][0], HIGH);
}

// Display dashboard page with on/off button for relay
// It also print Temperature in C and F
void dashboardPage(EthernetClient &Client) {
  Client.println(F("<!DOCTYPE HTML><html><head>"));
  Client.println(F("<meta name='apple-mobile-web-app-capable' content='yes' />"));
  Client.println(F("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />"));
//  Client.println(F("<link rel='stylesheet' type='text/css' href='https://randomnerdtutorials.com/ethernetcss.css' />"));
  Client.println(F("<title>Random Nerd Tutorials Project</title>"));
  Client.println(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>"));  
  Client.println(F("<H1>Random Nerd Tutorials Project</H1><hr /><br />"));  
  Client.println(F("<H2>Arduino with Ethernet Shield</H2>"));                                                           
  Client.println(F("<h3><a href=\"/\">Refresh</a></h3>"));
  // Generates buttons to control the relay
  Client.println("<h4>Relay 1 - State: " + relay1State + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(relay1State == "Off"){
    Client.println(F("<a href=\"/relay1on\"><button>ON</button></a>"));
  }
  // If relay is on, it shows the button to turn the output off         
  else if(relay1State == "On"){
    Client.println(F("<a href=\"/relay1off\"><button>OFF</button></a>"));                    
  }
  Client.println(F("<h3>DIY buttons<br />"));
  Client.println(F("<a href=/?on2 >ON</a>")); 
  Client.println(F("<a href=/?off3 >OFF</a>")); 
  Client.println(F("&nbsp;<a href=/?off357 >ALL OFF</a>")); 
  
  Client.println(F("<h3>Mousedown buttons<br />"));
  Client.println(F("<input type=button value=ON onmousedown=location.href='/?on4;'>")); 
  Client.println(F("<input type=button value=OFF onmousedown=location.href='/?off5;'>"));        
  Client.println(F("&nbsp;<input type=button value='ALL OFF' onmousedown=location.href='/?off3579;'>"));        
           
  Client.println(F("<h3>Mousedown radio buttons<br />"));
  Client.println(F("<input type=radio onmousedown=location.href='/?on6;'>ON</>")); 
  Client.println(F("<input type=radio onmousedown=location.href='/?off7;'>OFF</>")); 
  Client.println(F("&nbsp;<input type=radio onmousedown=location.href='/?off3579;'>ALL OFF</>"));    
  
  Client.println(F("<h3>Custom buttons<br />"));
  Client.print(F("<input type=submit value=ON style=width:100px;height:45px onClick=location.href='/?on8;'>"));
  Client.print(F("<input type=submit value=OFF style=width:100px;height:45px onClick=location.href='/?off9;'>"));
  Client.print(F("&nbsp;<input type=submit value='ALL OFF' style=width:100px;height:45px onClick=location.href='/?off3579;'>"));

  Client.println(F("</body></html>")); 
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
            digitalWrite(relais[1][0], LOW);
            relay1State = "Off";
          }
          if(readString.indexOf("GET /relay1on") > 0){
            digitalWrite(relais[1][0], HIGH);
            relay1State = "On";
          }

          if(readString.indexOf('2') >0)//checks for 2
          {
            digitalWrite(relais[1][1], HIGH);    // set pin 5 high
            Serial.println("Led 5 On");
          }
          if(readString.indexOf('3') >0)//checks for 3
          {
            digitalWrite(relais[1][1], LOW);    // set pin 5 low
            Serial.println("Led 5 Off");
          }
          
          if(readString.indexOf('4') >0)//checks for 4
          {
            digitalWrite(relais[1][2], HIGH);    // set pin 6 high
            Serial.println("Led 6 On");
          }
          if(readString.indexOf('5') >0)//checks for 5
          {
            digitalWrite(relais[1][2], LOW);    // set pin 6 low
            Serial.println("Led 6 Off");
          }
          
           if(readString.indexOf('6') >0)//checks for 6
          {
            digitalWrite(relais[0][0], HIGH);    // set pin 7 high
            Serial.println("Led 7 On");
          }
          if(readString.indexOf('7') >0)//checks for 7
          {
            digitalWrite(relais[0][0], LOW);    // set pin 7 low
            Serial.println("Led 7 Off");
          }     
          
            if(readString.indexOf('8') >0)//checks for 8
          {
            digitalWrite(relais[0][1], HIGH);    // set pin 8 high
            Serial.println("Led 8 On");
          }
          if(readString.indexOf('9') >0)//checks for 9
          {
            digitalWrite(relais[0][1], LOW);    // set pin 8 low
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
