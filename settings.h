#include "HardwareSerial.h"
#include <avr/wdt.h>
#include <EEPROM.h>
#include <Ethernet.h>

/// Defaults

// change this whenever the saved settings are not compatible with a change
// it force a load from defaults.
#define SETTINGS_VERSION 1

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
#define MAC  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}
#define IP {192, 168, 1, 177}
#define NETMASK {255, 255, 255, 0}

enum ErrorNums{
	ERR_SETTING_NUM,
	ERR_SETTING_RANGE
};

typedef struct {
	uint8_t settings_version; // stores the settings format version 	
	
	IPAddress server_ip;
  IPAddress server_netmask;
	uint8_t server_port;
  uint8_t server_mac[6];
	
}settings_t;

settings_t user_settings;

#define READ_BUFFER_LEN 8
#define CARRIAGE_RETURN 13
char readBuffer[READ_BUFFER_LEN];
uint8_t readIndex = 0;

long lastInputTime = 0;

void resetCpu();
void processSerial(char inChar);
void showSettingsMenu();
void resetSettings();
void changeSetting(char *line);
void settingsEepromWrite();
void printError(int reason);


void resetCpu()
{
  wdt_enable(WDTO_15MS);
  while(1)
  {
    // wait for it...boom
  }
}

void processSerial(char inChar)
{
	readBuffer[readIndex] = inChar;
	
		switch(readBuffer[readIndex]) {
			case '?':// show settings
				readIndex = 0;
				Serial.print(F("ETHER POWER SWITCH VERSION: ")); Serial.println(F(VERSION));
				showSettingsMenu();
				return;
			break;
			
			case 'R': // reset to defaults
				readIndex = 0;
				resetSettings();
				return;
			break;
			
			case '!': // reboot
				resetCpu();
			break;
			
			default:
			break;
		}
	
		if (readBuffer[readIndex] == CARRIAGE_RETURN) {
			if (readIndex < 3) {
				// not enough characters
				readIndex = 0;
			}
			else {				
				readBuffer[readIndex] = 0; // mark it as the end of the string
				changeSetting(readBuffer);	
				readIndex = 0;
			}
		}
		else if (readIndex >= READ_BUFFER_LEN) {
			readIndex = 0; // too many characters. Reset and try again
		}
		else
			readIndex++;
}

void showSettingsMenu() {
	
	
	Serial.println(F("\r\n====== EtherPower Settings Menu ========"));
	Serial.println(F("=       Current values are shown         ="));
	Serial.println(F("=       Send new values like B=150       ="));
	Serial.println(F("=       with a carriage return           ="));
	Serial.println(F("=========================================="));
	
	Serial.print(F("\r\nI="));
	//Serial.print(user_settings.server_ip);
	Serial.println(F(" (Server IP address)"));
	
	Serial.print(F("N="));	
	//Serial.print(user_settings.server_netmask);
	Serial.println(F(" (Server netmask)"));

  Serial.print(F("P="));
  //Serial.print(user_settings.server_port);
  Serial.println(F(" (Server port. Port 80 is default for HTTP"));

  Serial.print(F("M="));
  //Serial.print(user_settings.server_mac);
  Serial.println(F(" (Ethernet MAC address)"));	
	
	Serial.println(F("\r\n(Send...)"));
	Serial.println(F("  ? to show current settings"));
	Serial.println(F("  R to reset everything to defaults"));
  Serial.println(F("  ! to reboot\r\n"));
	
}

void resetSettings() {
	user_settings.settings_version = SETTINGS_VERSION;
	
	settingsEepromWrite();
	
}

void changeSetting(char *line) {
  
	
	char setting_val[6];
  char param;
  uint16_t newValue;
	
	if (readBuffer[1] != '='){  // check if the equals sign is there
		Serial.print(F("Missing '=' in command"));
		readIndex = 0;
		return;
  }
	
	// move the value characters into a char array while verifying they are digits
  for(int i=0; i<5; i++) {
	if (i+2 < readIndex) {
		if (isDigit(readBuffer[i+2]))
			setting_val[i] = readBuffer[i+2];
		else {
			Serial.println(F("Invalid setting value"));
			return;
			
		}			
	}
	else
		setting_val[i] = 0;
  }
	
	param = readBuffer[0];
  newValue = atoi(setting_val); // convert the val section to an integer
	
	switch (param) {		 
		
		lastInputTime = millis(); // reset screensaver count		
		
		case 'C': // LED Count
//        user_settings.led_count = constrain(newValue, MIN_LEDS, MAX_LEDS);
				settingsEepromWrite();
		break;	
		
		default:
			Serial.print(F("Command Error: "));
			Serial.println(readBuffer[0]);
			return;
		break;
	
  } 	
	showSettingsMenu(); 
  
}

void settingsEepromRead()
{
	uint8_t ver = EEPROM.read(0);
	uint8_t temp[sizeof(user_settings)];
	bool read_fail = false;

	if (ver != SETTINGS_VERSION) {
		Serial.println(F("Error: Reading EEPROM settings failed"));
		Serial.println(F("Loading defaults"));
		resetSettings();		
		return;
	}				
	
	for (int i=0; i<sizeof(user_settings); i++)
	{
		temp[i] = EEPROM.read(i);
	}	
	
	memcpy((char*)&user_settings, temp, sizeof(user_settings));
	
	// if any values are out of range, reset them all	
	
  //	if (user_settings.led_count < MIN_LEDS || user_settings.led_count > MAX_LEDS)
  //		read_fail = true;	
	
	if (read_fail) {
		resetSettings();
		
	}
	
}

void settingsEepromWrite() {
	uint8_t temp[sizeof(user_settings)];	
	memcpy(temp, (char*)&user_settings, sizeof(user_settings));
	
	for (int i=0; i<sizeof(user_settings); i++)
	{
		EEPROM.write(i, temp[i]);
	}	
}

void printError(int reason) {
	switch(reason) {
		case ERR_SETTING_NUM:
			Serial.print(F("Error: Invalid setting number"));
		break;
		case ERR_SETTING_RANGE:
			Serial.print(F("Error: Setting out of range"));
		break;
		default:
			Serial.print(F("Error:"));Serial.println(reason);
		break;
	}
}
