#include <SPI.h>
#include <SD.h>
#include "sdcard.h"

#define SDCardCSPin 5

void SDCardInit() {
  pinMode(SDCardCSPin, OUTPUT);
  digitalWrite(SDCardCSPin, HIGH);
  if (!SD.begin(SDCardCSPin)) {
    Serial.println("Card Mount Failed");
  }
}

char *printDirectoryContents(File dir, String currentDir) {
  char *contents = (char *)malloc(150 * sizeof(char));
  char index = 0;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (char i = 0; i < strlen(entry.name()); i++) {
      contents[index++] = entry.name()[i];
    }
    if (entry.isDirectory()) {
      contents[index++] = '#';
    } else {
      contents[index++] = '%';
    }
    entry.close();
  }
  contents[index] = '\0';
  return contents;
}

void printDirectoryRecursive(File dir, int numTabs, String currentDir) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      if (numTabs == 0)
        Serial.println("** Done **");
      return;
    }

    for (uint8_t i = 0; i < numTabs; i++)
      Serial.print('\t');

    Serial.print(entry.name());

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectoryRecursive(entry, numTabs + 1, currentDir + "/" + entry.name());
    } else {
      File file = SD.open(currentDir + "/" + entry.name(), FILE_READ);
      unsigned char *text = (unsigned char *)malloc(sizeof(char) * 100);
      text[file.read(text, 100)] = '\0';
      Serial.print("\t\t");
      Serial.print("Dimensiune fisier: ");
      Serial.println(entry.size(), DEC);
      Serial.print("\t\tContents: ");
      Serial.println((char *)text);
      file.close();
      free(text);
    }
    entry.close();
  }
}
