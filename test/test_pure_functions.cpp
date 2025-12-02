#include <iostream>
#include <cassert>
#include <cstring>
#include <stdint.h>

// Include the pure function
#include "../src/LatchImageHelper.h"

// Mock function for getBMPColor
uint16_t mockGetBMPColor(const char* filename) {
    if (strcmp(filename, "/logos/custom1.bmp") == 0) return 0x1111;
    if (strcmp(filename, "/logos/custom2.bmp") == 0) return 0x2222;
    if (strcmp(filename, "/logos/screen_logo0.bmp") == 0) return 0x3333;
    if (strcmp(filename, "/logos/screen_logo1.bmp") == 0) return 0x4444;
    if (strcmp(filename, "/logos/screen_logo2.bmp") == 0) return 0x5555;
    return 0x0000; // Default for unknown files
}

void test_getLatchLogoPath() {
    std::cout << "Testing getLatchLogoPath..." << std::endl;
    
    const char* menuButtons[5] = {
        "/logos/custom1.bmp",
        "/logos/",
        "/logos/custom2.bmp",
        "/logos/",
        "/logos/custom1.bmp"
    };
    
    // Test valid inputs
    assert(strcmp(getLatchLogoPath(menuButtons, 0), "/logos/custom1.bmp") == 0);
    assert(strcmp(getLatchLogoPath(menuButtons, 1), "/logos/") == 0);
    assert(strcmp(getLatchLogoPath(menuButtons, 2), "/logos/custom2.bmp") == 0);
    
    // Test invalid inputs
    assert(getLatchLogoPath(menuButtons, -1) == nullptr);
    assert(getLatchLogoPath(menuButtons, 5) == nullptr);
    
    std::cout << "✓ getLatchLogoPath tests passed!" << std::endl;
}

void test_getLatchImageBGPure_custom_logos() {
    std::cout << "Testing getLatchImageBGPure with custom logos..." << std::endl;
    
    const char* menuButtons[5] = {
        "/logos/custom1.bmp",
        "/logos/custom2.bmp",
        "/logos/custom1.bmp",
        "/logos/custom2.bmp",
        "/logos/custom1.bmp"
    };
    
    const char* screenLogos[5] = {
        "/logos/screen_logo0.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp"
    };
    
    // Test custom logos
    uint16_t result = getLatchImageBGPure(1, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x1111); // custom1.bmp
    
    result = getLatchImageBGPure(1, 1, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x2222); // custom2.bmp
    
    std::cout << "✓ Custom logo tests passed!" << std::endl;
}

void test_getLatchImageBGPure_fallback_logos() {
    std::cout << "Testing getLatchImageBGPure with fallback logos..." << std::endl;
    
    const char* menuButtons[5] = {
        "/logos/",  // Should fall back to screen logo
        "/logos/",  // Should fall back to screen logo
        "/logos/custom2.bmp",
        "/logos/",  // Should fall back to screen logo
        "/logos/custom1.bmp"
    };
    
    const char* screenLogos[5] = {
        "/logos/screen_logo0.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp"
    };
    
    // Test fallback to screen logos
    uint16_t result = getLatchImageBGPure(1, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x3333); // screen_logo0.bmp
    
    result = getLatchImageBGPure(1, 1, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x4444); // screen_logo1.bmp
    
    result = getLatchImageBGPure(1, 3, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x4444); // screen_logo1.bmp
    
    std::cout << "✓ Fallback logo tests passed!" << std::endl;
}

void test_getLatchImageBGPure_invalid_inputs() {
    std::cout << "Testing getLatchImageBGPure with invalid inputs..." << std::endl;
    
    const char* menuButtons[5] = {
        "/logos/custom1.bmp",
        "/logos/",
        "/logos/custom2.bmp",
        "/logos/",
        "/logos/custom1.bmp"
    };
    
    const char* screenLogos[5] = {
        "/logos/screen_logo0.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp"
    };
    
    // Test invalid page numbers
    uint16_t result = getLatchImageBGPure(0, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    result = getLatchImageBGPure(6, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    result = getLatchImageBGPure(-1, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    // Test invalid button numbers
    result = getLatchImageBGPure(1, -1, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    result = getLatchImageBGPure(1, 5, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    result = getLatchImageBGPure(1, 10, menuButtons, screenLogos, mockGetBMPColor);
    assert(result == 0x0000);
    
    std::cout << "✓ Invalid input tests passed!" << std::endl;
}

void test_getLatchImageBGPure_boundary_values() {
    std::cout << "Testing getLatchImageBGPure with boundary values..." << std::endl;
    
    const char* menuButtons[5] = {
        "/logos/custom1.bmp",
        "/logos/custom2.bmp",
        "/logos/custom1.bmp",
        "/logos/custom2.bmp",
        "/logos/custom1.bmp"
    };
    
    const char* screenLogos[5] = {
        "/logos/screen_logo0.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp",
        "/logos/screen_logo1.bmp",
        "/logos/screen_logo2.bmp"
    };
    
    // Test boundary values for valid ranges
    uint16_t result = getLatchImageBGPure(1, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result != 0x0000); // Should be valid
    
    result = getLatchImageBGPure(1, 4, menuButtons, screenLogos, mockGetBMPColor);
    assert(result != 0x0000); // Should be valid
    
    result = getLatchImageBGPure(5, 0, menuButtons, screenLogos, mockGetBMPColor);
    assert(result != 0x0000); // Should be valid
    
    std::cout << "✓ Boundary value tests passed!" << std::endl;
}

int main() {
    std::cout << "Running pure function tests..." << std::endl;
    std::cout << "===============================" << std::endl;
    
    test_getLatchLogoPath();
    test_getLatchImageBGPure_custom_logos();
    test_getLatchImageBGPure_fallback_logos();
    test_getLatchImageBGPure_invalid_inputs();
    test_getLatchImageBGPure_boundary_values();
    
    std::cout << "===============================" << std::endl;
    std::cout << "🎉 All tests passed!" << std::endl;
    
    return 0;
}