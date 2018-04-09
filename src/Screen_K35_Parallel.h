// Screen_K35_Parallel.h
//
// Based on:
//     Screen_K35_SPI.cpp by Rei VILO, Jun 29, 2013 embedXcode.weebly.com
//     https://github.com/energia/msp430-lg-core/tree/master/libraries/Kentec_35_SPI
//     Per the ReadMe.txt on GitHub, original version is licensed for hobbyist and personal usage under CC BY-NC-SA 3.0
//     Contact the author above for other uses
//
// Modfied by Andy4495 for use with Kentec EB-LM4F120-L35 BoosterPack with parallel interface
// 04/08/2018
// https://gitlab.com/Andy4495/Screen_K35_Parallel
// This version continues to be licensed under CC BY-NC-SA 3.0
// See LICENSE file at above GitLab repository
//
//

#include "Energia.h"

#ifndef Screen_K35_PARALLEL_RELEASE
///
/// @brief	Library release number
///
#define Screen_K35_PARALLEL_RELEASE 104

#include "LCD_screen_font.h"

///
/// @brief      Class for 3.5" 480x320 screen
/// @details    Screen controllers
/// *   LCD: SSD2119, 8-bit 8080 parallel
/// *   touch: not supported with this library
/// @note       The class configures the GPIOs
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

    // Touch -- Not supported, empty definition in cpp file
  void _getRawTouch(uint16_t &x0, uint16_t &y0, uint16_t &z0); // compulsory

    // * Other functions specific to the screen
    // Communication, write
    void _writeRegister(uint8_t command8, uint16_t data16);
    void _writeCommand16(uint16_t command16);
    void _writeData16(uint16_t data16);

    void _setCursor(uint16_t x1, uint16_t y1);

    // Energy
    void _setIntensity(uint8_t intensity); // compulsory
    void _setBacklight(bool flag); // compulsory

    uint8_t _pinScreenDataCommand, _pinScreenReset, _pinScreenChipSelect, _pinScreenBackLight, _pinScreenWR, _pinScreenRD;
    uint8_t _pinScreenD0, _pinScreenD1, _pinScreenD2, _pinScreenD3, _pinScreenD4, _pinScreenD5, _pinScreenD6, _pinScreenD7;
};

#endif
