#ifndef OLED_H
#define OLED_H

#include "oled.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
void oled_init();
void printTruncatedDirectoryOLED(char** listOfFileToBeDisplayed, int selectedIndex, char* fileType, int numberOfFilesInDirectory);

#endif  //OLED_H