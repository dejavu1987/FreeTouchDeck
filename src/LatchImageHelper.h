#ifndef LATCH_IMAGE_HELPER_H
#define LATCH_IMAGE_HELPER_H

#include <stdint.h>

/**
 * @brief Pure function to get latch logo path for a specific menu and button
 * 
 * @param menuButtons Array of 5 button latch logo paths for the menu
 * @param buttonNum Button number (0-4)
 * 
 * @return const char* - path to latch logo or nullptr if invalid
 */
const char* getLatchIconPath(const char* menuButtons[5], int buttonNum) {
    if (buttonNum < 0 || buttonNum > 4) {
        return nullptr;
    }
    return menuButtons[buttonNum];
}

/**
 * @brief Pure function to determine background color for a latch image
 * 
 * @param pageNum Current page number (1-5)
 * @param logonumber Button number (0-4)
 * @param menuButtons Array of 5 button latch logo paths for current menu
 * @param screenLogos Array of 5 screen logo paths for current screen
 * @param getBMPColorFunc Function pointer to get color from BMP file
 * 
 * @return uint16_t RGB565 color value
 */
uint16_t getLatchImageBGPure(
    int pageNum, 
    int logonumber,
    const char* menuButtons[5],
    const char* screenLogos[5],
    uint16_t (*getBMPColorFunc)(const char*)
) {
    // Handle invalid inputs
    if (pageNum < 1 || pageNum > 5 || logonumber < 0 || logonumber > 4) {
        return 0x0000;
    }
    
    // Get the latch logo path for this button
    const char* latchLogoPath = getLatchIconPath(menuButtons, logonumber);
    if (latchLogoPath == nullptr) {
        return 0x0000;
    }
    
    // If latch logo is default/empty ("/logos/"), fall back to regular screen logo
    if (strcmp(latchLogoPath, "/logos/") == 0) {
        if (screenLogos[logonumber] == nullptr) {
            return 0x0000;
        }
        return getBMPColorFunc(screenLogos[logonumber]);
    }
    
    // Use the custom latch logo
    return getBMPColorFunc(latchLogoPath);
}

#endif // LATCH_IMAGE_HELPER_H