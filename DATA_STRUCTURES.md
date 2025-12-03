# FreeTouchDeck Data Structure Documentation

This document provides comprehensive documentation of all data structures and data objects used in the FreeTouchDeck project.

## Table of Contents

1. [C++ Data Structures](#c-data-structures)
2. [JSON Configuration Files](#json-configuration-files)
3. [Action System](#action-system)
4. [Global Variables](#global-variables)

---

## C++ Data Structures

### Core Structures

#### `struct Icons`
Holds icon file paths for each screen.
```cpp
struct Icons {
  char icons[6][32];  // 6 icons per screen, max 32 chars per path
};
```
- **Purpose**: Stores file paths to bitmap images displayed on buttons
- **Usage**: One instance per menu screen (screen0-screen6)
- **Capacity**: 6 icon paths per screen, 32 character limit per path

#### `struct Action`
Defines a single action configuration.
```cpp
struct Action {
  uint8_t action;    // Action type (0-14)
  uint8_t value;     // Action value
  char    symbol[64]; // Action symbol/text (64 chars max)
};
```
- **Purpose**: Stores a single action with its type, value, and optional symbol
- **Action**: Action type ranging from 0-14
- **Value**: Numeric value associated with the action
- **Symbol**: Used for text input or special characters

#### `struct Actions`
Defines the action configuration for a button.
```cpp
struct Actions {
  struct Action actions[3];  // Array of 3 actions
};
```
- **Purpose**: Stores up to 3 actions per button
- **Actions**: Array of Action structs, executed sequentially
- **Capacity**: Fixed array of 3 actions per button

#### `struct Button`
Represents a single button configuration.
```cpp
struct Button {
  struct Actions actions;   // Button's action configuration
  bool           latch;     // Whether button latches (toggles state)
  char           latchlogo[32];  // Logo to show when latched (32 chars max)
};
```
- **Purpose**: Complete button definition including actions and latch behavior
- **Latch**: When true, button toggles between pressed/unpressed states
- **Latchlogo**: Alternative image shown when button is in latched state

#### `struct Menu`
Represents a complete menu with 6 buttons.
```cpp
struct Menu {
  struct Button buttons[6];  // 6 buttons per menu
};
```
- **Purpose**: Groups 6 buttons into a functional menu
- **Layout**: Arranged in 2x3 grid on screen

#### `struct SystemIcons`
Holds paths to system-level icons.
```cpp
struct SystemIcons {
  char settings[64];      // Settings icon path
  char homebutton[64];    // Home button icon path
  char configurator[64];  // Configurator icon path
};
```
- **Purpose**: System UI elements that appear across all menus

### Configuration Structures

#### `struct Config`
General system configuration and appearance settings.
```cpp
struct Config {
  uint16_t menuButtonColour;      // Menu button background color
  uint16_t functionButtonColour;  // Function button background color
  uint16_t backgroundColour;      // Screen background color
  uint16_t latchedColour;        // Color when button is latched
  bool     sleepenable;          // Enable/disable sleep mode
  uint16_t sleeptimer;           // Sleep timeout in minutes
  bool     beep;                 // Enable/disable button beep sound
  uint8_t  modifier1;            // First global modifier key (0=none, 128=CTRL, 129=SHIFT, 130=ALT, 131=GUI)
  uint8_t  modifier2;            // Second global modifier key
  uint8_t  modifier3;            // Third global modifier key
  uint16_t helperdelay;          // Delay after helper actions (milliseconds)
};
```
- **Colors**: 16-bit color values (RGB565 format)
- **Sleep**: Automatic sleep functionality
- **Modifiers**: Global modifier keys applied to helper functions
- **Helper delay**: Pause after executing helper functions

#### `struct Wificonfig`
WiFi connection and network configuration.
```cpp
struct Wificonfig {
  char     ssid[64];        // WiFi network name (64 chars max)
  char     password[64];    // WiFi password (64 chars max)  
  char     wifimode[9];     // WiFi mode: "WIFI_STA" or "WIFI_AP"
  char     hostname[64];    // Device hostname (64 chars max)
  uint8_t  attempts;        // Connection retry attempts
  uint16_t attemptdelay;    // Delay between attempts (milliseconds)
};
```
- **Modes**: Station mode (WIFI_STA) or Access Point mode (WIFI_AP)
- **Connection**: Retry logic with configurable attempts and delays

---

## JSON Configuration Files

### Menu Configuration Files (`menu1.json` - `menu5.json`)

Each menu configuration follows this structure:

```json
{
  "logo0": "image1.bmp",
  "logo1": "image2.bmp", 
  "logo2": "image3.bmp",
  "logo3": "image4.bmp",
  "logo4": "image5.bmp",
  "button0": {
    "latch": false,
    "latchlogo": "",
    "actionarray": ["5", "4", "0"],
    "valuearray": ["1", "t", "0"]
  },
  "button1": { /* ... similar structure ... */ },
  "button2": { /* ... similar structure ... */ },
  "button3": { /* ... similar structure ... */ },
  "button4": { /* ... similar structure ... */ }
}
```

#### Field Descriptions:
- **`logo0-logo4`**: File paths to button images (5 buttons + 1 home button)
- **`button0-button4`**: Button configurations (button5 is reserved for home)

#### Button Object:
- **`latch`**: Boolean - whether button toggles state
- **`latchlogo`**: String - alternative image when latched
- **`actionarray`**: Array of 3 action type strings
- **`valuearray`**: Array of 3 value strings (numbers or characters)

### Home Screen Configuration (`homescreen.json`)

```json
{
  "logo0": "multimedia.bmp",      // Menu 1 icon
  "logo1": "video-wireless.bmp",  // Menu 2 icon
  "logo2": "youtube.bmp",         // Menu 3 icon
  "logo3": "refresh.bmp",         // Menu 4 icon
  "logo4": "google-chrome.bmp",   // Menu 5 icon
  "logo5": "sys/ico/settings.bmp" // Settings icon
}
```

### General Configuration (`general.json`)

```json
{
  "menubuttoncolor": "#510101",     // Hex color for menu buttons
  "functionbuttoncolor": "#3c0548", // Hex color for function buttons
  "latchcolor": "#f9b31a",         // Hex color for latched buttons
  "background": "#000000",          // Hex color for background
  "sleepenable": true,             // Boolean - enable sleep mode
  "beep": false,                   // Boolean - enable button beep
  "sleeptimer": 20,                // Integer - sleep timeout (minutes)
  "modifier1": 0,                  // Integer - global modifier key 1
  "modifier2": 0,                  // Integer - global modifier key 2
  "modifier3": 0,                  // Integer - global modifier key 3
  "helperdelay": 0                 // Integer - helper delay (milliseconds)
}
```

### WiFi Configuration (`wificonfig.json`)

```json
{
  "ssid": "YourWiFiName",          // String - WiFi network name
  "password": "YourPassword",       // String - WiFi password
  "wifimode": "WIFI_STA",          // String - "WIFI_STA" or "WIFI_AP"
  "wifihostname": "freetouchdeck", // String - device hostname
  "attempts": 20,                  // Integer - connection retry attempts
  "attemptdelay": 1000             // Integer - delay between attempts (ms)
}
```

### Default Configuration Template (`default.json`)

Provides template structure for menu configurations:

```json
{
  "logo0": "question.bmp",
  "logo1": "question.bmp",
  "logo2": "question.bmp", 
  "logo3": "question.bmp",
  "logo4": "question.bmp",
  "button0": {
    "latch": false,
    "latchlogo": "",
    "actionarray": ["0", "0", "0"],
    "valuearray": ["0", "0", "0"]
  },
  // ... similar structure for button1-button4
}
```

---

## Action System

The action system supports 15 different action types (0-14), each with specific value interpretations:

### Action Types

| Action | Type | Description | Value Range | Symbol Usage |
|--------|------|-------------|-------------|--------------|
| 0 | No Action | Does nothing | N/A | No |
| 1 | Delay | Pause execution | Milliseconds | No |
| 2 | Navigation Keys | Arrow keys, Tab, etc. | 1-14 (see table below) | No |
| 3 | Media Keys | Volume, playback control | 1-7 (see table below) | No |
| 4 | Send Character | Print text string | N/A | Yes (text to print) |
| 5 | Modifier Keys | Ctrl, Shift, Alt, GUI | 1-9 (see table below) | No |
| 6 | Function Keys | F1-F24 | 1-24 | No |
| 7 | Send Number | Print numeric value | Any integer | No |
| 8 | Send Special Character | Print symbol | N/A | Yes (character to print) |
| 9 | Key Combinations | Multiple modifier keys | 1-14 (see table below) | No |
| 10 | Helper Functions | Configurable shortcuts | 1-8 | No |
| 11 | Special Functions | System functions | 1-4 (see table below) | No |
| 12 | Numpad | Numeric keypad keys | 0-15 (see table below) | No |
| 13 | Custom Functions | User-defined actions | 1-7 | No |
| 14 | Mouse Actions | Mouse clicks and scroll | 1-7 (see table below) | No |

### Action Value Details

#### Action 2 - Navigation Keys
| Value | Key |
|-------|-----|
| 1 | Up Arrow |
| 2 | Down Arrow |
| 3 | Left Arrow |
| 4 | Right Arrow |
| 5 | Backspace |
| 6 | Tab |
| 7 | Enter |
| 8 | Page Up |
| 9 | Page Down |
| 10 | Delete |
| 11 | Print Screen |
| 12 | Escape |
| 13 | Home |
| 14 | End |

#### Action 3 - Media Keys
| Value | Key |
|-------|-----|
| 1 | Mute |
| 2 | Volume Down |
| 3 | Volume Up |
| 4 | Play/Pause |
| 5 | Stop |
| 6 | Next Track |
| 7 | Previous Track |

#### Action 5 - Modifier Keys
| Value | Key | Action |
|-------|-----|--------|
| 1 | Left Ctrl | Press |
| 2 | Left Shift | Press |
| 3 | Left Alt | Press |
| 4 | Left GUI | Press |
| 5 | Right Ctrl | Press |
| 6 | Right Shift | Press |
| 7 | Right Alt | Press |
| 8 | Right GUI | Press |
| 9 | All Keys | Release |

#### Action 9 - Key Combinations
| Value | Combination |
|-------|-------------|
| 1 | Left Ctrl + Left Shift |
| 2 | Left Alt + Left Shift |
| 3 | Left GUI + Left Shift |
| 4 | Left Ctrl + Left GUI |
| 5 | Left Alt + Left GUI |
| 6 | Left Ctrl + Left Alt |
| 7 | Left Ctrl + Left Alt + Left GUI |
| 8 | Right Ctrl + Right Shift |
| 9 | Right Alt + Right Shift |
| 10 | Right GUI + Right Shift |
| 11 | Right Ctrl + Right GUI |
| 12 | Right Alt + Right GUI |
| 13 | Right Ctrl + Right Alt |
| 14 | Right Ctrl + Right Alt + Right GUI |

#### Action 10 - Helper Functions
| Value | Function |
|-------|----------|
| 1-8 | F1-F8 with global modifiers applied |

#### Action 11 - Special Functions
| Value | Function |
|-------|----------|
| 1 | Enter configuration mode |
| 2 | Decrease display brightness |
| 3 | Increase display brightness |
| 4 | Toggle sleep mode |

#### Action 12 - Numpad
| Value | Key |
|-------|-----|
| 0-9 | Numpad 0-9 |
| 10 | Numpad / |
| 11 | Numpad * |
| 12 | Numpad - |
| 13 | Numpad + |
| 14 | Numpad Enter |
| 15 | Numpad . |

#### Action 14 - Mouse Actions
| Value | Action |
|-------|--------|
| 1 | Left Click |
| 2 | Right Click |
| 3 | Middle Click |
| 4 | Scroll Down |
| 5 | Scroll Up |
| 6 | Scroll Right |
| 7 | Scroll Left |

---

## Global Variables

### Structure Instances

The following global instances are created from the structures:

```cpp
// Configuration instances
Wificonfig wificonfig;          // WiFi settings
Config generalconfig;           // General system settings
SystemIcons systemIcons;       // System icon paths

// Menu icon instances  
Icons screen0;                  // Home screen icons
Icons screen1;                  // Menu 1 icons
Icons screen2;                  // Menu 2 icons
Icons screen3;                  // Menu 3 icons
Icons screen4;                  // Menu 4 icons
Icons screen5;                  // Menu 5 icons
Icons screen6;                  // Menu 6 icons

// Menu instances array
Menu menus[6];                  // Array of 6 menu configurations (menu0-menu5)
```

### State Arrays

```cpp
bool islatched[30];             // Latch states for all buttons (30 total)
TFT_eSPI_Button key[6];        // TFT button objects for display
```

### Utility Variables

```cpp
char templogopath[64];         // Temporary buffer for logo path construction
unsigned long previousMillis;  // Timing for sleep functionality
unsigned long Interval;        // Sleep interval in milliseconds
bool displayinginfo;           // Flag for info display state
const char* jsonfilefail;      // Error message for JSON loading failures
```

---

## File System Structure

### Image Assets
- **Location**: `/logos/` directory on SPIFFS filesystem
- **Format**: BMP files (1-bit, 2-bit, 3-bit, or 24-bit RGB)
- **Size**: Maximum 75x75 pixels
- **Naming**: Referenced by filename in JSON configurations

### System Icons
- **Location**: `/logos/sys/ico/` directory
- **Usage**: System-level UI elements (settings, home, configurator)

### Configuration Storage
- **Location**: `/config/` directory on SPIFFS filesystem
- **Format**: JSON files
- **Access**: Loaded at boot and modified via web configurator

---

## Notes

### Memory Management
- All strings use fixed-size character arrays to avoid dynamic allocation
- Maximum path lengths are enforced (32 or 64 characters)
- Button arrays use fixed sizes (6 buttons per menu)
- Menu configurations are stored in a unified array structure for efficient access

### Color Format
- Colors stored as 16-bit values (RGB565 format)
- JSON configurations use hex color strings (#RRGGBB)
- Automatic conversion between formats during loading

### Action Chaining
- Each button supports up to 3 sequential actions stored in the `actions[3]` array
- Actions execute in order (actions[0], actions[1], actions[2])
- Each action in the array contains its own action type, value, and symbol
- The new `Action` struct provides cleaner organization of action data

### Latch Behavior
- Latched buttons maintain state between presses
- Alternative logos display when button is latched
- Global latch state array tracks all button states

This documentation provides a complete reference for understanding and working with FreeTouchDeck's data structures and configuration system.

---

## Recent Updates

**Version 0.9.18a Refactoring Changes:**
- **Icons Structure**: The `Logos` struct has been renamed to `Icons` with `icons[6][32]` array replacing `logos[6][32]`
- **Action System**: Introduced new `Action` struct for individual actions, with `Actions` struct now containing an array of 3 `Action` structs
- **Menu Storage**: Menu instances are now stored in a single array `menus[6]` instead of individual `menu1-menu6` variables
- **Improved Modularity**: Action handling has been refactored for better code organization and maintainability