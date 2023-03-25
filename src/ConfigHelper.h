
/**
* @brief This function checks if a file exists and returns a boolean
accordingly. It then prints a debug message to the serial as wel as the tft.
*
* @param filename (const char *)
*
* @return boolean True if succeeded. False otherwise.
*
* @note Pass the filename including a leading /
*/
bool checkfile(const char *filename) {

  if (!FILESYSTEM.exists(filename)) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(1, 3);
    tft.setTextFont(2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("%s not found!\n\n", filename);
    tft.setTextSize(1);
    tft.printf("If this has happend after confguration, the data on the ESP "
               "may \nbe corrupted.");
    return false;
  } else {
    return true;
  }
}

bool resetconfig(String file) {

  if (file != "menu1" && file != "menu2" && file != "menu3" &&
      file != "menu4" && file != "menu5" && file != "homescreen" &&
      file != "general") {
    Serial.println("[WARNING]: Invalid reset option. Choose: menu1, menu2, "
                   "menu3, menu4, menu5, homescreen, or general");
    return false;
  }

  if (file == "menu1" || file == "menu2" || file == "menu3" ||
      file == "menu4" || file == "menu5") {
    // Reset a menu config

    // Delete the corrupted json file
    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json")) {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    // Copy default.json to the new config file
    File defaultfile = FILESYSTEM.open("/config/default.json", "r");

    size_t  n;
    uint8_t buf[64];

    if (defaultfile) {
      File newfile = FILESYSTEM.open(filetoremove, "w");
      if (newfile) {
        while ((n = defaultfile.read(buf, sizeof(buf))) > 0) {
          newfile.write(buf, n);
        }
        // Close the newly created file
        newfile.close();
      }
      Serial.println("[INFO]: Done resetting.");
      Serial.println("[INFO]: Type \"restart\" to reload configuration.");

      // Close the default.json file
      defaultfile.close();
      return true;
    }

  } else if (file == "homescreen") {

    // Reset the homescreen
    // For this we do not need to open a default file because we can easily
    // write it ourselfs
    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json")) {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.println("{");
    newfile.println("\"logo0\": \"question.bmp\",");
    newfile.println("\"logo1\": \"question.bmp\",");
    newfile.println("\"logo2\": \"question.bmp\",");
    newfile.println("\"logo3\": \"question.bmp\",");
    newfile.println("\"logo4\": \"question.bmp\",");
    newfile.println("\"logo5\": \"settings.bmp\"");
    newfile.println("}");

    newfile.close();
    Serial.println("[INFO]: Done resetting homescreen.");
    Serial.println("[INFO]: Type \"restart\" to reload configuration.");
    return true;

  } else if (file == "general") {

    // Reset the general config
    // For this we do not need to open a default file because we can easily
    // write it ourselfs

    String filetoremove = "/config/" + file;
    if (!filetoremove.endsWith(".json")) {
      filetoremove = filetoremove + ".json";
    }

    FILESYSTEM.remove(filetoremove);

    File newfile = FILESYSTEM.open(filetoremove, "w");
    newfile.println("{");
    newfile.println("\"menubuttoncolor\": \"#009bf4\",");
    newfile.println("\"functionbuttoncolor\": \"#00efcb\",");
    newfile.println("\"latchcolor\": \"#fe0149\",");
    newfile.println("\"background\": \"#000000\",");
    newfile.println("\"sleepenable\": true,");
    newfile.println("\"sleeptimer\": 10,");
    newfile.println("\"beep\": true,");
    newfile.println("\"modifier1\": 130,");
    newfile.println("\"modifier2\": 129,");
    newfile.println("\"modifier3\": 0,");
    newfile.println("\"helperdelay\": 500");
    newfile.println("}");

    newfile.close();
    Serial.println("[INFO]: Done resetting general config.");
    Serial.println("[INFO]: Type \"restart\" to reload configuration.");
    return true;

  } else {
    return false;
  }

  return false;
}
