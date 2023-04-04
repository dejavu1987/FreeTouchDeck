
#include <TFT_eSPI.h> // The TFT_eSPI library

#include <ArduinoJson.h> // Using ArduinoJson to read and write config files

TFT_eSPI tft = TFT_eSPI();

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

// templogopath is used to hold the complete path of an image. It is empty for
// now.
char templogopath[64] = "";

// Struct to hold the logos per screen
struct Logos {
  char logo0[32];
  char logo1[32];
  char logo2[32];
  char logo3[32];
  char logo4[32];
  char logo5[32];
};

// Struct Action: 3 actions and 3 values per button
struct Actions {
  uint8_t action0;
  uint8_t value0;
  char    symbol0[64];
  uint8_t action1;
  uint8_t value1;
  char    symbol1[64];
  uint8_t action2;
  uint8_t value2;
  char    symbol2[64];
};

// Each button has an action struct in it
struct Button {
  struct Actions actions;
  bool           latch;
  char           latchlogo[32];
};

// Each menu has 6 buttons
struct Menu {
  struct Button button0;
  struct Button button1;
  struct Button button2;
  struct Button button3;
  struct Button button4;
  struct Button button5;
};

// Struct to hold the general logos.
struct Generallogos {
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

// Array to hold all the latching statuses
bool islatched[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

Config generalconfig;

Generallogos generallogo;

Logos screen0;
Logos screen1;
Logos screen2;
Logos screen3;
Logos screen4;
Logos screen5;
Logos screen6;

Menu menu1;
Menu menu2;
Menu menu3;
Menu menu4;
Menu menu5;
Menu menu6;

unsigned long previousMillis = 0;
unsigned long Interval = 0;
bool          displayinginfo;
char         *jsonfilefail = "";

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//--------- Internal references ------------
// (this needs to be below all structs etc..)
#include "ConfigHelper.h"
#include "ConfigLoad.h"
#include "DrawHelper.h"
#ifndef _SCREEN_HELPER_
#include "ScreenHelper.h"
#endif
#include "Touch.h"

//-------------------------------- SETUP
//--------------------------------------------------------------

void setup() {

  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Draw a splash screen
  drawBmp("/logos/freetouchdeck_logo.bmp", 0, 0);
  tft.setCursor(1, 3);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  strcpy(generallogo.homebutton, "/logos/home.bmp");
  strcpy(generallogo.configurator, "/logos/wifi.bmp");
  Serial.println("[INFO]: General logos loaded.");

  // Setup the Font used for plain text
  tft.setFreeFont(LABEL_FONT);

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

void loop(void) {

  if (pageNum == 8) {

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

        if (b == 0) {
          col = 0;
          row = 0;
        } else if (b == 1) {
          col = 1;
          row = 0;
        } else if (b == 2) {
          col = 2;
          row = 0;
        } else if (b == 3) {
          col = 0;
          row = 1;
        } else if (b == 4) {
          col = 1;
          row = 1;
        } else if (b == 5) {
          col = 2;
          row = 1;
        }

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
            KEY_W, KEY_H, TFT_WHITE, buttonBG, TFT_WHITE, "", KEY_TEXTSIZE);
        key[b].drawButton();

        // After drawing the button outline we call this to draw a logo.
        if (islatched[index] && b < 5) {
          drawlogo(b, col, row, drawTransparent, true);
        } else {
          drawlogo(b, col, row, drawTransparent, false);
        }
      }

      if (key[b].justPressed()) {

        int col, row;

        if (b == 0) {
          col = 0;
          row = 0;
        } else if (b == 1) {
          col = 1;
          row = 0;
        } else if (b == 2) {
          col = 2;
          row = 0;
        } else if (b == 3) {
          col = 0;
          row = 1;
        } else if (b == 4) {
          col = 1;
          row = 1;
        } else if (b == 5) {
          col = 2;
          row = 1;
        }

        tft.setFreeFont(LABEL_FONT);
        key[b].initButton(
            &tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
            KEY_Y + row * (KEY_H +
                           KEY_SPACING_Y), // x, y, w, h, outline, fill, text
            KEY_W, KEY_H, TFT_WHITE, TFT_WHITE, TFT_WHITE, "", KEY_TEXTSIZE);
        key[b].drawButton();

        //---------------------------------------- Button press handeling
        //--------------------------------------------------

        if (pageNum == 0) // Home menu
        {
          if (b == 0) // Button 0
          {
            pageNum = 1;
            drawKeypad();
          } else if (b == 1) // Button 1
          {
            pageNum = 2;
            drawKeypad();
          } else if (b == 2) // Button 2
          {
            pageNum = 3;
            drawKeypad();
          } else if (b == 3) // Button 3
          {
            pageNum = 4;
            drawKeypad();
          } else if (b == 4) // Button 4
          {
            pageNum = 5;
            drawKeypad();
          } else if (b == 5) // Button 5
          {
            pageNum = 6;
            drawKeypad();
          }
        }

        else if (pageNum == 1) // Menu 1
        {
          if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 2) // Menu 2
        {
          if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 3) // Menu 3
        {
          if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 4) // Menu 4
        {
          if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 5) // Menu 5
        {
          if (b == 5) // Button 5 / Back home
          {
            pageNum = 0;
            drawKeypad();
          }
        }

        else if (pageNum == 6) // Settings page
        {
          if (b == 4) // Button 4
          {
            pageNum = 8;
            drawKeypad();
          } else if (b == 5) {
            pageNum = 0;
            drawKeypad();
          }
        }

        delay(10); // UI debouncing
      }
    }
  }
}
