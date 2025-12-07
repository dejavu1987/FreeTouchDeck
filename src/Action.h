/**
* @brief This function takes an int as an "action" and "value". It uses
         a switch statement to determine which type of action to do.
         e.g. write, print, press. If an action requires a char, you
         can pass the pointer to that char through the parameter "symbol"
*
* @param action int
* @param value int
* @param symbol char *
*
* @return none
*
* @note Case 11 is used for special functions, none bleKeyboard related.
*/

void bleKeyboardAction(int action, int value, char *symbol) {

  Serial.println("[INFO]: BLE Keyboard action received");
  if (!bleCombo.isConnected() && action != 11) {
    Serial.println("[WARN]: Ble not connected");
    return;
  }
  switch (action) {
  case 0:
    // No Action
    break;
  case 1: // Delay
    delay(value);
    break;
  case 2: // Send TAB ARROW etc
    {
      static const uint8_t keyMap[] = {
        0,              // 0 - unused
        KEY_UP_ARROW,   // 1
        KEY_DOWN_ARROW, // 2
        KEY_LEFT_ARROW, // 3
        KEY_RIGHT_ARROW,// 4
        KEY_BACKSPACE,  // 5
        KEY_TAB,        // 6
        KEY_RETURN,     // 7
        KEY_PAGE_UP,    // 8
        KEY_PAGE_DOWN,  // 9
        KEY_DELETE,     // 10
        KEY_PRTSC,      // 11
        KEY_ESC,        // 12
        KEY_HOME,       // 13
        KEY_END         // 14
      };
      
      if (value > 0 && value < sizeof(keyMap)/sizeof(keyMap[0])) {
        bleCombo.write(keyMap[value]);
      }
    }
    break;
  case 3: // Send Media Key
#if defined(USEUSBHID)

#else
    {
      static const uint8_t mediaKeyMap[] = {
        0,                           // 0 - unused
        KEY_MEDIA_MUTE,              // 1
        KEY_MEDIA_VOLUME_DOWN,       // 2
        KEY_MEDIA_VOLUME_UP,         // 3
        KEY_MEDIA_PLAY_PAUSE,        // 4
        KEY_MEDIA_STOP,              // 5
        KEY_MEDIA_NEXT_TRACK,        // 6
        KEY_MEDIA_PREVIOUS_TRACK     // 7
      };
      
      if (value > 0 && value < sizeof(mediaKeyMap)/sizeof(mediaKeyMap[0])) {
        bleCombo.write(mediaKeyMap[value]);
      }
    }
    break;
#endif    // if defined(USEUSBHID)
  case 4: // Send Character
    bleCombo.print(symbol);
    break;
  case 5: // Option Keys
    {
      static const uint8_t optionKeyMap[] = {
        0,                // 0 - unused
        KEY_LEFT_CTRL,    // 1
        KEY_LEFT_SHIFT,   // 2
        KEY_LEFT_ALT,     // 3
        KEY_LEFT_GUI,     // 4
        KEY_RIGHT_CTRL,   // 5
        KEY_RIGHT_SHIFT,  // 6
        KEY_RIGHT_ALT,    // 7
        KEY_RIGHT_GUI     // 8
      };
      
      if (value == 9) {
        bleCombo.keyReleaseAll();
      } else if (value > 0 && value < sizeof(optionKeyMap)/sizeof(optionKeyMap[0])) {
        bleCombo.keyPress(optionKeyMap[value]);
      }
    }
    break;
  case 6: // Function Keys
    {
      static const uint8_t functionKeyMap[] = {
        0,        // 0 - unused
        KEY_F1,   // 1
        KEY_F2,   // 2
        KEY_F3,   // 3
        KEY_F4,   // 4
        KEY_F5,   // 5
        KEY_F6,   // 6
        KEY_F7,   // 7
        KEY_F8,   // 8
        KEY_F9,   // 9
        KEY_F10,  // 10
        KEY_F11,  // 11
        KEY_F12,  // 12
        KEY_F13,  // 13
        KEY_F14,  // 14
        KEY_F15,  // 15
        KEY_F16,  // 16
        KEY_F17,  // 17
        KEY_F18,  // 18
        KEY_F19,  // 19
        KEY_F20,  // 20
        KEY_F21,  // 21
        KEY_F22,  // 22
        KEY_F23,  // 23
        KEY_F24   // 24
      };
      
      if (value > 0 && value < sizeof(functionKeyMap)/sizeof(functionKeyMap[0])) {
        bleCombo.keyPress(functionKeyMap[value]);
      }
    }
    break;
  case 7: // Send Number
    bleCombo.print(value);
    break;
  case 8: // Send Special Character
    bleCombo.print(symbol);
    break;
  case 9: // Combos
    {
      static const uint8_t comboMap[][3] = {
        {0, 0, 0},                                        // 0 - unused
        {KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 0},              // 1
        {KEY_LEFT_ALT, KEY_LEFT_SHIFT, 0},               // 2  
        {KEY_LEFT_GUI, KEY_LEFT_SHIFT, 0},               // 3
        {KEY_LEFT_CTRL, KEY_LEFT_GUI, 0},                // 4
        {KEY_LEFT_ALT, KEY_LEFT_GUI, 0},                 // 5
        {KEY_LEFT_CTRL, KEY_LEFT_ALT, 0},                // 6
        {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI},     // 7
        {KEY_RIGHT_CTRL, KEY_RIGHT_SHIFT, 0},            // 8
        {KEY_RIGHT_ALT, KEY_RIGHT_SHIFT, 0},             // 9
        {KEY_RIGHT_GUI, KEY_RIGHT_SHIFT, 0},             // 10
        {KEY_RIGHT_CTRL, KEY_RIGHT_GUI, 0},              // 11
        {KEY_RIGHT_ALT, KEY_RIGHT_GUI, 0},               // 12
        {KEY_RIGHT_CTRL, KEY_RIGHT_ALT, 0},              // 13
        {KEY_RIGHT_CTRL, KEY_RIGHT_ALT, KEY_RIGHT_GUI}   // 14
      };
      
      if (value > 0 && value < sizeof(comboMap)/sizeof(comboMap[0])) {
        for (int i = 0; i < 3; i++) {
          if (comboMap[value][i] != 0) {
            bleCombo.keyPress(comboMap[value][i]);
          }
        }
      }
    }
    break;
  case 10: // Helpers
    {
      static const uint8_t helperFunctionKeys[] = {
        0,        // 0 - unused
        KEY_F1,   // 1
        KEY_F2,   // 2
        KEY_F3,   // 3
        KEY_F4,   // 4
        KEY_F5,   // 5
        KEY_F6,   // 6
        KEY_F7,   // 7
        KEY_F8,   // 8
        KEY_F9,   // 9
        KEY_F10,  // 10
        KEY_F11   // 11
      };
      
      if (value > 0 && value < sizeof(helperFunctionKeys)/sizeof(helperFunctionKeys[0])) {
        // Press configured modifiers
        if (generalconfig.modifier1 != 0) {
          bleCombo.keyPress(generalconfig.modifier1);
        }
        if (generalconfig.modifier2 != 0) {
          bleCombo.keyPress(generalconfig.modifier2);
        }
        if (generalconfig.modifier3 != 0) {
          bleCombo.keyPress(generalconfig.modifier3);
        }
        
        // Press the function key
        bleCombo.keyPress(helperFunctionKeys[value]);
        bleCombo.keyReleaseAll();
        delay(generalconfig.helperdelay);
      }
    }
    break;
  case 11: // Special functions
    switch (value) {
    case 1: // Enter config mode

      configmode();

      break;
    case 2: // Display Brightness Down
      if (ledBrightness > 25) {
        ledBrightness = ledBrightness - 25;
        ledcWrite(0, ledBrightness);
        savedStates.putInt("ledBrightness", ledBrightness);
      }
      break;
    case 3: // Display Brightness Up
      if (ledBrightness < 230) {
        ledBrightness = ledBrightness + 25;
        ledcWrite(0, ledBrightness);
        savedStates.putInt("ledBrightness", ledBrightness);
      }
      break;
    case 4: // Sleep Enabled
      if (generalconfig.sleepenable) {
        generalconfig.sleepenable = false;
        Serial.println("[INFO]: Sleep disabled.");
      } else {
        generalconfig.sleepenable = true;
        Interval = generalconfig.sleeptimer * 60000;
        Serial.println("[INFO]: Sleep enabled.");
        Serial.print("[INFO]: Timer set to: ");
        Serial.println(generalconfig.sleeptimer);
      }
      break;
    }
    break;
  case 12: // Numpad
    {
      static const uint8_t numpadKeyMap[] = {
        KEY_NUM_0,        // 0
        KEY_NUM_1,        // 1
        KEY_NUM_2,        // 2
        KEY_NUM_3,        // 3
        KEY_NUM_4,        // 4
        KEY_NUM_5,        // 5
        KEY_NUM_6,        // 6
        KEY_NUM_7,        // 7
        KEY_NUM_8,        // 8
        KEY_NUM_9,        // 9
        KEY_NUM_SLASH,    // 10
        KEY_NUM_ASTERISK, // 11
        KEY_NUM_MINUS,    // 12
        KEY_NUM_PLUS,     // 13
        KEY_NUM_ENTER,    // 14
        KEY_NUM_PERIOD    // 15
      };
      
      if (value >= 0 && value < sizeof(numpadKeyMap)/sizeof(numpadKeyMap[0])) {
        bleCombo.write(numpadKeyMap[value]);
      }
    }
    break;
  case 13: // Custom functions
    switch (value) {
    case 1:
      userAction1();
      break;
    case 2:
      userAction2();
      break;
    case 3:
      userAction3();
      break;
    case 4:
      userAction4();
      break;
    case 5:
      userAction5();
      break;
    case 6:
      userAction6();
      break;
    case 7:
      userAction7();
      break;
    }
    break;
  case 14: // Mouse
    switch (value) {
    case 1:
      bleCombo.mouseClick(MOUSE_LEFT);
      break;
    case 2:
      bleCombo.mouseClick(MOUSE_RIGHT);
      break;
    case 3:
      bleCombo.mouseClick(MOUSE_MIDDLE);
      break;
    case 4:
      bleCombo.mouseMove(0, 0, 20); // scroll down
      break;
    case 5:
      bleCombo.mouseMove(0, 0, -20); // scroll up
      break;
    case 6:
      bleCombo.mouseMove(0, 0, 0, 20); // scroll right
      break;
    case 7:
      bleCombo.mouseMove(0, 0, 0, -20); // scroll left
      break;
    }
    break;
  default:
    // If nothing matches do nothing
    break;
  }
}
