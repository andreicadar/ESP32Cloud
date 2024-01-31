#include <Wire.h>
#include <Arduino.h>
#include "eeprom.h"
//0x8000 maximum address, 256Kb -> 32 KB -> 32768 -> 0x8000

#define eepromAdress 0x50

void eepromSetup(int sda, int scl){
  Wire.begin(sda, scl); //SDA- > GPIO 6; SCK -> GPIO 7

  //unsigned int address = 0;

  //writeStringToEEPROM("Cool WiFi.com", eepromAdress, 0x1337);
  //Serial.println(readStringFromEEPROM(eepromAdress, 0x1337));

  //writeStringToEEPROM("C0d3N1nj@", eepromAdress, 0xbeef);
  //Serial.println(readStringFromEEPROM(eepromAdress, 0xbeef));
}

char* EEPROM_read_WI_FI_SSID()
{
	return readStringFromEEPROM(eepromAdress, 0x1337);
}

char* EEPROM_read_WI_FI_Password()
{
	return readStringFromEEPROM(eepromAdress, 0xbeef);
}

void writeStringToEEPROM(char* buffer, int deviceaddress, unsigned int address) {
  for (int i = 0; i < strlen(buffer); i++) {
    writeEEPROM(deviceaddress, (address + i), buffer[i]);
  }
  writeEEPROM(deviceaddress, (address + strlen(buffer)), '\0');
}

char* readStringFromEEPROM(int deviceaddress, unsigned int address) {
  int bufferSize = 256;
  char* buffer = (char*)malloc(bufferSize * sizeof(char));

  char readChar = 'a';
  int i = 0;

  for (i = 0; i < bufferSize & readChar != '\0'; i++) {
    readChar = readEEPROM(deviceaddress, (address + i));
    buffer[i] = readChar;
  }
  if (i == bufferSize) {
    Serial.println("Aborting reading string, no null terminated character found!");
    return NULL;
  }
  buffer[i + 1] = '\0';
  return buffer;
}

void writeEEPROM(int deviceaddress, unsigned int address, byte data) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(address >> 8));
  Wire.write((int)(address & 0xFF));
  Wire.write(data);
  Wire.endTransmission();
  delay(2);
}

byte readEEPROM(int deviceaddress, unsigned int address) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(address >> 8));
  Wire.write((int)(address & 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  delay(2);
  if (Wire.available())
    rdata = Wire.read();
  return rdata;
}