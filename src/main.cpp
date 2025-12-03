/*
  Author: Dustin Watts
  Date: 27-08-2020

  My thanks goes out to Brian Lough, Colin Hickey, and the people on my Discord
  server for helping me a lot with the code and troubleshooting!
  https://discord.gg/RE3XevS

  FreeTouchDeck is based on the FreeDeck idea by Koriwi. It uses the TFT_eSPI
  library by Bodmer for the display and touch functionality and it uses an
  ESP32-BLE-Keyboard fork with a few modifications. For saving and loading
  configuration it uses ArduinoJson V6.

  FreeTouchDeck uses some libraries from other sources. These must be installed
  for FreeTouchDeck to compile and run.

  These are those libraries:

      !----------------------------- Library Dependencies
  --------------------------- !
      - Adafruit-GFX-Library (tested with version 1.10.4), available through
  Library Manager
      - TFT_eSPI (tested with version 2.3.70), available through Library Manager
      - ESP32-BLE-Keyboard (latest version) download from:
  https://github.com/T-vK/ESP32-BLE-Keyboard
      - ESPAsyncWebserver (latest version) download from:
  https://github.com/me-no-dev/ESPAsyncWebServer
      - AsyncTCP (latest version) download from:
  https://github.com/me-no-dev/AsyncTCP
      - ArduinoJson (tested with version 6.17.3), available through Library
  Manager

      --- If you use Capacitive touch (ESP32 TouchDown) ---
      - Dustin Watts FT6236 Library (version 1.0.2),
  https://github.com/DustinWatts/FT6236

  The FILESYSTEM (SPI FLASH filing system) is used to hold touch screen
  calibration data. It has to be runs at least once when using resistive touch.
  After that you can set REPEAT_CAL to false (default).

  !-- Make sure you have setup your TFT display and ESP setup correctly in
  TFT_eSPI/user_setup.h --!

        Select the right screen driver and the board (ESP32 is the only one
  tested) you are using. Also make sure TOUCH_CS is defined correctly. TFT_BL is
  also be needed!

        You can find examples of User_Setup.h in the "user_setup.h Examples"
  folder.

*/

// ------- Uncomment the next line if you use capacitive touch -------
// (The ESP32 TOUCHDOWN and the ESP32 TouchDown S3 uses this!)
// #define USECAPTOUCH

// ------- If your board is capapble of USB HID you can uncomment this -

// #define USEUSBHID

// ------- Uncomment and populate the following if your cap touch uses custom
// i2c pins -------
// #define CUSTOM_TOUCH_SDA 17
// #define CUSTOM_TOUCH_SCL 18

// ------- Uncomment the define below if you want to use SLEEP and wake up on
// touch ------- The pin where the IRQ from the touch screen is connected uses
// ESP-style GPIO_NUM_* instead of just pinnumber
#define touchInterruptPin GPIO_NUM_14

// ------- Uncomment the define below if you want to use a piezo buzzer and
// specify the pin where the speaker is connected -------
// #define speakerPin 26

// ------- NimBLE definition, use only if the NimBLE library is installed
// and if you are using the original ESP32-BLE-Keyboard library by T-VK -------
#define USE_NIMBLE

#define USE_AIR_MOUSE

// Define the filesystem to be used. For now just SPIFFS.
#define FILESYSTEM SPIFFS

#include <SPIFFS.h> // Filesystem support header
// #include <LittleFS.h>   // Filesystem support header

const char *versionnumber = "0.9.18a";

/* Version 0.9.18a.
 *
 * Adding ESP32-S3 support
 * Trying to add LitteFS Support
 * Fix #89
 * Fix #90
 */

#include <FS.h>       // Filesystem support header
#include <pgmspace.h> // PROGMEM support header

#include <Preferences.h> // Used to store states before sleep/reboot

#include <TFT_eSPI.h> // The TFT_eSPI library

#if defined(USEUSBHID)

#include "Keydefines.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard bleKeyboard;

#else

#include <BLECombo.h> // BleKeyboard is used to communicate over BLE
#include <keyvals.cpp>
BLECombo bleCombo;

// Checking for BLE Keyboard version
#ifndef BLE_KEYBOARD_VERSION
#warning Old BLE Keyboard version detected. Please update.
#define BLE_KEYBOARD_VERSION "Outdated"
#endif // !defined(BLE_KEYBOARD_VERSION)

#endif // if

#if defined(USE_NIMBLE)

#include "NimBLEBeacon.h" // Additional BLE functionaity using NimBLE
#include "NimBLEDevice.h" // Additional BLE functionaity using NimBLE
#include "NimBLEUtils.h"  // Additional BLE functionaity using NimBLE

#else

#include "BLEBeacon.h" // Additional BLE functionaity
#include "BLEDevice.h" // Additional BLE functionaity
#include "BLEUtils.h"  // Additional BLE functionaity

#endif // defined(USE_NIMBLE)

#include "esp_bt_device.h" // Additional BLE functionaity
#include "esp_bt_main.h"   // Additional BLE functionaity
#include "esp_sleep.h"     // Additional BLE functionaity

#include <ArduinoJson.h> // Using ArduinoJson to read and write config files

#include <WiFi.h> // Wifi support

#include <AsyncTCP.h>          //Async Webserver support header
#include <ESPAsyncWebServer.h> //Async Webserver support header

#include <ESPmDNS.h> // DNS functionality

#ifdef USECAPTOUCH
#include <FT6236.h>
#include <Wire.h>
FT6236 ts = FT6236();
#endif // defined(USECAPTOUCH)

AsyncWebServer webserver(80);

TFT_eSPI tft = TFT_eSPI();

Preferences savedStates;

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The FILESYSTEM file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Set the width and height of your screen here:
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Keypad start position, centre of the first button
#define KEY_X SCREEN_WIDTH / 6
#define KEY_Y SCREEN_HEIGHT / 4

// Gaps between buttons
#define KEY_SPACING_X SCREEN_WIDTH / 24
#define KEY_SPACING_Y SCREEN_HEIGHT / 16

// Width and height of a button
#define KEY_W (SCREEN_WIDTH / 3) - KEY_SPACING_X
#define KEY_H (SCREEN_WIDTH / 3) - KEY_SPACING_Y

// Font size multiplier
#define KEY_TEXTSIZE 1

// Text Button Label Font
#define LABEL_FONT &FreeSansBold12pt7b

#ifdef USE_AIR_MOUSE
#include <Wire.h>
#define I2C_GYRO_SCL 21
#define I2C_GYRO_SDA 18
#define MOUSE_SENSITIVITY 200
#include "AirMouse.h"
#endif

// placeholder for the pagenumber we are on (0 indicates home)
int pageNum = 0;

// Initial LED brightness
int ledBrightness = 255;

// Every button has a row associated with it
uint8_t rowArray[6] = {0, 0, 0, 1, 1, 1};
// Every button has a column associated with it
uint8_t colArray[6] = {0, 1, 2, 0, 1, 2};

// path to the directory the logo are in ! including leading AND trailing / !
char logopath[64] = "/logos/";

char emptStr[] = "";

// templogopath is used to hold the complete path of an image. It is empty for
// now.
char templogopath[64] = "";

// Struct to hold the logos per screen
struct Icons {
  char icons[6][32];  // 6 logos per screen, max 32 chars per path
};

// Struct for individual action with action, value and symbol
struct Action {
  uint8_t action;
  uint8_t value;
  char    symbol[64];
};

// Struct Actions: 3 actions per button
struct Actions {
  struct Action actions[3];
};

// Each button has an action struct in it
struct Button {
  struct Actions actions;
  bool           latch;
  char           latchlogo[32];
};

// Each menu has 6 buttons
struct Menu {
  struct Button buttons[6];  // 6 buttons per menu
};

// Struct to hold the general logos.
struct SystemIcons {
  char settings[64];
  char homebutton[64];
  char configurator[64];
};

// Struct to hold the general config like colours.
struct Config {
  uint16_t menuButtonColour;
  uint16_t functionButtonColour;
  uint16_t backgroundColour;
  uint16_t latchedColour;
  bool     sleepenable;
  uint16_t sleeptimer;
  bool     beep;
  uint8_t  modifier1;
  uint8_t  modifier2;
  uint8_t  modifier3;
  uint16_t helperdelay;
};

struct Wificonfig {
  char     ssid[64];
  char     password[64];
  char     wifimode[9];
  char     hostname[64];
  uint8_t  attempts;
  uint16_t attemptdelay;
};

// Array to hold all the latching statuses
bool islatched[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Create instances of the structs
Wificonfig wificonfig;

Config generalconfig;

SystemIcons systemIcons;

Icons screen0;
Icons screen1;
Icons screen2;
Icons screen3;
Icons screen4;
Icons screen5;
Icons screen6;

Menu menus[6];

unsigned long previousMillis = 0;
unsigned long Interval = 0;
bool          displayinginfo;
const char* jsonfilefail = "";

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//--------- Function declarations ------------
void playBeepTone(int frequency, int duration);
void processButtonActions(struct Button* button, int latchIndex);
bool loadConfigWithErrorHandling(const char* configName);
void checkConfigFileExists(const char* filename);
bool handleMenuSwitchCommand(const char* command);
void getButtonCoordinates(int buttonIndex, int& col, int& row);
bool readSerialValue(char* buffer, size_t bufferSize);
bool handleWifiConfigCommand(const char* command, const char* configType);
void navigateToPage(int newPageNum, bool enableMouse = false);

// Button handler function declarations
void handleHomePageButton(int buttonIndex);
void handleMenuPageButton(int buttonIndex);
void handleSettingsPageButton(int buttonIndex);
void handleButtonPress(int buttonIndex);

//--------- Internal references ------------
// (this needs to be below all structs etc..)
#include "ScreenHelper.h"
#include "ConfigLoad.h"
#include "DrawHelper.h"
#include "ConfigHelper.h"
#include "Touch.h"
#include "UserActions.h"
#include "Action.h"
#include "Webserver.h"

//-------------------------------- SETUP
//--------------------------------------------------------------

void setup() {

  // Use serial port
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");

  Serial.println("[INFO]: Loading saved brightness state");
  savedStates.begin("ftd", false);

  ledBrightness = savedStates.getInt("ledBrightness", 255);

  Serial.println("[INFO]: Reading latch stated back from memory:");
  savedStates.getBytes("latched", islatched, sizeof(islatched));

  for (int i = 0; i < sizeof(islatched); i++) {

    Serial.print(islatched[i]);
  }
  Serial.println("");

#ifdef USECAPTOUCH
#ifdef CUSTOM_TOUCH_SDA
  if (!ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL))
#else
  if (!ts.begin(40))
#endif // defined(CUSTOM_TOUCH_SDA)
  {
    Serial.println("[WARNING]: Unable to start the capacitive touchscreen.");
  } else {
    Serial.println("[INFO]: Capacitive touch started!");
  }
#endif // defined(USECAPTOUCH)

  // Setup PWM channel and attach pin bl_pin
  ledcSetup(0, 5000, 8);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 0);
#else
  ledcAttachPin(backlightPin, 0);
#endif                         // defined(TFT_BL)
  ledcWrite(0, ledBrightness); // Start @ initial Brightness

  // --------------- Init Display -------------------------

  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // -------------- Start filesystem ----------------------

  if (!FILESYSTEM.begin()) {
    Serial.println("[ERROR]: FILESYSTEM initialisation failed!");
    drawErrorMessage(
        "Failed to init FILESYSTEM! Did you upload the data folder?");
    while (1)
      yield(); // We stop here
  }
  Serial.println("[INFO]: FILESYSTEM initialised.");

  // Check for free space

  Serial.print("[INFO]: Free Space: ");
  Serial.println(FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes());

  //------------------ Load Wifi Config
  //----------------------------------------------

  Serial.println("[INFO]: Loading Wifi Config");
  if (!loadMainConfig()) {
    Serial.println("[WARNING]: Failed to load WiFi Credentials!");
  } else {
    Serial.println("[INFO]: WiFi Credentials Loaded");
  }

  // ----------------- Load webserver ---------------------

  handlerSetup();

  // ------------------- Splash screen ------------------

  // If we are woken up we do not need the splash screen
  if (wakeup_reason > 0) {
    // But we do draw something to indicate we are waking up
    tft.setTextFont(2);
    tft.println(" Waking up...");
  } else {

    // Draw a splash screen
    drawBmp("/sys/ico/home.bmp", 0, 0);
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("Loading version %s\n", versionnumber);
    Serial.printf("[INFO]: Loading version %s\n", versionnumber);
  }

// Calibrate the touch screen and retrieve the scaling factors
#ifndef USECAPTOUCH
  Serial.println("[INFO]: Waiting for touch calibration...");
  touch_calibrate();
  Serial.println("[INFO]: Touch calibration completed!");
#endif // !defined(USECAPTOUCH)

  // Check if all required configuration files exist
  checkConfigFileExists("/config/general.json");
  checkConfigFileExists("/config/homescreen.json");
  checkConfigFileExists("/config/menu1.json");
  checkConfigFileExists("/config/menu2.json");
  checkConfigFileExists("/config/menu3.json");
  checkConfigFileExists("/config/menu4.json");
  checkConfigFileExists("/config/menu5.json");

  // After checking the config files exist, actually load them
  if (!loadConfig("general")) {
    Serial.println("[WARNING]: general.json seems to be corrupted!");
    Serial.println("[WARNING]: To reset to default type 'reset general'.");
    jsonfilefail = "general";
    pageNum = 10;
  }

  // Setup PWM channel for Piezo speaker

#ifdef speakerPin
  ledcSetup(2, 500, 8);

  // Play startup beep sequence
  playBeepTone(600, 150);
  playBeepTone(800, 150);
  playBeepTone(1200, 150);

#endif // defined(speakerPin)

  // Load all configuration files with error handling
  loadConfigWithErrorHandling("homescreen");
  loadConfigWithErrorHandling("menu1");
  loadConfigWithErrorHandling("menu2");
  loadConfigWithErrorHandling("menu3");
  loadConfigWithErrorHandling("menu4");
  loadConfigWithErrorHandling("menu5");
  Serial.println("[INFO]: All configs loaded");

  strcpy(systemIcons.settings, "/sys/ico/settings.bmp");
  strcpy(systemIcons.homebutton, "/sys/ico/home.bmp");
  strcpy(systemIcons.configurator, "/sys/ico/wifi.bmp");
  Serial.println("[INFO]: General logos loaded.");

  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);

  //------------------BLE Initialization
  //------------------------------------------------------------------------

#if defined(USEUSBHID)

  // initialize control over the keyboard:
  Keyboard.begin();
  USB.begin();

#else
  bleCombo.begin();
  Serial.println("[INFO]: Starting BLE");

#endif // if defined(USEUSBHID)

#ifdef USE_AIR_MOUSE

  Wire.begin(I2C_GYRO_SDA, I2C_GYRO_SCL, 100000);

  i2cData[0] = 7;
  i2cData[1] = 0x00;
  i2cData[3] = 0x00;

  while (i2cWrite(0x19, i2cData, 4, false))
    ;
  while (i2cWrite2(0x6B, 0x01, true))
    ;
  while (i2cRead(0x75, i2cData, 1))
    ;
  delay(100);
  while (i2cRead(0x3B, i2cData, 6))
    ;

#endif

    // ---------------- Printing version numbers
    // -----------------------------------------------

#if defined(USEUSBHID)
  Serial.println("[INFO]: Using USB Keyboard");
#else
  Serial.print("[INFO]: BLE Keyboard version: ");
  Serial.println(BLE_KEYBOARD_VERSION);
#endif // if defined(USEUSBHID)

  Serial.print("[INFO]: ArduinoJson version: ");
  Serial.println(ARDUINOJSON_VERSION);
  Serial.print("[INFO]: TFT_eSPI version: ");
  Serial.println(TFT_ESPI_VERSION);

  // ---------------- Start the first keypad -------------

  // Draw background
  tft.fillScreen(generalconfig.backgroundColour);

  // Draw keypad
  Serial.println("[INFO]: Drawing keypad");
  drawKeypad();

#ifdef touchInterruptPin
  if (generalconfig.sleepenable) {
    pinMode(touchInterruptPin, INPUT_PULLUP);
    Interval = generalconfig.sleeptimer * 60000;
    Serial.println("[INFO]: Sleep enabled.");
    Serial.print("[INFO]: Sleep timer = ");
    Serial.print(generalconfig.sleeptimer);
    Serial.println(" minutes");
    islatched[28] = 1;
  }
#endif // defined(touchInterruptPin)

  Serial.println("[INFO]: Boot completed and successful!");
}

//--------------------- LOOP
//---------------------------------------------------------------------
bool mouseEnabled = false;
void loop(void) {

  if (mouseEnabled) {
    while (i2cRead(0x3B, i2cData, 14))
      ;

    gyroX = ((i2cData[8] << 8) | i2cData[9]);
    gyroY = ((i2cData[10] << 8) | i2cData[11]);
    gyroZ = ((i2cData[12] << 8) | i2cData[13]);

    // gyroX = gyroX / MOUSE_SENSITIVITY;
    gyroY = gyroY / MOUSE_SENSITIVITY;
    gyroZ = gyroZ / MOUSE_SENSITIVITY;

    if (bleCombo.isConnected()) {
      // Serial.print(gyroX);
      // Serial.print("   ");
      // Serial.print(gyroZ);
      // Serial.print("\r\n");
      bleCombo.mouseMove(-gyroZ, gyroY);
    }
    delay(6);
  }

  // Check if there is data available on the serial input that needs to be
  // handled.

  if (Serial.available()) {

    char command[32];  // Buffer for command (adjust size as needed)
    size_t len = Serial.readBytesUntil(' ', command, sizeof(command) - 1);
    command[len] = '\0';  // Null terminate

    if (strcmp(command, "cal") == 0) {
      FILESYSTEM.remove(CALIBRATION_FILE);
      ESP.restart();
    } else if (handleWifiConfigCommand(command, "setssid") ||
               handleWifiConfigCommand(command, "setpassword") ||
               handleWifiConfigCommand(command, "setwifimode")) {
      // WiFi config commands handled by helper function
    } else if (strcmp(command, "restart") == 0) {
      Serial.println("[WARNING]: Restarting");
      ESP.restart();
    } else if (strcmp(command, "reset") == 0) {
      char file[32];
      if (readSerialValue(file, sizeof(file))) {
        Serial.printf("[INFO]: Resetting %s.json now\n", file);
        resetconfig(file);
      }
    } else {
      // Handle menu switch commands
      handleMenuSwitchCommand(command);
    }
  }

  if (pageNum == 7) {
    uint16_t t_x = 0, t_y = 0;
    boolean  pressed = false;

    // If pageNum = 7, we are in STA or AP mode.
    // We no check if the button is pressed, and if so restart.
#ifdef USECAPTOUCH
    if (ts.touched()) {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      // Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif // defined(USECAPTOUCH)

    if (pressed) {
      // If pressed check if the touch falls within the restart button
      // drawSingleButton(140, 180, 200, 80, generalconfig.menuButtonColour,
      // TFT_WHITE, "Restart");
      if (t_x > 140 && t_x < 340) {
        if (t_y > 180 && t_y < 260) {
          // Touch falls within the boundaries of our button so we restart
          Serial.println("[WARNING]: Restarting");
          ESP.restart();
        }
      }
    }

  } else if (pageNum == 8) {

    if (!displayinginfo) {
      printinfo();
    }

    uint16_t t_x = 0, t_y = 0;

    // At the beginning of a new loop, make sure we do not use last loop's
    // touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched()) {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      // Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif // defined(USECAPTOUCH)

    if (pressed) {
      displayinginfo = false;
      pageNum = 6;
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  } else if (pageNum == 9) {

    // We were unable to connect to WiFi. Waiting for touch to get back to the
    // settings menu.
    uint16_t t_x = 0, t_y = 0;

    // At the beginning of a new loop, make sure we do not use last loop's
    // touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched()) {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      // Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif // defined(USECAPTOUCH)

    if (pressed) {
      // Return to Settings page
      displayinginfo = false;
      pageNum = 6;
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  } else if (pageNum == 10) {

    // A JSON file failed to load. We are drawing an error message. And waiting
    // for a touch.
    uint16_t t_x = 0, t_y = 0;

    // At the beginning of a new loop, make sure we do not use last loop's
    // touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched()) {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      // Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif // defined(USECAPTOUCH)

    if (pressed) {
      // Load home screen
      displayinginfo = false;
      pageNum = 0;
      tft.fillScreen(generalconfig.backgroundColour);
      drawKeypad();
    }
  } else {

    // Check if sleep is enabled and if our timer has ended.

#ifdef touchInterruptPin
    if (generalconfig.sleepenable) {
      if (millis() > previousMillis + Interval) {

        // The timer has ended and we are going to sleep  .
        tft.fillScreen(TFT_BLACK);
        Serial.println("[INFO]: Going to sleep.");
        // Play sleep beep sequence (descending tones)
        playBeepTone(1200, 150);
        playBeepTone(800, 150);
        playBeepTone(600, 150);
        Serial.println("[INFO]: Saving latched states");

        //        You could uncomment this to see the latch stated before going
        //        to sleep for(int i = 0; i < sizeof(islatched); i++){
        //
        //        Serial.print(islatched[i]);
        //
        //        }
        //        Serial.println("");

        savedStates.putBytes("latched", &islatched, sizeof(islatched));
        esp_sleep_enable_ext0_wakeup(touchInterruptPin, 0);
        esp_deep_sleep_start();
      }
    }
#endif // defined(touchInterruptPin)

    // Touch coordinates are stored here
    uint16_t t_x = 0, t_y = 0;

    // At the beginning of a new loop, make sure we do not use last loop's
    // touch.
    boolean pressed = false;

#ifdef USECAPTOUCH
    if (ts.touched()) {

      // Retrieve a point
      TS_Point p = ts.getPoint();

      // Flip things around so it matches our screen rotation
      p.x = map(p.x, 0, 320, 320, 0);
      t_y = p.x;
      t_x = p.y;

      pressed = true;
    }

#else

    pressed = tft.getTouch(&t_x, &t_y);

#endif // defined(USECAPTOUCH)

    // Check if the X and Y coordinates of the touch are within one of our
    // buttons
    for (uint8_t b = 0; b < 6; b++) {
      if (pressed && key[b].contains(t_x, t_y)) {
        key[b].press(true); // tell the button it is pressed

        // After receiving a valid touch reset the sleep timer
        previousMillis = millis();
      } else {
        key[b].press(false); // tell the button it is NOT pressed
      }
    }

    // Check if any key has changed state
    for (uint8_t b = 0; b < 6; b++) {
      if (key[b].justReleased()) {

        // Draw normal button space (non inverted)

        int col, row;
        getButtonCoordinates(b, col, row);

        int index;

        if (pageNum == 2) {
          index = b + 5;
        } else if (pageNum == 3) {
          index = b + 10;
        } else if (pageNum == 4) {
          index = b + 15;
        } else if (pageNum == 5) {
          index = b + 20;
        } else if (pageNum == 6) {
          index = b + 25;
        } else {
          index = b;
        }

        uint16_t buttonBG;
        bool     drawTransparent;

        uint16_t imageBGColor;
        if (islatched[index] && b < 5) {
          imageBGColor = getLatchImageBG(b);
        } else {
          imageBGColor = getImageBG(b);
        }

        if (imageBGColor > 0) {
          buttonBG = imageBGColor;
          drawTransparent = false;
        } else {
          if (pageNum == 0) {
            buttonBG = generalconfig.menuButtonColour;
            drawTransparent = true;
          } else {
            if (pageNum == 6 && b == 5) {
              buttonBG = generalconfig.menuButtonColour;
              drawTransparent = true;
            } else {
              buttonBG = generalconfig.functionButtonColour;
              drawTransparent = true;
            }
          }
        }
        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(
            &tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            KEY_Y + row * (KEY_H +
                           KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            KEY_W, KEY_H, TFT_WHITE, buttonBG, TFT_WHITE, emptStr,
            KEY_TEXTSIZE);
        key[b].drawButton();

        // After drawing the button outline we call this to draw a logo.
        if (islatched[index] && b < 5) {
          drawlogo(b, col, row, drawTransparent, true);
        } else {
          drawlogo(b, col, row, drawTransparent, false);
        }
      }

      if (key[b].justPressed()) {

        // Beep
        // Play button press beep
        playBeepTone(600, 50);

        int col, row;
        getButtonCoordinates(b, col, row);
        

        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(
            &tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            KEY_Y + row * (KEY_H +
                           KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            KEY_W, KEY_H, TFT_WHITE, TFT_WHITE, TFT_WHITE, emptStr,
            KEY_TEXTSIZE);
        key[b].drawButton();

        //---Button press handeling
        //--------------------------------------------------

        handleButtonPress(b);

        delay(10); // UI debouncing
      }
    }
  }
}

/**
 * @brief Play a beep tone on the speaker
 * @param frequency The frequency of the tone in Hz
 * @param duration The duration of the tone in milliseconds
 */
void playBeepTone(int frequency, int duration) {
#ifdef speakerPin
  if (generalconfig.beep) {
    ledcAttachPin(speakerPin, 2);
    ledcWriteTone(2, frequency);
    delay(duration);
    ledcDetachPin(speakerPin);
    ledcWrite(2, 0);
  }
#endif
}

/**
 * @brief Process button actions (3 sequential actions) and handle latch state
 * @param button Pointer to the button structure containing the actions
 * @param latchIndex Index in the islatched array for this button
 */
void processButtonActions(struct Button* button, int latchIndex) {
  // Execute the three button actions sequentially
  bleKeyboardAction(button->actions.actions[0].action,
                    button->actions.actions[0].value,
                    button->actions.actions[0].symbol);
  bleKeyboardAction(button->actions.actions[1].action,
                    button->actions.actions[1].value,
                    button->actions.actions[1].symbol);
  bleKeyboardAction(button->actions.actions[2].action,
                    button->actions.actions[2].value,
                    button->actions.actions[2].symbol);
  bleCombo.keyReleaseAll();
  
  // Handle latch state if this button is configured as a latch
  if (button->latch) {
    if (islatched[latchIndex]) {
      islatched[latchIndex] = 0;
    } else {
      islatched[latchIndex] = 1;
    }
  }
}

/**
 * @brief Load a configuration file with standardized error handling
 * @param configName Name of the configuration (without .json extension)
 * @return true if successful, false if failed
 */
bool loadConfigWithErrorHandling(const char* configName) {
  if (!loadConfig(configName)) {
    Serial.printf("[WARNING]: %s.json seems to be corrupted!\n", configName);
    Serial.printf("[WARNING]: To reset to default type 'reset %s'.\n", configName);
    jsonfilefail = configName;
    pageNum = 10;
    return false;
  }
  return true;
}

/**
 * @brief Check if a configuration file exists and halt execution if not found
 * @param filename Full path to the configuration file (with extension)
 */
void checkConfigFileExists(const char* filename) {
  if (!checkfile(filename)) {
    Serial.printf("[ERROR]: %s not found!\n", filename);
    while (1)
      yield(); // Stop!
  }
}

/**
 * @brief Handle serial menu switch commands with standardized behavior
 * @param command The command string to check
 * @return true if the command was handled, false otherwise
 */
bool handleMenuSwitchCommand(const char* command) {
  // Check each menu command (menu1 through menu5)
  for (int menuNumber = 1; menuNumber <= 5; menuNumber++) {
    char expectedCommand[10];
    sprintf(expectedCommand, "menu%d", menuNumber);
    if (strcmp(command, expectedCommand) == 0 && pageNum != menuNumber && pageNum != 7) {
      pageNum = menuNumber;
      drawKeypad();
      Serial.printf("Auto Switched to Menu %d\n", menuNumber);
      return true;
    }
  }
  return false;
}

/**
 * @brief Convert button index (0-5) to grid coordinates 
 * @param buttonIndex The button index (0-5)
 * @param col Reference to store column coordinate (0-2)
 * @param row Reference to store row coordinate (0-1)
 */
void getButtonCoordinates(int buttonIndex, int& col, int& row) {
  if (buttonIndex == 0) {
    col = 0; row = 0;
  } else if (buttonIndex == 1) {
    col = 1; row = 0;
  } else if (buttonIndex == 2) {
    col = 2; row = 0;
  } else if (buttonIndex == 3) {
    col = 0; row = 1;
  } else if (buttonIndex == 4) {
    col = 1; row = 1;
  } else if (buttonIndex == 5) {
    col = 2; row = 1;
  }
}

/**
 * @brief Navigate to a specific page and optionally enable mouse
 * @param newPageNum The page number to navigate to
 * @param enableMouse Whether to enable mouse functionality
 */
void navigateToPage(int newPageNum, bool enableMouse) {
  pageNum = newPageNum;
  if (enableMouse) {
    mouseEnabled = true;
  }
  drawKeypad();
}

/**
 * @brief Handle button press for home page (pageNum == 0)
 * @param buttonIndex The index of the pressed button (0-5)
 */
void handleHomePageButton(int buttonIndex) {
  int targetPage = buttonIndex + 1;
  bool enableMouse = (targetPage == 4); // Only enable mouse for page 4
  navigateToPage(targetPage, enableMouse);
}

/**
 * @brief Handle button press for menu pages (pageNum 1-5)
 * @param buttonIndex The index of the pressed button (0-5)
 */
void handleMenuPageButton(int buttonIndex) {
  if (buttonIndex == 5) { // Back home button
    if (pageNum == 4) {
      mouseEnabled = false;
    }
    pageNum = 0;
    drawKeypad();
    return;
  }
  
  if (buttonIndex >= 0 && buttonIndex <= 4) {
    // Get the appropriate menu and button based on pageNum and button index
    struct Button* button = nullptr;
    int latchIndex = (pageNum - 1) * 5 + buttonIndex;
    
    if (pageNum >= 1 && pageNum <= 5) {
      button = &menus[pageNum - 1].buttons[buttonIndex];
    }
    
    if (button != nullptr) {
      processButtonActions(button, latchIndex);
    }
  }
}

/**
 * @brief Handle button press for settings page (pageNum == 6)
 * @param buttonIndex The index of the pressed button (0-5)
 */
void handleSettingsPageButton(int buttonIndex) {
  switch (buttonIndex) {
    case 0:
      bleKeyboardAction(11, 1, 0);
      break;
    case 1:
      bleKeyboardAction(11, 2, 0);
      break;
    case 2:
      bleKeyboardAction(11, 3, 0);
      break;
    case 3:
      bleKeyboardAction(11, 4, 0);
      if (islatched[28]) {
        islatched[28] = 0;
      } else {
        islatched[28] = 1;
      }
      break;
    case 4:
      pageNum = 8;
      drawKeypad();
      break;
    case 5:
      pageNum = 0;
      drawKeypad();
      break;
  }
}

/**
 * @brief Main button handler that dispatches to appropriate page handler
 * @param buttonIndex The index of the pressed button (0-5)
 */
void handleButtonPress(int buttonIndex) {
  if (pageNum == 0) {
    handleHomePageButton(buttonIndex);
  } else if (pageNum >= 1 && pageNum <= 5) {
    handleMenuPageButton(buttonIndex);
  } else if (pageNum == 6) {
    handleSettingsPageButton(buttonIndex);
  }
}

/**
 * @brief Read a value from serial input with proper null termination
 * @param buffer Buffer to store the read value
 * @param bufferSize Size of the buffer
 * @return true if value was successfully read, false otherwise
 */
bool readSerialValue(char* buffer, size_t bufferSize) {
  size_t valueLen = Serial.readBytes(buffer, bufferSize - 1);
  buffer[valueLen] = '\0';
  return valueLen > 0;
}

/**
 * @brief Handle WiFi configuration commands with unified logic
 * @param command The command to check
 * @param configType The type of config command to handle ("setssid", "setpassword", "setwifimode")
 * @return true if the command was handled, false otherwise
 */
bool handleWifiConfigCommand(const char* command, const char* configType) {
  if (strcmp(command, configType) != 0) {
    return false;
  }

  char value[64];
  if (!readSerialValue(value, sizeof(value))) {
    return true; // Command was for us, even if reading failed
  }

  bool success = false;
  const char* configName = "";

  if (strcmp(configType, "setssid") == 0) {
    success = saveWifiSSID(value);
    configName = "SSID";
  } else if (strcmp(configType, "setpassword") == 0) {
    success = saveWifiPW(value);
    configName = "Password";
  } else if (strcmp(configType, "setwifimode") == 0) {
    success = saveWifiMode(value);
    configName = "WiFi Mode";
  }

  if (success) {
    Serial.printf("[INFO]: Saved new %s: %s\n", configName, value);
    loadMainConfig();
    Serial.println("[INFO]: New configuration loaded");
  }

  return true;
}
