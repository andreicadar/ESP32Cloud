#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include <SPI.h>

#include "eeprom.h"
#include "sdcard.h"
#include "oled.h"

#define JOYSTICK_X 35
#define JOYSTICK_Y 34
#define JOYSTICK_BTN 32

int joystickBtnValue = 0;
int oldJoystickBtnValue = 0;
int joystickXValue = 0;
int oldJoystickXValue = 0;
int joystickYValue = 0;
int oldJoystickYValue = 0;

const String SETUP_INIT = "SETUP: Initializing ESP32 dev board";
const String SETUP_ERROR = "!!ERROR!! SETUP: Unable to start SoftAP mode";
const String SETUP_SERVER_START = "SETUP: HTTP server started --> IP addr: ";
const String SETUP_SERVER_PORT = " on port: ";
const String INFO_NEW_CLIENT = "New client connected";
const String INFO_DISCONNECT_CLIENT = "Client disconnected";

const String HTTP_DOCTYPE = "<!DOCTYPE html><html>\r\n\r\n";
const String HTTP_HEAD = "<head><title> My cool webpage </title> <style>/* CSS styles */body {font-family: Arial, sans-serif;}.black-text {color: black;}.blue-underlined {color: blue;text-decoration: underline;}</style></head>\r\n";
const String HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";
const String HTML_WELCOME = "<h1 style = text-align:center;>Welcome to your cloud!!</h1>\r\n";
String HTML_WEBPAGE = "";
const String FOLDER_EMOJI = "&#128193";
const String IMAGE_EMOJI = "&#128444";
String header = "";
char* SSID = NULL;
char* PASS = NULL;

const int HTTP_PORT_NO = 80;

int ledState = 0;
WiFiServer HttpServer(HTTP_PORT_NO);

char fileType[15];
char webFileType[15];
int numberOfFilesInDirectory = 0;
int webNumberOfFilesInDirectory = 0;
int globalFileIndex = 0;
int selectedIndex = 0;
char** fileList;
char** webFileList;
int oldJoyStickXState = 1;    //0 up 1 - 2 down
int oldJoyStickYState = 1;    //0 left 1 - 2 right
int oldJoyStickBtnState = 0;  //0 unpressed 1 pressed
char absolutePathSoFar[100] = "/";
char webAbsolutePathSoFar[100] = "";

char** arrayOfFiles(char* directoryListString, char* fileType, int* numberOfFilesInDirectory);
void readJoystick();
void printWelcomePage(WiFiClient client);
void changeWebDirectory(char* newPath);
void buildWebpage();


void setup() {
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);

  pinMode(2, OUTPUT);

  Serial.begin(115200);
  eepromSetup(21, 22);
  SDCardInit();
  oled_init();

  File file = SD.open(absolutePathSoFar);
  delay(50);
  char* x = printDirectoryContents(file, absolutePathSoFar);

  fileList = arrayOfFiles(x, fileType, &numberOfFilesInDirectory);
  webFileList = arrayOfFiles(x, webFileType, &webNumberOfFilesInDirectory);

  printTruncatedDirectoryOLED(fileList + globalFileIndex, selectedIndex, fileType, numberOfFilesInDirectory);
  display.display();
  free(x);
  file.close();

  delay(10);

  SSID = EEPROM_read_WI_FI_SSID();
  PASS = EEPROM_read_WI_FI_Password();

  if (!WiFi.softAP(SSID, PASS)) {
    while (1)
      ;
  }


  const IPAddress accessPointIP = WiFi.softAPIP();
  const String webServerInfoMessage = SETUP_SERVER_START + accessPointIP.toString()
                                      + SETUP_SERVER_PORT + HTTP_PORT_NO;

  HttpServer.begin();
  delay(100);
}

void loop() {

  readJoystick();


  WiFiClient client = HttpServer.available();

  if (client) {                     // if you get a client,
    char currentLine[3000] = "\0";  // make a String to hold incoming data from the client
    char c = '\0';
    while (client.connected()) {  // loop while the client's connected
      if (client.available()) {   // if there's bytes to read from the client,
        c = client.read();        // read a byte, then
        header += c;

        if (c == '\n') {
          if (strlen(currentLine) == 0) {
            printWelcomePage(client);

            break;
          } else currentLine[0] = '\0';
        } else if (c != '\r') {                  // if you got anything else but a carriage return character,
          currentLine[strlen(currentLine)] = c;  // add it to the end of the currentLine
          currentLine[strlen(currentLine) + 1] = '\0';
        }

        String copyPath = currentLine;

        if (copyPath.startsWith("GET / ")) {
          changeWebDirectory("/");
          strcpy(webAbsolutePathSoFar, "");
          buildWebpage();
        }

        if (copyPath.endsWith("file!") && copyPath.startsWith("GET")) {
          currentLine[strlen(currentLine) - 5] = '\0';
          strcpy(webAbsolutePathSoFar, currentLine + 4);
          changeWebDirectory(currentLine + 4);
          buildWebpage();
        }
      }
    }
    client.stop();
    header = "";
  }
}

void readJoystick() {
  joystickXValue = analogRead(JOYSTICK_X);

  if (joystickXValue > 3800 && oldJoyStickXState != 0) {
    scroll(0);  //up
    oldJoyStickXState = 0;

  } else if (joystickXValue < 200 && oldJoyStickXState != 2) {
    scroll(1);  //down
    oldJoyStickXState = 2;

  } else if (joystickXValue > 1700 && joystickXValue < 2050) {
    oldJoyStickXState = 1;
  }

  joystickYValue = analogRead(JOYSTICK_Y);

  if (joystickYValue > 3800 && oldJoyStickYState != 0) {
    if (fileType[globalFileIndex + selectedIndex] == 1) {
      changeDirectory();  //forth (right)
      oldJoyStickYState = 0;
    }
  } else if (joystickYValue < 200 && oldJoyStickYState != 2) {
    if (strcmp(absolutePathSoFar, "/") != 0) {
      goBackOneDirectory();  //back (left)
    }
    oldJoyStickYState = 2;

  } else if (joystickYValue > 1700 && joystickYValue < 2050) {
    oldJoyStickYState = 1;
  }

  joystickBtnValue = digitalRead(JOYSTICK_BTN);
  if (joystickBtnValue == 0 && oldJoyStickBtnState != 1) {
    if (fileType[globalFileIndex + selectedIndex] == 1) {
      changeDirectory();
    }
    oldJoyStickBtnState = 1;
  } else if (joystickBtnValue != 0) {
    oldJoyStickBtnState = 0;
  }
}

void changeDirectory() {
  if (strcmp(absolutePathSoFar, "/") != 0) {
    strcat(absolutePathSoFar, "/");
  }
  strcat(absolutePathSoFar, fileList[globalFileIndex + selectedIndex]);
  File file = SD.open(absolutePathSoFar);
  delay(50);
  char* x = printDirectoryContents(file, absolutePathSoFar);
  display.clearDisplay();

  for (int i = 0; i < 15; i++) {
    fileType[i] = 0;
  }

  fileList = arrayOfFiles(x, fileType, &numberOfFilesInDirectory);
  globalFileIndex = 0;
  selectedIndex = 0;
  printTruncatedDirectoryOLED(fileList + globalFileIndex, selectedIndex, fileType, numberOfFilesInDirectory);
}

void goBackOneDirectory() {
  int i = strlen(absolutePathSoFar) - 1;
  while (absolutePathSoFar[i] != '/') {
    i--;
  }
  absolutePathSoFar[i] = '\0';
  if (i == 0) {
    strcpy(absolutePathSoFar, "/");
  }
  File file = SD.open(absolutePathSoFar);
  delay(50);
  char* x = printDirectoryContents(file, absolutePathSoFar);
  display.clearDisplay();

  for (int i = 0; i < 15; i++) {
    fileType[i] = 0;
  }
  fileList = arrayOfFiles(x, fileType, &numberOfFilesInDirectory);
  globalFileIndex = 0;
  selectedIndex = 0;
  printTruncatedDirectoryOLED(fileList + globalFileIndex, selectedIndex, fileType, numberOfFilesInDirectory);
}

void scroll(int direction)  // 0 up 1 down
{
  bool doNotChange = 0;
  if (selectedIndex == 0) {
    if (direction == 0) {
      if (globalFileIndex != 0) {
        globalFileIndex--;
      } else {
        doNotChange = 1;
      }
    } else {
      if (numberOfFilesInDirectory > 1) {
        selectedIndex = 1;
      } else {
        doNotChange = 1;
      }
    }
  } else if (selectedIndex == 1) {
    if (direction == 0) {
      selectedIndex = 0;
    } else {
      if (numberOfFilesInDirectory > 2) {
        selectedIndex = 2;
      } else {
        doNotChange = 1;
      }
    }
  } else if (selectedIndex == 2) {
    if (direction == 0) {
      selectedIndex = 1;
    } else if (globalFileIndex != (numberOfFilesInDirectory - 3)) {
      globalFileIndex++;
    } else {
      doNotChange = 1;
    }
  }
  if (!doNotChange) {
    printTruncatedDirectoryOLED(fileList + globalFileIndex, selectedIndex, fileType + globalFileIndex, numberOfFilesInDirectory);
  }
}

void changeWebDirectory(char* newPath) {
  File file = SD.open(newPath);
  delay(50);
  char* x = printDirectoryContents(file, newPath);

  for (int i = 0; i < 15; i++) {
    webFileType[i] = 0;
  }
  webFileList = arrayOfFiles(x, webFileType, &webNumberOfFilesInDirectory);
}

void buildWebpage() {
  HTML_WEBPAGE = "<body>\r\n";
  for (int i = 0; i < webNumberOfFilesInDirectory; i++) {
    if (webFileType[i] == 0) {
      HTML_WEBPAGE += "<p class=";
      HTML_WEBPAGE += "\"black-text\">";
      HTML_WEBPAGE += webFileList[i];
      if (strcmp(webFileList[i] + (strlen(webFileList[i]) - 4), ".bmp") == 0) {
        HTML_WEBPAGE += " ";
        HTML_WEBPAGE += IMAGE_EMOJI;
      }
      HTML_WEBPAGE += "</p>";
    } else {
      HTML_WEBPAGE += "<p> <a href=";
      int j = 0;
      while (webAbsolutePathSoFar[j] != '\0') {
        HTML_WEBPAGE += webAbsolutePathSoFar[j++];
      }
      HTML_WEBPAGE += "/";
      HTML_WEBPAGE += webFileList[i];
      HTML_WEBPAGE += "file!>";  //file is allowed in links but not in filenames :).
      HTML_WEBPAGE += webFileList[i];
      HTML_WEBPAGE += "</a> ";
      HTML_WEBPAGE += FOLDER_EMOJI;
      HTML_WEBPAGE += "</p>";
    }
  }
  if (webNumberOfFilesInDirectory == 0) {
    HTML_WEBPAGE += "<h1 style = text-align:center;>Directory empty</h1>\r\n";
  }
  HTML_WEBPAGE += "</body>";
  HTML_WEBPAGE += "</html>";
}

char** arrayOfFiles(char* directoryListString, char* fileType, int* numberOfFilesInDirectory) {
  int maxNumberOfFilesInFolder = 15;
  int fileMaxNameLength = 15;
  char** filesList = (char**)malloc(maxNumberOfFilesInFolder * sizeof(char*));
  for (int i = 0; i < maxNumberOfFilesInFolder; i++) filesList[i] = (char*)malloc(fileMaxNameLength * sizeof(char));

  for (int i = 0; i < 15; i++) {
    fileType[i] = 0;
  }

  int indexInFilesList = 0;
  int indexInFile = 0;

  for (int i = 0; i < strlen(directoryListString); i++) {
    indexInFile = 0;
    while (directoryListString[i] != '%' && directoryListString[i] != '#') {
      filesList[indexInFilesList][indexInFile++] = directoryListString[i++];
    }
    filesList[indexInFilesList][indexInFile] = '\0';
    if (directoryListString[i] == '#') {
      fileType[indexInFilesList] = 1;
    }
    indexInFilesList++;
  }
  *numberOfFilesInDirectory = indexInFilesList;
  return filesList;
}

void printWelcomePage(WiFiClient client) {
  client.println(HTTP_DOCTYPE);
  client.println(HTTP_HEAD);
  buildWebpage();
  client.print(HTML_WELCOME);
  client.println(HTML_WEBPAGE);
  client.println();
}