#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI 13
#define OLED_CLK 14
#define OLED_DC 4
#define OLED_CS 27
#define OLED_RESET 15

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LOGO_HEIGHT 14
#define LOGO_WIDTH 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

static const unsigned char PROGMEM folder_bmp[] = { 0b11111000, 0b00000000,
                                                    0b10000111, 0b11111111,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b11111111, 0b11111111,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b10000000, 0b00000001,
                                                    0b11111111, 0b11111111 };

void oled_init() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
}

void printTruncatedDirectoryOLED(char** listOfFileToBeDisplayed, int selectedIndex, char* fileType, int numberOfFilesInDirectory) {
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(SSD1306_WHITE);
  for (int i = 0; i < 3; i++) {
    if (i < numberOfFilesInDirectory) {
      if (i == selectedIndex) {
        display.fillRect(0, selectedIndex * 20, display.width(), 20, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }

      display.display();
      display.setCursor(0, i * 20);
      Serial.println(listOfFileToBeDisplayed[i]);
      display.println();
      display.println(listOfFileToBeDisplayed[i]);

      if (fileType[i] == 1) {
        if (i == selectedIndex) {
          display.drawBitmap(5 * strlen(listOfFileToBeDisplayed[i]) + 20, 5 + 20 * i, folder_bmp, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_BLACK, 1);
        } else {
          display.drawBitmap(5 * strlen(listOfFileToBeDisplayed[i]) + 20, 5 + 20 * i, folder_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }
      }
    }
    if(numberOfFilesInDirectory == 0)
    {
      display.setCursor(10, 20);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.println("Directory   empty");
    }
  }
  display.display();
}