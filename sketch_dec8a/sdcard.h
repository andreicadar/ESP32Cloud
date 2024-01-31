 #ifndef SDCARD_H
 #define SDCARD_H
 
 #include <SD.h>

 void SDCardInit();
 void printDirectoryRecursive(File dir, int numTabs, String currentDir);
 char *printDirectoryContents(File dir, String currentDir);
 #endif //EEPROM_H