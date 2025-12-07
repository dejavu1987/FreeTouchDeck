/**
* @brief This function reads chuncks of 2 bytes of data from a
         file and returns the data.
*
* @param &f
*
* @return uint16_t
*
* @note litte-endian
*/
uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

/**
* @brief This function reads chuncks of 4 bytes of data from a
         file and returns the data.
*
* @param &f
*
* @return uint32_t
*
* @note litte-endian
*/
uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

/**
* @brief This functions accepts a HTML including the # colour code 
         (eg. #FF00FF)  and returns it in RGB888 format.
*
* @param *html char (including #)
*
* @return unsigned long
*
* @note none
*/
unsigned long convertHTMLtoRGB888(char *html)
{
  char *hex = html + 1; // remove the #
  unsigned long rgb = strtoul(hex, NULL, 16);
  return rgb;
}

/**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
unsigned int convertRGB888ToRGB565(unsigned long rgb)
{
  return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
}

/**
* @brief Internal function that draws a BMP on the TFT screen according
         to the given x and y coordinates. Supports 1-bit, 4-bit, 16-bit, and 24-bit BMPs
*
* @param  *filename
* @param x int16_t 
* @param y int16_t 
* @param transparent bool - if true, black pixels (0x0000) are not drawn
*
* @return none
*/
void drawBmpInternal(const char *filename, int16_t x, int16_t y, bool transparent)
{
  
  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (bmpFS.size() == 0)
  {
    if (transparent) {
      Serial.println("[WARNING]: Bitmap not found: ");
      Serial.println(filename);
      filename = "/sys/ico/question.bmp";
      bmpFS = FILESYSTEM.open(filename, "r");
    } else {
      Serial.print("File not found:");
      Serial.println(filename);
      return;
    }
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;
  uint16_t bitsPerPixel;

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);
    read16(bmpFS); // planes
    bitsPerPixel = read16(bmpFS);
    read32(bmpFS); // compression
  
    if (bitsPerPixel == 24) {
      // Original 24-bit BMP handling
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        if (transparent) {
          tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer, TFT_BLACK);
        } else {
          tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
        }
      }
      tft.setSwapBytes(oldSwapBytes);
    }
    else if (bitsPerPixel == 1) {
      // 1-bit BMP handling (monochrome)
      y += h - 1;
      
      // Read color table from BMP file
      uint16_t palette[2];
      bmpFS.seek(0x36); // Color table starts at offset 0x36 for 1-bit
      for (int i = 0; i < 2; i++) {
        uint8_t b = bmpFS.read();
        uint8_t g = bmpFS.read();
        uint8_t r = bmpFS.read();
        bmpFS.read(); // Skip alpha/reserved byte
        palette[i] = tft.color565(r, g, b);
      }
      
      bmpFS.seek(seekOffset);
      uint16_t bytesPerLine = (w + 7) / 8;
      uint16_t padding = (4 - (bytesPerLine & 3)) & 3;
      uint8_t lineBuffer[bytesPerLine + padding];
      uint16_t pixelBuffer[w];

      for (row = 0; row < h; row++)
      {
        bmpFS.read(lineBuffer, bytesPerLine + padding);
        
        for (uint16_t col = 0; col < w; col++)
        {
          uint8_t byteIndex = col / 8;
          uint8_t bitIndex = 7 - (col % 8);
          uint8_t pixelValue = (lineBuffer[byteIndex] >> bitIndex) & 1;
          pixelBuffer[col] = palette[pixelValue];
        }
        
        if (transparent) {
          tft.pushImage(x, y--, w, 1, pixelBuffer, 0x0000);
        } else {
          tft.pushImage(x, y--, w, 1, pixelBuffer);
        }
      }
    }
    else if (bitsPerPixel == 16) {
      // 16-bit BMP handling (RGB565 or RGB555)
      y += h - 1;
      
      bmpFS.seek(seekOffset);
      uint16_t bytesPerLine = w * 2; // 2 bytes per pixel
      uint16_t padding = (4 - (bytesPerLine & 3)) & 3;
      uint8_t lineBuffer[bytesPerLine + padding];
      uint16_t pixelBuffer[w];

      for (row = 0; row < h; row++)
      {
        bmpFS.read(lineBuffer, bytesPerLine + padding);
        
        for (uint16_t col = 0; col < w; col++)
        {
          uint16_t pixelValue = lineBuffer[col * 2] | (lineBuffer[col * 2 + 1] << 8);
          // Convert RGB555 to RGB565 if needed (most 16-bit BMPs are RGB565 already)
          pixelBuffer[col] = pixelValue;
        }
        
        if (transparent) {
          tft.pushImage(x, y--, w, 1, pixelBuffer, 0x0000);
        } else {
          tft.pushImage(x, y--, w, 1, pixelBuffer);
        }
      }
    }
    else if (bitsPerPixel == 4) {
      // 4-bit BMP handling (16 colors)
      y += h - 1;
      
      // Read color table from BMP file
      uint16_t palette[16];
      bmpFS.seek(0x36); // Color table starts at offset 0x36
      for (int i = 0; i < 16; i++) {
        uint8_t b = bmpFS.read();
        uint8_t g = bmpFS.read();
        uint8_t r = bmpFS.read();
        bmpFS.read(); // Skip alpha/reserved byte
        palette[i] = tft.color565(r, g, b);
      }
      
      bmpFS.seek(seekOffset);
      uint16_t bytesPerLine = (w + 1) / 2;
      uint16_t padding = (4 - (bytesPerLine & 3)) & 3;
      uint8_t lineBuffer[bytesPerLine + padding];
      uint16_t pixelBuffer[w];

      for (row = 0; row < h; row++)
      {
        bmpFS.read(lineBuffer, bytesPerLine + padding);
        
        for (uint16_t col = 0; col < w; col++)
        {
          uint8_t byteIndex = col / 2;
          uint8_t pixelValue;
          if (col % 2 == 0) {
            pixelValue = (lineBuffer[byteIndex] >> 4) & 0x0F; // Upper 4 bits
          } else {
            pixelValue = lineBuffer[byteIndex] & 0x0F; // Lower 4 bits
          }
          pixelBuffer[col] = palette[pixelValue];
        }
        
        if (transparent) {
          tft.pushImage(x, y--, w, 1, pixelBuffer, 0x0000);
        } else {
          tft.pushImage(x, y--, w, 1, pixelBuffer);
        }
      }
    }
    else {
      Serial.printf("[WARNING]: BMP format not supported: %d bpp\n", bitsPerPixel);
    }
  }
  bmpFS.close();
}

/**
* @brief This function draws a BMP on the TFT screen according
         to the given x and y coordinates. Supports 1-bit, 4-bit, 16-bit, and 24-bit BMPs
         Does not draw black pixels, so that background images with logos can be combined. 
*
* @param  *filename
* @param x int16_t 
* @param y int16_t 
*
* @return none
*
* @note A completely black pixel is transparent e.g. (0x0000) not drawn.
*/
void drawBmpTransparent(const char *filename, int16_t x, int16_t y)
{
  drawBmpInternal(filename, x, y, true);
}

/**
* @brief This function draws a BMP on the TFT screen according
         to the given x and y coordinates. Supports 1-bit, 4-bit, 16-bit, and 24-bit BMPs
*
* @param  *filename
* @param x int16_t 
* @param y int16_t 
*
* @return none
*
* @note In contradiction to drawBmpTransparent() this does draw black pixels.
*/
void drawBmp(const char *filename, int16_t x, int16_t y)
{
  drawBmpInternal(filename, x, y, false);
}

/**
* @brief This function reads a number of bytes from the given
         file at the given position.
*
* @param *p_file File
* @param position int
* @param nBytes byte 
*
* @return int32_t
*
* @note none
*/
int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
  if (nBytes > 4)
    return 0;

  p_file->seek(position);

  int32_t weight = 1;
  int32_t result = 0;
  for (; nBytes; nBytes--)
  {
    result += weight * p_file->read();
    weight <<= 8;
  }
  return result;
}

/**
* @brief This function reads the RGB565 colour of the first pixel for a
         given file. Supports 1-bit, 2-bit, 3-bit, and 24-bit BMPs
*
* @param *filename const char
*
* @return uint16_t
*
* @note Uses readNbytesInt
*/
uint16_t getBMPColor(const char *filename)
{

  // Open File
  File bmpImage;
  bmpImage = FILESYSTEM.open(filename, FILE_READ);

  if (bmpImage.size() == 0)
  {
    Serial.println("[WARNING]: getBMPColor: File not found");
    return 0x0000;
  }

  int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);
  int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

  if (pixelsize == 24)
  {
    bmpImage.seek(dataStartingOffset); //skip bitmap header

    byte R, G, B;

    B = bmpImage.read();
    G = bmpImage.read();
    R = bmpImage.read();

    bmpImage.close();

    return tft.color565(R, G, B);
  }
  else if (pixelsize == 1)
  {
    // 1-bit BMP - read color from palette
    uint16_t palette[2];
    bmpImage.seek(0x36); // Color table starts at offset 0x36
    for (int i = 0; i < 2; i++) {
      uint8_t b = bmpImage.read();
      uint8_t g = bmpImage.read();
      uint8_t r = bmpImage.read();
      bmpImage.read(); // Skip alpha/reserved byte
      palette[i] = tft.color565(r, g, b);
    }
    
    bmpImage.seek(dataStartingOffset);
    uint8_t firstByte = bmpImage.read();
    uint8_t firstPixel = (firstByte >> 7) & 1; // Get MSB (first pixel)
    bmpImage.close();
    return palette[firstPixel];
  }
  else if (pixelsize == 16)
  {
    // 16-bit BMP - direct color (RGB565 or RGB555)
    bmpImage.seek(dataStartingOffset);
    uint8_t lowByte = bmpImage.read();
    uint8_t highByte = bmpImage.read();
    uint16_t pixelValue = lowByte | (highByte << 8);
    bmpImage.close();
    return pixelValue; // Assume RGB565 format
  }
  else if (pixelsize == 4)
  {
    // 4-bit BMP - read color from palette
    uint16_t palette[16];
    bmpImage.seek(0x36); // Color table starts at offset 0x36
    for (int i = 0; i < 16; i++) {
      uint8_t b = bmpImage.read();
      uint8_t g = bmpImage.read();
      uint8_t r = bmpImage.read();
      bmpImage.read(); // Skip alpha/reserved byte
      palette[i] = tft.color565(r, g, b);
    }
    
    bmpImage.seek(dataStartingOffset);
    uint8_t firstByte = bmpImage.read();
    uint8_t firstPixel = (firstByte >> 4) & 0x0F; // Get upper 4 bits (first pixel)
    bmpImage.close();
    return palette[firstPixel];
  }
  else
  {
    Serial.printf("[WARNING]: getBMPColor: Unsupported bit depth: %d bpp\n", pixelsize);
    bmpImage.close();
    return 0x0000;
  }
}

/**
* @brief This function returns the RGB565 colour of the first pixel for a
         given the logo number. The pagenumber is global.
*
* @param logonumber int
*
* @return uint16_t
*
* @note Uses getBMPColor to read the actual image data.
*/
uint16_t getImageBG(int iconNumber)
{
  // Logo 5 on each screen is the back home button except on the home screen
  if (iconNumber == 5 && pageNum > 0)
  {
    return getBMPColor("/logos/home.bmp");
  }

  // Bounds checking
  if (pageNum < 0 || pageNum > 6 || iconNumber < 0 || iconNumber >= 6)
  {
    return 0x0000;
  }
  
  // Screen6 has no logos
  if (pageNum == 6)
  {
    return 0x0000;
  }
  
  // Use global screens array
  return getBMPColor(screens[pageNum].icons[iconNumber]);
}

#include "LatchImageHelper.h"

/**
* @brief Helper function to get latch logo path for a specific menu and button
*
* @param menuNum int - menu number (1-5)
* @param buttonNum int - button number (0-4)
*
* @return const char* - path to latch logo or nullptr if invalid
*/
const char* getLatchIconPath(int pageNum, int buttonNum)
{
  // Bounds checking
  if (pageNum < 1 || pageNum > 5 || buttonNum < 0 || buttonNum >= 5)
  {
    return nullptr;
  }
  
  // Use global menus array (pageNum 1-5 maps to array indices 0-4)
  return menus[pageNum - 1].buttons[buttonNum].latchlogo;
}

/**
* @brief This function returns the RGB565 colour of the first pixel of the image which
*          is being latched to for a given the logo number. The pagenumber is global.
*
* @param logonumber int
*
* @return uint16_t
*
* @note Uses getBMPColor to read the actual image data.
*/
uint16_t getLatchImageBG(int logonumber)
{
  // Bounds checking
  if (pageNum < 1 || pageNum > 5 || logonumber < 0 || logonumber >= 5)
  {
    return 0x0000;
  }
  
  // Prepare arrays for pure function
  const char* menuButtons[5];
  const char* screenLogos[5];
  
  for (int i = 0; i < 5; i++)
  {
    menuButtons[i] = menus[pageNum - 1].buttons[i].latchlogo;
    screenLogos[i] = screens[pageNum].icons[i];
  }
  
  return getLatchImageBGPure(pageNum, logonumber, menuButtons, screenLogos, getBMPColor);
}
