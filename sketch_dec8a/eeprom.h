 #ifndef EEPROM_H
 #define EEPROM_H
 
 void eepromSetup(int sda, int scl);
 void writeStringToEEPROM(char* buffer, int deviceaddress, unsigned int address);
 char* readStringFromEEPROM(int deviceaddress, unsigned int address);
 void writeEEPROM(int deviceaddress, unsigned int address, byte data);
 byte readEEPROM(int deviceaddress, unsigned int address);
 char* EEPROM_read_WI_FI_SSID();
 char* EEPROM_read_WI_FI_Password();
 
 #endif //EEPROM_H