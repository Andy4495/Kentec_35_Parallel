// Screen_K35_Parallel.h
//
// Based on:
//     Screen_K35_SPI.cpp by Rei VILO, Jun 29, 2013 embedXcode.weebly.com
//     https://github.com/energia/msp430-lg-core/tree/master/libraries/Kentec_35_SPI
//     Per the ReadMe.txt on GitHub, original version is licensed for hobbyist and personal usage under CC BY-NC-SA 3.0
//     Contact the author above for other uses
//
// Modfied by Andy4495 for use with Kentec EB-LM4F120-L35 BoosterPack with parallel interface
//
// 1.0.0 - 04/08/2018 - Andy4495 - Initial release.
//
// https://gitlab.com/Andy4495/Kentec_35_Parallel
// This version continues to be licensed under CC BY-NC-SA 3.0 for hobbyist and personal usage.
// See LICENSE file at above GitLab repository
//
//


// Core library - IDE-based
#if defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific
#include "Energia.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(CORE_TEENSY) // Teensy specific
#include "WProgram.h"
#elif defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(ARDUINO) // Arduino 1.0x and 1.5x specific
#include "Arduino.h"
#endif // end IDE

#ifndef Screen_K35_PARALLEL_RELEASE
///
/// @brief	Library release number
///
#define Screen_K35_PARALLEL_RELEASE 100

#include "LCD_screen_font.h"

///
/// @brief      Class for 3.5" 480x320 screen
/// @details    Screen controllers
/// *   LCD: SSD2119, 8-bit 8080 parallel
/// *   touch: direct ADC, no controller
/// @note       The class configures the GPIOs and the SPI port.
///
class Screen_K35_Parallel : public LCD_screen_font {
public:

    ///
    /// @brief	Constructor with default pins
    ///
    /// @note	Default pins for BoosterPack on LaunchPad
    ///
    Screen_K35_Parallel();

    ///
    /// @brief      Initialise
    /// @details	Open connections
    ///
    void begin();

    ///
    /// @brief	Request information about the screen
    /// @return	string with hardware version
    ///
    String WhoAmI();

private:
	// * Virtual =0 compulsory functions
    // Orientation
    void _setOrientation(uint8_t orientation); // compulsory
    void _orientCoordinates(uint16_t &x1, uint16_t &y1); // compulsory

	// Position
    void _setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1); // compulsory
    void _closeWindow(); // compulsory
	void _setPoint(uint16_t x1, uint16_t y1, uint16_t colour); // compulsory
    void _fastFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour); // compulsory

    // Write and Read
    void _writeData88(uint8_t dataHigh8, uint8_t dataLow8); // compulsory;

	// Touch
    void _getRawTouch(uint16_t &x0, uint16_t &y0, uint16_t &z0); // compulsory

    // * Other functions specific to the screen
    // Communication, write
    void _writeRegister(uint8_t command8, uint16_t data16);
    void _writeCommand16(uint16_t command16);
    void _writeCommandAndData16(uint16_t command16, uint8_t dataHigh8, uint8_t dataLow8);
    void _writeData16(uint16_t data16);

    void _setCursor(uint16_t x1, uint16_t y1);

    // Energy
    void _setIntensity(uint8_t intensity); // compulsory
    void _setBacklight(bool flag); // compulsory

    // Touch
    void _getOneTouch(uint8_t command8, uint8_t &a, uint8_t &b);

    uint8_t _pinScreenDataCommand, _pinScreenReset, _pinScreenChipSelect, _pinScreenBackLight, _pinScreenWR, _pinScreenRD;
    uint8_t _pinScreenD0, _pinScreenD1, _pinScreenD2, _pinScreenD3, _pinScreenD4, _pinScreenD5, _pinScreenD6, _pinScreenD7;
#if defined(__MSP430F5529__)
    volatile uint8_t *out3;
    volatile uint8_t *out2;
    volatile uint8_t *out1;
    volatile uint8_t *out6;
    volatile uint8_t *out4;
#endif

};

#if defined(__MSP430F5529__)
// lookup table to convert the data value to the PORT3 bit positions
// D7 -> P3.0
// D6 -> P3.1
// D4 -> P3.2
// D1 -> P3.1
// D0 -> P3.0
// Using the lookup table speeds up the display routines significantly
// and uses less code, since the altertative requires either bit shifts
// or a conditional statement with an AND bit mask.
const uint8_t p3Lookup[] = {
  0x00, 0x10, 0x08, 0x18, 0x00, 0x10, 0x08, 0x18,
  0x00, 0x10, 0x08, 0x18, 0x00, 0x10, 0x08, 0x18,
  0x04, 0x14, 0x0C, 0x1C, 0x04, 0x14, 0x0C, 0x1C,
  0x04, 0x14, 0x0C, 0x1C, 0x04, 0x14, 0x0C, 0x1C,
  0x00, 0x10, 0x08, 0x18, 0x00, 0x10, 0x08, 0x18,
  0x00, 0x10, 0x08, 0x18, 0x00, 0x10, 0x08, 0x18,
  0x04, 0x14, 0x0C, 0x1C, 0x04, 0x14, 0x0C, 0x1C,
  0x04, 0x14, 0x0C, 0x1C, 0x04, 0x14, 0x0C, 0x1C,
  0x02, 0x12, 0x0A, 0x1A, 0x02, 0x12, 0x0A, 0x1A,
  0x02, 0x12, 0x0A, 0x1A, 0x02, 0x12, 0x0A, 0x1A,
  0x06, 0x16, 0x0E, 0x1E, 0x06, 0x16, 0x0E, 0x1E,
  0x06, 0x16, 0x0E, 0x1E, 0x06, 0x16, 0x0E, 0x1E,
  0x02, 0x12, 0x0A, 0x1A, 0x02, 0x12, 0x0A, 0x1A,
  0x02, 0x12, 0x0A, 0x1A, 0x02, 0x12, 0x0A, 0x1A,
  0x06, 0x16, 0x0E, 0x1E, 0x06, 0x16, 0x0E, 0x1E,
  0x06, 0x16, 0x0E, 0x1E, 0x06, 0x16, 0x0E, 0x1E,
  0x01, 0x11, 0x09, 0x19, 0x01, 0x11, 0x09, 0x19,
  0x01, 0x11, 0x09, 0x19, 0x01, 0x11, 0x09, 0x19,
  0x05, 0x15, 0x0D, 0x1D, 0x05, 0x15, 0x0D, 0x1D,
  0x05, 0x15, 0x0D, 0x1D, 0x05, 0x15, 0x0D, 0x1D,
  0x01, 0x11, 0x09, 0x19, 0x01, 0x11, 0x09, 0x19,
  0x01, 0x11, 0x09, 0x19, 0x01, 0x11, 0x09, 0x19,
  0x05, 0x15, 0x0D, 0x1D, 0x05, 0x15, 0x0D, 0x1D,
  0x05, 0x15, 0x0D, 0x1D, 0x05, 0x15, 0x0D, 0x1D,
  0x03, 0x13, 0x0B, 0x1B, 0x03, 0x13, 0x0B, 0x1B,
  0x03, 0x13, 0x0B, 0x1B, 0x03, 0x13, 0x0B, 0x1B,
  0x07, 0x17, 0x0F, 0x1F, 0x07, 0x17, 0x0F, 0x1F,
  0x07, 0x17, 0x0F, 0x1F, 0x07, 0x17, 0x0F, 0x1F,
  0x03, 0x13, 0x0B, 0x1B, 0x03, 0x13, 0x0B, 0x1B,
  0x03, 0x13, 0x0B, 0x1B, 0x03, 0x13, 0x0B, 0x1B,
  0x07, 0x17, 0x0F, 0x1F, 0x07, 0x17, 0x0F, 0x1F,
  0x07, 0x17, 0x0F, 0x1F, 0x07, 0x17, 0x0F, 0x1F
};
#endif

#endif
