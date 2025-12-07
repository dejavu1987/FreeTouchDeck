#pragma once

#include <functional>

#ifdef USECAPTOUCH
#include <FT6236.h>
extern FT6236 ts;
#endif

extern TFT_eSPI tft;
extern TFT_eSPI_Button key[6];

// Touch handling structure for consistent coordinate and state management
struct TouchState {
  uint16_t x;
  uint16_t y;
  bool pressed;
  bool valid;
};

/**
 * @brief Initialize touch handling based on touch type (capacitive or resistive)
 * @return true if touch initialization was successful
 */
bool initializeTouchHandling() {
#ifdef USECAPTOUCH
#ifdef CUSTOM_TOUCH_SDA
  bool success = ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL);
#else
  bool success = ts.begin(40);
#endif // defined(CUSTOM_TOUCH_SDA)
  
  if (!success) {
    Serial.println("[WARNING]: Unable to start the capacitive touchscreen.");
    return false;
  } else {
    Serial.println("[INFO]: Capacitive touch started!");
    return true;
  }
#else
  // For resistive touch, initialization happens during calibration
  return true;
#endif // defined(USECAPTOUCH)
}

/**
 * @brief Get touch input from either capacitive or resistive touch screen
 * @return TouchState containing coordinates and press state
 */
TouchState getTouchInput() {
  TouchState touch = {0, 0, false, false};
  
#ifdef USECAPTOUCH
  if (ts.touched()) {
    // Retrieve a point from capacitive touch
    TS_Point p = ts.getPoint();
    
    // Flip coordinates to match screen rotation
    p.x = map(p.x, 0, 320, 320, 0);
    touch.y = p.x;
    touch.x = p.y;
    touch.pressed = true;
    touch.valid = true;
  }
#else
  // Get touch from resistive touch screen
  touch.pressed = tft.getTouch(&touch.x, &touch.y);
  touch.valid = true;
#endif // defined(USECAPTOUCH)
  
  return touch;
}

/**
 * @brief Check if touch coordinates are within a rectangular area
 * @param touch TouchState containing coordinates
 * @param x1 Left boundary
 * @param y1 Top boundary  
 * @param x2 Right boundary
 * @param y2 Bottom boundary
 * @return true if touch is within bounds
 */
bool isTouchInBounds(const TouchState& touch, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  return (touch.pressed && touch.valid && 
          touch.x >= x1 && touch.x <= x2 && 
          touch.y >= y1 && touch.y <= y2);
}

/**
 * @brief Handle touch input for simple button press detection
 * @param buttonX1 Left boundary of button
 * @param buttonY1 Top boundary of button
 * @param buttonX2 Right boundary of button
 * @param buttonY2 Bottom boundary of button
 * @return true if button was pressed
 */
bool handleSimpleButtonTouch(uint16_t buttonX1, uint16_t buttonY1, uint16_t buttonX2, uint16_t buttonY2) {
  TouchState touch = getTouchInput();
  return isTouchInBounds(touch, buttonX1, buttonY1, buttonX2, buttonY2);
}

/**
 * @brief Process touch input for button grid and update button states
 * @param resetSleepTimer Callback function to reset sleep timer when touch is detected
 * @return TouchState for further processing if needed
 */
TouchState processButtonGridTouch(std::function<void()> resetSleepTimer = nullptr) {
  TouchState touch = getTouchInput();
  
  // Check if the X and Y coordinates of the touch are within any button
  for (uint8_t b = 0; b < 6; b++) {
    if (touch.pressed && touch.valid && key[b].contains(touch.x, touch.y)) {
      key[b].press(true); // tell the button it is pressed
      
      // Reset sleep timer if callback provided
      if (resetSleepTimer) {
        resetSleepTimer();
      }
    } else {
      key[b].press(false); // tell the button it is NOT pressed
    }
  }
  
  return touch;
}

/**
 * @brief This function presents the user with 4 points to touch and saves
         that data to a calibration file.
*
* @param none
*
* @return none
*
* @note If USECAPTOUCH is defined we do not need to calibrate touch
*/

#if !defined(USECAPTOUCH)
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check if calibration file exists and size is correct
  if (FILESYSTEM.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      FILESYSTEM.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = FILESYSTEM.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    // calibration data valid
    tft.setTouch(calData);
  }
  else
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = FILESYSTEM.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
#endif //!defined(USECAPTOUCH)
