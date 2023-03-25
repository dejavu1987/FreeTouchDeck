/**
* @brief This function returns all the files in a given directory in a json
         formatted string.
*
* @param path String
*
* @return String
*
* @note none
*/
String handleFileList(String path) {

  File root = FILESYSTEM.open(path);
  path = String();
  int filecount = 0;

  String output = "[";
  if (root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      if (output != "[") {
        output += ',';
      }

      output += "{\"";
      output += filecount;
      output += "\":\"";

#ifdef ESP_ARDUINO_VERSION_MAJOR
      output += String(file.name());
#else
      output += String(file.name()).substring(7);
#endif

      output += "\"}";
      file = root.openNextFile();
      filecount++;
    }

    file.close();
  }
  output += "]";
  root.close();
  return output;
}

String handleAPISList() {

  File root = FILESYSTEM.open("/uploads");

  int filecount = 0;

  String output = "[";
  if (root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      String filepath = String(file.name()).substring(0, 16);
      if (filepath == "/uploads/config_") {

        file = root.openNextFile();
        filecount++;

      } else {
        String filename = String(file.name()).substring(9);
        if (output != "[") {
          output += ',';
        }

        output += "{\"";
        output += filecount;
        output += "\":\"";
        output += String(file.name()).substring(9);
        output += "\"}";
        file = root.openNextFile();
        filecount++;
      }
    }
    file.close();
  }
  output += "]";
  root.close();
  return output;
}

/**
* @brief This function returns information about FreeTouchDeck in a json
         formatted string.
*
* @param none
*
* @return String
*
* @note none
*/
String handleInfo() {

  float freemem = FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes();

  String output = "[";

  output += "{\"";
  output += "Version";
  output += "\":\"";
  output += String(versionnumber);
  output += "\"},";

  output += "{\"";
  output += "Free Space";
  output += "\":\"";
  output += String(freemem / 1000);
  output += " kB\"},";

#if defined(USEUSBHID)

  output += "{\"";
  output += "Keyboard Type";
  output += "\":\"";
  output += String("Using USB");
  output += "\"},";

#else

  output += "{\"";
  output += "BLE Keyboard Version";
  output += "\":\"";
  output += String(BLE_KEYBOARD_VERSION);
  output += "\"},";

#endif // if defined(USEUSBHID)

  output += "{\"";
  output += "ArduinoJson Version";
  output += "\":\"";
  output += String(ARDUINOJSON_VERSION);
  output += "\"},";

  output += "{\"";
  output += "TFT_eSPI Version";
  output += "\":\"";
  output += String(TFT_ESPI_VERSION);
  output += "\"},";

  output += "{\"";
  output += "ESP-IDF";
  output += "\":\"";
  output += String(esp_get_idf_version());
  output += "\"},";

#ifdef touchInterruptPin
  output += "{\"";
  output += "Sleep";
  output += "\":\"";
  if (generalconfig.sleepenable) {
    output += String("Enabled. ");
    output += String("Timer: ");
    output += String(generalconfig.sleeptimer);
    output += String(" minutes");
    output += "\"}";
  } else {
    output += String("Disabled");
    output += "\"}";
  }
#else
  output += "{\"";
  output += "Sleep";
  output += "\":\"";
  output += String("Disabled");
  output += "\"}";

#endif

  output += "]";

  return output;
}

/* --------------- Checking for free space on FILESYSTEM ----------------
Purpose: This checks if the free memory on the FILESYSTEM is bigger then a set
threshold Input  : none Output : boolean Note   : none
*/

bool spaceLeft() {
  float minmem = 100000.00; // Always leave 100 kB free space on FILESYSTEM
  float freeMemory = FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes();
  Serial.printf("[INFO]: Free memory left: %f bytes\n", freeMemory);
  if (freeMemory < minmem) {
    return false;
  }

  return true;
}
