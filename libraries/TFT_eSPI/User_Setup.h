/* 
  Rui Santos & Sara Santos - Random Nerd Tutorials

  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
  *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
  *** YOU MUST USE THIS User_Setup.h FILE, CHECK FOR THE LATEST VERSION IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
  https://RandomNerdTutorials.com/cyd/
  https://RandomNerdTutorials.com/esp32-tft/
  FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd/ or https://RandomNerdTutorials.com/esp32-tft/
*/

//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pins used and SPI control method etc
//
//   See the User_Setup_Select.h file if you wish to be able to define multiple
//   setups and then easily select which setup file is used by the compiler.
//
//   If this file is edited correctly then all the library example sketches should
//   run without the need to make any more changes for a particular hardware setup!
//   Note that some sketches are designed for a particular TFT pixel width/height

// User defined information reported by "Read_User_Setup" test & diagnostics example
#define USER_SETUP_INFO "User_Setup"

// Define to disable all #warnings in library (can be put in User_Setup_Select.h)
//#define DISABLE_ALL_LIBRARY_WARNINGS

// ##################################################################################
//
// Section 1. Call up the right driver file and any options for it
//
// ##################################################################################

 #define ILI9341_DRIVER
//#define ILI9486_DRIVER

#define TFT_HEIGHT 320 // ST7789 240 x 320

#define TFT_INVERSION_OFF

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

#define TOUCH_CS 21     // Chip select pin (T_CS) of touch screen

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000