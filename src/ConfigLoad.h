/**
* @brief This function opens wificonfig.json and fills the wificonfig
*        struct accordingly.
*
* @param none
*
* @return True when succeeded. False otherwise.
*
* @note This is also where the sleep configuration lives.
*/
bool loadMainConfig()
{
  if (!FILESYSTEM.exists("/config/wificonfig.json"))
  {
    Serial.println("[WARNING]: Config file not found!");
    return false;
  }
  File configfile = FILESYSTEM.open("/config/wificonfig.json");

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, configfile);

  strlcpy(wificonfig.ssid, doc["ssid"] | "FAILED", sizeof(wificonfig.ssid));
  strlcpy(wificonfig.password, doc["password"] | "FAILED", sizeof(wificonfig.password));
  strlcpy(wificonfig.wifimode, doc["wifimode"] | "FAILED", sizeof(wificonfig.wifimode));
  strlcpy(wificonfig.hostname, doc["wifihostname"] | "freetouchdeck", sizeof(wificonfig.hostname));

  uint8_t attempts = doc["attempts"] | 10 ;
  wificonfig.attempts = attempts;

  uint16_t attemptdelay = doc["attemptdelay"] | 500 ;
  wificonfig.attemptdelay = attemptdelay;

  configfile.close();

  if (error)
  {
    Serial.println("[ERROR]: deserializeJson() error");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

/**
* @brief Helper function to load a single menu configuration
*
* @param menuIndex The menu index (0-4 for menu1-menu5)
* @param screenIcons The screen icons array to populate
* @param menuButtons The menu buttons array to populate
*
* @return bool True if successful, false otherwise
*/
bool loadMenuConfig(int menuIndex, Icons& screenIcons, Menu& menuButtons)
{
  char filename[32];
  sprintf(filename, "/config/menu%d.json", menuIndex + 1);
  
  File configfile = FILESYSTEM.open(filename, "r");
  if (!configfile) {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configfile);
  
  if (error) {
    Serial.println("[ERROR]: deserializeJson() error");
    Serial.println(error.c_str());
    configfile.close();
    return false;
  }

  // Load logos
  const char *logos[5] = {
    doc["logo0"] | "question.bmp",
    doc["logo1"] | "question.bmp", 
    doc["logo2"] | "question.bmp",
    doc["logo3"] | "question.bmp",
    doc["logo4"] | "question.bmp"
  };

  // Load latch logos
  const char *latchlogos[5] = {
    doc["button0"]["latchlogo"] | "question.bmp",
    doc["button1"]["latchlogo"] | "question.bmp",
    doc["button2"]["latchlogo"] | "question.bmp", 
    doc["button3"]["latchlogo"] | "question.bmp",
    doc["button4"]["latchlogo"] | "question.bmp"
  };

  // Load screen logos
  for (int i = 0; i < 5; i++) {
    strcpy(templogopath, logopath);
    strcat(templogopath, logos[i]);
    strcpy(screenIcons.icons[i], templogopath);
  }

  // Load button configurations
  for (int buttonIdx = 0; buttonIdx < 5; buttonIdx++) {
    char buttonKey[20];
    sprintf(buttonKey, "button%d", buttonIdx);
    
    // Load latch setting and latch logo
    menuButtons.buttons[buttonIdx].latch = doc[buttonKey]["latch"] | false;
    strcpy(templogopath, logopath);
    strcat(templogopath, latchlogos[buttonIdx]);
    strcpy(menuButtons.buttons[buttonIdx].latchlogo, templogopath);

    // Load action arrays
    JsonArray actionArray = doc[buttonKey]["actionarray"];
    JsonArray valueArray = doc[buttonKey]["valuearray"];

    int actions[3] = {actionArray[0], actionArray[1], actionArray[2]};
    
    // Set actions
    menuButtons.buttons[buttonIdx].actions.actions[0].action = actions[0];
    menuButtons.buttons[buttonIdx].actions.actions[1].action = actions[1]; 
    menuButtons.buttons[buttonIdx].actions.actions[2].action = actions[2];

    // Load values/symbols for each action
    for (int actionIdx = 0; actionIdx < 3; actionIdx++) {
      if (actions[actionIdx] == 4 || actions[actionIdx] == 8) {
        // Symbol/string value
        const char *symbol = valueArray[actionIdx];
        strcpy(menuButtons.buttons[buttonIdx].actions.actions[actionIdx].symbol, symbol);
      } else {
        // Numeric value
        int value = valueArray[actionIdx];
        menuButtons.buttons[buttonIdx].actions.actions[actionIdx].value = value;
      }
    }
  }

  configfile.close();
  return true;
}

/**
* @brief This function loads the menu configuration.
*
* @param String the config to be loaded
*
* @return none
*
* @note Options for values are: colors, homescreen, menu1, menu2, menu3
         menu4, and menu5
*/
bool loadConfig(String value)
{

  if (value == "general")
  {
    File configfile = FILESYSTEM.open("/config/general.json", "r");

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, configfile);

    // Parsing colors
    const char *menubuttoncolor = doc["menubuttoncolor"] | "#009bf4";         // Get the colour for the menu and back home buttons.
    const char *functionbuttoncolor = doc["functionbuttoncolor"] | "#00efcb"; // Get the colour for the function buttons.
    const char *latchcolor = doc["latchcolor"] | "#fe0149";                   // Get the colour to use when latching.
    const char *bgcolor = doc["background"] | "#000000";                      // Get the colour for the background.

    char menubuttoncolorchar[64];
    strcpy(menubuttoncolorchar, menubuttoncolor);
    unsigned long rgb888menubuttoncolor = convertHTMLtoRGB888(menubuttoncolorchar);
    generalconfig.menuButtonColour = convertRGB888ToRGB565(rgb888menubuttoncolor);

    char functionbuttoncolorchar[64];
    strcpy(functionbuttoncolorchar, functionbuttoncolor);
    unsigned long rgb888functionbuttoncolor = convertHTMLtoRGB888(functionbuttoncolorchar);
    generalconfig.functionButtonColour = convertRGB888ToRGB565(rgb888functionbuttoncolor);

    char latchcolorchar[64];
    strcpy(latchcolorchar, latchcolor);
    unsigned long rgb888latchcolor = convertHTMLtoRGB888(latchcolorchar);
    generalconfig.latchedColour = convertRGB888ToRGB565(rgb888latchcolor);

    char backgroundcolorchar[64];
    strcpy(backgroundcolorchar, bgcolor);
    unsigned long rgb888backgroundcolor = convertHTMLtoRGB888(backgroundcolorchar);
    generalconfig.backgroundColour = convertRGB888ToRGB565(rgb888backgroundcolor);

    // Loading general settings

         bool sleepenable = doc["sleepenable"] | false;
      if (sleepenable)
      {
        generalconfig.sleepenable = true;
        islatched[28] = 1;
      }
      else
      {
        generalconfig.sleepenable = false;
      }
    
      //uint16_t sleeptimer = doc["sleeptimer"];
      uint16_t sleeptimer = doc["sleeptimer"] | 60 ;
      generalconfig.sleeptimer = sleeptimer;
    
      bool beep = doc["beep"] | false;
      generalconfig.beep = beep;
    
      uint8_t modifier1 = doc["modifier1"] | 0 ;
      generalconfig.modifier1 = modifier1;
    
      uint8_t modifier2 = doc["modifier2"] | 0 ;
      generalconfig.modifier2 = modifier2;
    
      uint8_t modifier3 = doc["modifier3"] | 0 ;
      generalconfig.modifier3 = modifier3;
    
      uint16_t helperdelay = doc["helperdelay"] | 250 ;
      generalconfig.helperdelay = helperdelay;

    configfile.close();

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
      return false;
    }
    return true;
  }
  else if (value == "homescreen")
  {
    File configfile = FILESYSTEM.open("/config/homescreen.json", "r");

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, configfile);

    const char *logos[6] = {
      doc["logo0"] | "question.bmp",
      doc["logo1"] | "question.bmp", 
      doc["logo2"] | "question.bmp",
      doc["logo3"] | "question.bmp",
      doc["logo4"] | "question.bmp",
      doc["logo5"] | "question.bmp"  // Only screen 0 has 6 buttons
    };

    for (int i = 0; i < 6; i++)
    {
      strcpy(templogopath, logopath);
      strcat(templogopath, logos[i]);
      strcpy(screens[0].icons[i], templogopath);
    }

    configfile.close();

    if (error)
    {
      Serial.println("[ERROR]: deserializeJson() error");
      Serial.println(error.c_str());
      return false;
    }
    return true;

  // --------------------- Loading menus 1-5 ----------------------
  }
  else if (value == "menu1")
  {
    return loadMenuConfig(0, screens[1], menus[0]);
  }
  else if (value == "menu2")
  {
    return loadMenuConfig(1, screens[2], menus[1]);
  }
  else if (value == "menu3")
  {
    return loadMenuConfig(2, screens[3], menus[2]);
  }
  else if (value == "menu4")
  {
    return loadMenuConfig(3, screens[4], menus[3]);
  }
  else if (value == "menu5")
  {
    return loadMenuConfig(4, screens[5], menus[4]);
  }
  else
  {
    return false;
  }
}
