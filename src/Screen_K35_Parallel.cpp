// Screen_K35_Parallel.cpp
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
// 1.1.0 - 05/09/2018 - Andy4495 - Add support for custom F5529 interface board.
// 2.0.0 - 06/23/2022 - Andy4495 - Comment out call to _getRawTouch()
//
// https://github.com/Andy4495/Kentec_35_Parallel
// This version continues to be licensed under CC BY-NC-SA 3.0 for hobbyist and personal usage.
// See LICENSE file at above github repository
//
//

// Library header
#include "Screen_K35_Parallel.h"
#if defined(__MSP432P401R__)
#include <msp.h>
#endif

#if defined(__LM4F120H5QR__)
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#endif

///
/// @name	SSD2119 constants
///
/// @{

#define SSD2119_DEVICE_CODE_READ_REG  0x00
#define SSD2119_OSC_START_REG         0x00
#define SSD2119_OUTPUT_CTRL_REG       0x01
#define SSD2119_LCD_DRIVE_AC_CTRL_REG 0x02
#define SSD2119_PWR_CTRL_1_REG        0x03
#define SSD2119_DISPLAY_CTRL_REG      0x07
#define SSD2119_FRAME_CYCLE_CTRL_REG  0x0B
#define SSD2119_PWR_CTRL_2_REG        0x0C
#define SSD2119_PWR_CTRL_3_REG        0x0D
#define SSD2119_PWR_CTRL_4_REG        0x0E
#define SSD2119_GATE_SCAN_START_REG   0x0F
#define SSD2119_SLEEP_MODE_REG        0x10
#define SSD2119_ENTRY_MODE_REG        0x11
#define SSD2119_GEN_IF_CTRL_REG       0x15
#define SSD2119_PWR_CTRL_5_REG        0x1E
#define SSD2119_RAM_DATA_REG          0x22
#define SSD2119_FRAME_FREQ_REG        0x25
#define SSD2119_VCOM_OTP_1_REG        0x28
#define SSD2119_VCOM_OTP_2_REG        0x29
#define SSD2119_GAMMA_CTRL_1_REG      0x30
#define SSD2119_GAMMA_CTRL_2_REG      0x31
#define SSD2119_GAMMA_CTRL_3_REG      0x32
#define SSD2119_GAMMA_CTRL_4_REG      0x33
#define SSD2119_GAMMA_CTRL_5_REG      0x34
#define SSD2119_GAMMA_CTRL_6_REG      0x35
#define SSD2119_GAMMA_CTRL_7_REG      0x36
#define SSD2119_GAMMA_CTRL_8_REG      0x37
#define SSD2119_GAMMA_CTRL_9_REG      0x3A
#define SSD2119_GAMMA_CTRL_10_REG     0x3B
#define SSD2119_V_RAM_POS_REG         0x44
#define SSD2119_H_RAM_START_REG       0x45
#define SSD2119_H_RAM_END_REG         0x46
#define SSD2119_X_RAM_ADDR_REG        0x4E
#define SSD2119_Y_RAM_ADDR_REG        0x4F

#define ENTRY_MODE_DEFAULT 0x6830
#define MAKE_ENTRY_MODE(x) ((ENTRY_MODE_DEFAULT & 0xFF00) | (x))

#define K35_WIDTH       320 // Vertical
#define K35_HEIGHT      240 // Horizontal

/// @}

#define GPIO_SLOW 0
#define GPIO_FAST 1
#define GPIO_MODE GPIO_SLOW

#if ((GPIO_MODE == GPIO_FAST) && defined(__LM4F120H5QR__))
//
// LCD control line GPIO definitions.
//
#define LCD_CS_PERIPH           SYSCTL_PERIPH_GPIOA
#define LCD_CS_BASE             GPIO_PORTA_BASE
#define LCD_CS_PIN              GPIO_PIN_4
#define LCD_DC_PERIPH           SYSCTL_PERIPH_GPIOA
#define LCD_DC_BASE             GPIO_PORTA_BASE
#define LCD_DC_PIN              GPIO_PIN_5

#endif

#if defined(__MSP430F5529__)  // Use direct port access with F5529
#define F5529_DIRECT_IO
#endif


///
/// @name   Touch constants
///
/// @{

#define TOUCH_TRIM  0x10 ///< Touch threshold


/// @}


///
/// @brief	Solution for touch on MSP432 ADC
/// MSP432 14-bit but 10-bit available by default
/// * Solution 1: keep 10-bit
/// * Solution 2: set to 12-bit with
///
#define MSP432_SOLUTION 1

#if defined(__MSP432P401R__)
#   if (MSP432_SOLUTION == 1)
// Solution 1: MSP432 14-bit but 10-bit available by default
#       define ANALOG_RESOLUTION 1023
#   else
// Solution 2: MSP432 14-bit set to 12-bit
#       define ANALOG_RESOLUTION 4095
#   endif
#else
// LM4F TM4C F5529 12-bit
#   define ANALOG_RESOLUTION 4095
#endif


// Code
Screen_K35_Parallel::Screen_K35_Parallel(uint8_t interface_board, uint8_t touch_feature)
{
    _pinScreenDataCommand =  9;
//    _pinScreenReset       = 16;    // Hardwired to LaunchPad reset
    _pinScreenChipSelect  = 10;
//    _pinScreenBackLight   = 40; // not connected -- DNP resistor R12 on schematic
    _pinScreenWR          =  8;
    _pinScreenRD          = 13;
    _pinScreenD0          =  3;
    _pinScreenD1          =  4;
    _pinScreenD2          = 19;
    _pinScreenD3          = 38; // Note that this pin requires BOOST-XL support
    _pinScreenD4          =  7;
    _pinScreenD5          =  2;
    _pinScreenD6          = 14;
    _pinScreenD7          = 15;
    TOUCH_XP              =  5;  // Analog pin
    TOUCH_YP              =  6;  // Analog pin
    TOUCH_XN              = 12;
    TOUCH_YN              = 11;
    _touch_feature        = touch_feature;
#ifdef F5529_DIRECT_IO
    out3 = (volatile uint8_t *)(P3_BASE+OFS_P3OUT);
    out2 = (volatile uint8_t *)(P2_BASE+OFS_P2OUT);
    out1 = (volatile uint8_t *)(P1_BASE+OFS_P1OUT);
    out6 = (volatile uint8_t *)(P6_BASE+OFS_P6OUT);
    out4 = (volatile uint8_t *)(P4_BASE+OFS_P4OUT);
    interface_board_installed = interface_board;
    if (interface_board_installed == F5529_INTERFACE_BOARD_INSTALLED) {
      _pinScreenDataCommand = 37;
  //    _pinScreenReset       = 16;    // Hardwired to LaunchPad reset
      _pinScreenChipSelect  = 36;
//      _pinScreenBackLight   = 40; // not connected -- DNP resistor R12 on schematic
      _pinScreenWR          = 38;
      _pinScreenRD          = 13;   // Recommended pin location for interface board
      _pinScreenD0          = 23;
      _pinScreenD1          = 24;
      _pinScreenD2          = 25;
      _pinScreenD3          = 26; // Note that this pin requires BOOST-XL support
      _pinScreenD4          = 27;
      _pinScreenD5          = 30;
      _pinScreenD6          = 29;
      _pinScreenD7          = 32;
      TOUCH_XP              =  2;  // Analog pin
      TOUCH_YP              =  6;  // Analog pin
      TOUCH_XN              = 12;
      TOUCH_YN              = 11;
    }
#endif
}

void Screen_K35_Parallel::begin()
{
  // Default values
    digitalWrite(_pinScreenDataCommand, HIGH);
//    digitalWrite(_pinScreenReset, HIGH);       // Hardwired to LaunchPad reset
    digitalWrite(_pinScreenChipSelect, HIGH);
//    digitalWrite(_pinScreenBackLight, HIGH);   // Resistor R12 is DNP on schematic
    digitalWrite(_pinScreenWR, HIGH);
    digitalWrite(_pinScreenRD, HIGH);

    pinMode(_pinScreenDataCommand, OUTPUT);
//    pinMode(_pinScreenReset, OUTPUT);         // Hardwired to LaunchPad reset
    pinMode(_pinScreenChipSelect, OUTPUT);
//    pinMode(_pinScreenBackLight, OUTPUT);     // Resistor R12 is DNP on schematic
    pinMode(_pinScreenWR, OUTPUT);
    pinMode(_pinScreenRD, OUTPUT);

    pinMode(_pinScreenD0, OUTPUT);
    pinMode(_pinScreenD1, OUTPUT);
    pinMode(_pinScreenD2, OUTPUT);
    pinMode(_pinScreenD3, OUTPUT);
    pinMode(_pinScreenD4, OUTPUT);
    pinMode(_pinScreenD5, OUTPUT);
    pinMode(_pinScreenD6, OUTPUT);
    pinMode(_pinScreenD7, OUTPUT);

    // RESET cycle
    // On Parallel version, the display reset line is hard-wired to LaunchPad reset
//    digitalWrite(_pinScreenReset, LOW);
//    delayMicroseconds(20); // datasheet lists 15 us min reset pulse
//    digitalWrite(_pinScreenReset, HIGH);

    //    //
    //    // Enter sleep mode
    //    //
    //    _writeCommand16(SSD2119_SLEEP_MODE_REG);
    //    _writeData16(0x0001);

    //
    // Power parameters
    //
    _writeCommand16(SSD2119_PWR_CTRL_5_REG);
    _writeData16(0x00BA);
    _writeCommand16(SSD2119_VCOM_OTP_1_REG);
    _writeData16(0x0006);

    //
    // Start oscillator
    //
    _writeCommand16(SSD2119_OSC_START_REG);
    _writeData16(0x0001);

    //
    // Set pixel format and basic display orientation
    //
    _writeCommand16(SSD2119_OUTPUT_CTRL_REG);
    _writeData16(0x30EF);
    _writeCommand16(SSD2119_LCD_DRIVE_AC_CTRL_REG);
    _writeData16(0x0600);

    //
    // Exit sleep mode
    //
    _writeCommand16(SSD2119_SLEEP_MODE_REG);
    _writeData16(0x0000);

    delay(31);              // Datasheet specifies min 30 ms delay after exit sleep

    // Pixel color format
    //
    _writeCommand16(SSD2119_ENTRY_MODE_REG);
    _writeData16(ENTRY_MODE_DEFAULT);

    //
    // Enable  display
    //
    _writeCommand16(SSD2119_DISPLAY_CTRL_REG);
    _writeData16(0x0033);

    //
    // Set VCIX2 voltage to 6.1V
    //
    _writeCommand16(SSD2119_PWR_CTRL_2_REG);
    _writeData16(0x0005);

    //
    // Gamma correction
    //
    _writeCommand16(SSD2119_GAMMA_CTRL_1_REG);
    _writeData16(0x0000);
    _writeCommand16(SSD2119_GAMMA_CTRL_2_REG);
    _writeData16(0x0400);
    _writeCommand16(SSD2119_GAMMA_CTRL_3_REG);
    _writeData16(0x0106);
    _writeCommand16(SSD2119_GAMMA_CTRL_4_REG);
    _writeData16(0x0700);
    _writeCommand16(SSD2119_GAMMA_CTRL_5_REG);
    _writeData16(0x0002);
    _writeCommand16(SSD2119_GAMMA_CTRL_6_REG);
    _writeData16(0x0702);
    _writeCommand16(SSD2119_GAMMA_CTRL_7_REG);
    _writeData16(0x0707);
    _writeCommand16(SSD2119_GAMMA_CTRL_8_REG);
    _writeData16(0x0203);
    _writeCommand16(SSD2119_GAMMA_CTRL_9_REG);
    _writeData16(0x1400);
    _writeCommand16(SSD2119_GAMMA_CTRL_10_REG);
    _writeData16(0x0F03);

    //
    // Configure Vlcd63 and VCOMl
    //
    _writeCommand16(SSD2119_PWR_CTRL_3_REG);
    _writeData16(0x0007);
    _writeCommand16(SSD2119_PWR_CTRL_4_REG);
    _writeData16(0x3100);

    //
    // Display size and GRAM window
    //
    _writeCommand16(SSD2119_V_RAM_POS_REG);
    _writeData16((K35_HEIGHT-1) << 8);
    _writeCommand16(SSD2119_H_RAM_START_REG);
    _writeData16(0x0000);
    _writeCommand16(SSD2119_H_RAM_END_REG);
    _writeData16(K35_WIDTH-1);
    _writeCommand16(SSD2119_X_RAM_ADDR_REG);
    _writeData16(0x00);
    _writeCommand16(SSD2119_Y_RAM_ADDR_REG);
    _writeData16(0x00);

    // Standard
    setOrientation(1);

    _screenWidth  = K35_HEIGHT;
    _screenHeigth = K35_WIDTH;
    //    _screenDiagonal = 35;
    setFontSize(0);

    // Touch
    // The call to _getRawTouch() caused the code to hang when compied with MSP432
    // The SPI version of the board also sometimes hangs on this call
    // Therefore, comment out the code for now
/*
    if (_touch_feature == TOUCH_ENABLED) {
      uint16_t x0, y0, z0;
      _getRawTouch(x0, y0, z0);
    }
*/

    // Touch calibration
    _touchTrim = TOUCH_TRIM;

#if (ANALOG_RESOLUTION == 4095)
#   warning ANALOG_RESOLUTION == 4095
    _touchXmin = 3077;
    _touchXmax = 881;
    _touchYmin = 3354;
    _touchYmax = 639;
#elif (ANALOG_RESOLUTION == 1023)
#   warning ANALOG_RESOLUTION == 1023
    _touchXmin = 837;
    _touchXmax = 160;
    _touchYmin = 898;
    _touchYmax = 114;
#else
#error Wrong
#endif

    _penSolid  = false;
    _fontSolid = true;
    _flagRead  = false;
    //    _flagIntensity = true;
    //    _fsmArea   = true;
    //    _touchTrim = 10;

    // Solution 2: MSP432 14-bit set to 12-bit
#if defined(__MSP432P401R__) && (MSP432_SOLUTION == 2)
    analogReadResolution(12);
#endif

#if (ANALOG_RESOLUTION == 4095)
    _touchTrim *= 4;
#endif

    clear();
}

String Screen_K35_Parallel::WhoAmI()
{
    return "Kentec 3.5\" Parallel screen";
}

void Screen_K35_Parallel::_setOrientation(uint8_t orientation)
{
    // default = 0x6830 = 0x68 <<8 + 0b00110000
    switch (_orientation) {
        case 0:
            //                                                  hvO
            _writeRegister(SSD2119_ENTRY_MODE_REG, 0x6800 + 0b00101000);        // ok
            break;
        case 1:
            //                                                  hvO
            _writeRegister(SSD2119_ENTRY_MODE_REG, 0x6800 + 0b00000000);        // ok
            break;
        case 2:
            //                                                  hvO
            _writeRegister(SSD2119_ENTRY_MODE_REG, 0x6800 + 0b00011000);        // ok
            break;
        case 3:
            //                                                  hvO
            _writeRegister(SSD2119_ENTRY_MODE_REG, 0x6800 + 0b00110000);        // ok
            break;
    }
}

// Utilities
void Screen_K35_Parallel::_writeData16(uint16_t data16)
{
    _writeData88(data16 >> 8, data16);
}

void Screen_K35_Parallel::_writeData88(uint8_t dataHigh8, uint8_t dataLow8)
{
#ifdef F5529_DIRECT_IO
  if (interface_board_installed == F5529_INTERFACE_BOARD_INSTALLED) {
    // digitalWrite(_pinScreenDataCommand, HIGH)
    // digitalWrite(_pinScreenChipSelect, LOW);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 |=  0x10;
    *out1 &= ~0x28;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= dataHigh8 & 0x1f;
    *out3 |= dataHigh8 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 |=  0x20;
    *out1 &= ~0x20;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= dataLow8 & 0x1f;
    *out3 |= dataLow8 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenChipSelect, HIGH);
    *out1 |= 0x28;
  }
  else {
    *out4 |=  0x04;             // digitalWrite(_pinScreenDataCommand, HIGH)
    *out4 &= ~0x02;             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the IO Port bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Only need logic to set the IO bits here, since they were all cleared above
    if ((dataHigh8) & 0x04)  *out2 |= 0x01 ;
    if ((dataHigh8) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (dataHigh8) & 0x20;   // No need for conditional, since bit positions are the same: bit 5
    *out3 |= p3Lookup[dataHigh8];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the IO Port bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Only need logic to set the IO bits here, since they were all cleared above
    if ((dataLow8) & 0x04)  *out2 |= 0x01 ;
    if ((dataLow8) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (dataLow8) & 0x20;   // No need for conditional, since bit positions are the same: bit 5
    *out3 |= p3Lookup[dataLow8];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  0x02;             // digitalWrite(_pinScreenChipSelect, HIGH);
  }
#else

#if defined(__MSP432P401R__)
    P6OUT |=  BIT(5);      // digitalWrite(_pinScreenDataCommand, HIGH);
    P6OUT &= ~BIT(4);      // digitalWrite(_pinScreenChipSelect, LOW);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
    if (dataHigh8 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (dataHigh8 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (dataHigh8 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (dataHigh8 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (dataHigh8 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (dataHigh8 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (dataHigh8 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (dataHigh8 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
#else
    digitalWrite(_pinScreenDataCommand, HIGH);                                  // HIGH = data
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);
    digitalWrite(_pinScreenD0, dataHigh8 & 0x01);
    digitalWrite(_pinScreenD1, dataHigh8 & 0x02);
    digitalWrite(_pinScreenD2, dataHigh8 & 0x04);
    digitalWrite(_pinScreenD3, dataHigh8 & 0x08);
    digitalWrite(_pinScreenD4, dataHigh8 & 0x10);
    digitalWrite(_pinScreenD5, dataHigh8 & 0x20);
    digitalWrite(_pinScreenD6, dataHigh8 & 0x40);
    digitalWrite(_pinScreenD7, dataHigh8 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenWR, LOW);   // Get ready for next byte
#endif

    // Set output pins to data value - LSB
#if defined(__MSP432P401R__)
    if (dataLow8 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (dataLow8 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (dataLow8 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (dataLow8 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (dataLow8 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (dataLow8 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (dataLow8 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (dataLow8 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P6OUT |=  BIT(4);      // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenD0, dataLow8 & 0x01);
    digitalWrite(_pinScreenD1, dataLow8 & 0x02);
    digitalWrite(_pinScreenD2, dataLow8 & 0x04);
    digitalWrite(_pinScreenD3, dataLow8 & 0x08);
    digitalWrite(_pinScreenD4, dataLow8 & 0x10);
    digitalWrite(_pinScreenD5, dataLow8 & 0x20);
    digitalWrite(_pinScreenD6, dataLow8 & 0x40);
    digitalWrite(_pinScreenD7, dataLow8 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif

#endif
}

//*****************************************************************************
//
// Writes a command to the SSD2119.  This function implements the basic GPIO
// interface to the LCD display.
//
//*****************************************************************************
void Screen_K35_Parallel::_writeCommand16(uint16_t command16)
{
#ifdef F5529_DIRECT_IO
  if (interface_board_installed == F5529_INTERFACE_BOARD_INSTALLED) {
    // digitalWrite(_pinScreenDataCommand, LOW)
    // digitalWrite(_pinScreenChipSelect, LOW);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 &= ~0x38;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= command16 & 0x1f;
    *out3 |= command16 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenChipSelect, HIGH);
    *out1 |= 0x28;
  }
  else {
    *out4 &= ~0x04;             // digitalWrite(_pinScreenDataCommand, LOW)
    *out4 &= ~0x02;             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the IO Port bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Only need logic to set the IO bits here, since they were all cleared above
    if ((command16) & 0x04)  *out2 |= 0x01 ;
    if ((command16) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (command16) & 0x20;   // No need for conditional, since bit positions are the same: bit 5
    *out3 |= p3Lookup[command16];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  0x02;             // digitalWrite(_pinScreenChipSelect, HIGH);
  }
#else

    // Set output pins to data value
#if defined(__MSP432P401R__)
    P6OUT &= ~BIT(5);      // digitalWrite(_pinScreenDataCommand, LOW);
    P6OUT &= ~BIT(4);      // digitalWrite(_pinScreenChipSelect, LOW);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
    if (command16 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (command16 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (command16 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (command16 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (command16 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (command16 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (command16 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (command16 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P6OUT |=  BIT(4);      // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, LOW);                                   // LOW = command
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);
    digitalWrite(_pinScreenD0, command16 & 0x01);
    digitalWrite(_pinScreenD1, command16 & 0x02);
    digitalWrite(_pinScreenD2, command16 & 0x04);
    digitalWrite(_pinScreenD3, command16 & 0x08);
    digitalWrite(_pinScreenD4, command16 & 0x10);
    digitalWrite(_pinScreenD5, command16 & 0x20);
    digitalWrite(_pinScreenD6, command16 & 0x40);
    digitalWrite(_pinScreenD7, command16 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif

#endif
}

//*****************************************************************************
//
// Writes a command and data to the SSD2119 in a single function call.
// This avoids overhead of separate function calls for a slight speed
// improvement.
//
//*****************************************************************************
void Screen_K35_Parallel::_writeCommandAndData16(uint16_t command16, uint8_t dataHigh8, uint8_t dataLow8)
{
#ifdef F5529_DIRECT_IO
  if (interface_board_installed == F5529_INTERFACE_BOARD_INSTALLED) {
    // digitalWrite(_pinScreenDataCommand, LOW)
    // digitalWrite(_pinScreenChipSelect, LOW);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 &= ~0x38;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= command16 & 0x1f;
    *out3 |= command16 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenChipSelect, HIGH);
    *out1 |= 0x28;
  }
  else {
    *out4 &= ~0x04;             // digitalWrite(_pinScreenDataCommand, LOW)
    *out4 &= ~0x02;             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the IO Port bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Only need logic to set the IO bits here, since they were all cleared above
    if ((command16) & 0x04)  *out2 |= 0x01 ;
    if ((command16) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (command16) & 0x20;   // No need for conditional, since bit positions are the same: bit 5
    *out3 |= p3Lookup[command16];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  0x02;             // digitalWrite(_pinScreenChipSelect, HIGH);
  }
#else

    // Set output pins to data value
#if defined(__MSP432P401R__)
    P6OUT &= ~BIT(5);      // digitalWrite(_pinScreenDataCommand, LOW);
    P6OUT &= ~BIT(4);      // digitalWrite(_pinScreenChipSelect, LOW);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
    if (command16 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (command16 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (command16 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (command16 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (command16 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (command16 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (command16 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (command16 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P6OUT |=  BIT(4);      // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, LOW);                                   // LOW = command
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);
    digitalWrite(_pinScreenD0, command16 & 0x01);
    digitalWrite(_pinScreenD1, command16 & 0x02);
    digitalWrite(_pinScreenD2, command16 & 0x04);
    digitalWrite(_pinScreenD3, command16 & 0x08);
    digitalWrite(_pinScreenD4, command16 & 0x10);
    digitalWrite(_pinScreenD5, command16 & 0x20);
    digitalWrite(_pinScreenD6, command16 & 0x40);
    digitalWrite(_pinScreenD7, command16 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif

#endif
#ifdef F5529_DIRECT_IO
  if (interface_board_installed == F5529_INTERFACE_BOARD_INSTALLED) {
    // digitalWrite(_pinScreenDataCommand, HIGH)
    // digitalWrite(_pinScreenChipSelect, LOW);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 |=  0x10;
    *out1 &= ~0x28;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= dataHigh8 & 0x1f;
    *out3 |= dataHigh8 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenWR, LOW);
    *out1 |=  0x20;
    *out1 &= ~0x20;

    // Clear the IO Port bits first, then only set bits if needed
    *out6 &= ~0x1f; // D4 - D0
    *out3 &= ~0xe0; // D7 - D5

    // Only need logic to set the IO bits here, since they were all cleared above
    // When using the custom interface board, the data bit positions match the
    // I/O port bit positiongs, so no comparison needed; just OR the bits.
    *out6 |= dataLow8 & 0x1f;
    *out3 |= dataLow8 & 0xe0;

    // digitalWrite(_pinScreenWR, HIGH);
    // digitalWrite(_pinScreenChipSelect, HIGH);
    *out1 |= 0x28;
  }
  else {
    *out4 |=  0x04;             // digitalWrite(_pinScreenDataCommand, HIGH)
    *out4 &= ~0x02;             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the IO Port bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Only need logic to set the IO bits here, since they were all cleared above
    if ((dataHigh8) & 0x04)  *out2 |= 0x01 ;
    if ((dataHigh8) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (dataHigh8) & 0x20;   // No need for conditional, since bit positions are the same: bit 5
    *out3 |= p3Lookup[dataHigh8];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out2 &= ~0x80;             // digitalWrite(_pinScreenWR, LOW);

    // Clear the bits first, then only set bits if needed
    *out3 &= ~0x1f;
    *out1 &= ~0x20;
    *out2 &= ~0x01;
    *out6 &= ~0x20;

    // Don't need clear bit logic, since taken care of above.
    if ((dataLow8) & 0x04)  *out2 |= 0x01 ;
    if ((dataLow8) & 0x08)  *out1 |= 0x20 ;
    *out6 |= (dataLow8) & 0x20;   // No need for conditional, since bit posiitons are the same: bit 5
    *out3 |= p3Lookup[dataLow8];

    *out2 |=  0x80;             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  0x02;             // digitalWrite(_pinScreenChipSelect, HIGH);
  }
#else

#if defined(__MSP432P401R__)
    P6OUT |=  BIT(5);      // digitalWrite(_pinScreenDataCommand, HIGH);
    P6OUT &= ~BIT(4);      // digitalWrite(_pinScreenChipSelect, LOW);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
    if (dataHigh8 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (dataHigh8 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (dataHigh8 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (dataHigh8 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (dataHigh8 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (dataHigh8 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (dataHigh8 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (dataHigh8 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P4OUT &= ~BIT(6);      // digitalWrite(_pinScreenWR, LOW);
#else
    digitalWrite(_pinScreenDataCommand, HIGH);                                  // HIGH = data
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);
    digitalWrite(_pinScreenD0, dataHigh8 & 0x01);
    digitalWrite(_pinScreenD1, dataHigh8 & 0x02);
    digitalWrite(_pinScreenD2, dataHigh8 & 0x04);
    digitalWrite(_pinScreenD3, dataHigh8 & 0x08);
    digitalWrite(_pinScreenD4, dataHigh8 & 0x10);
    digitalWrite(_pinScreenD5, dataHigh8 & 0x20);
    digitalWrite(_pinScreenD6, dataHigh8 & 0x40);
    digitalWrite(_pinScreenD7, dataHigh8 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenWR, LOW);   // Get ready for next byte
#endif

    // Set output pins to data value - LSB
#if defined(__MSP432P401R__)
    if (dataLow8 & BIT(0)) P3OUT |= BIT(2); else P3OUT &= ~BIT(2);
    if (dataLow8 & BIT(1)) P3OUT |= BIT(3); else P3OUT &= ~BIT(3);
    if (dataLow8 & BIT(2)) P2OUT |= BIT(5); else P2OUT &= ~BIT(5);
    if (dataLow8 & BIT(3)) P2OUT |= BIT(4); else P2OUT &= ~BIT(4);
    if (dataLow8 & BIT(4)) P1OUT |= BIT(5); else P1OUT &= ~BIT(5);
    if (dataLow8 & BIT(5)) P6OUT |= BIT(0); else P6OUT &= ~BIT(0);
    if (dataLow8 & BIT(6)) P1OUT |= BIT(7); else P1OUT &= ~BIT(7);
    if (dataLow8 & BIT(7)) P1OUT |= BIT(6); else P1OUT &= ~BIT(6);
    P4OUT |=  BIT(6);      // digitalWrite(_pinScreenWR, HIGH);
    P6OUT |=  BIT(4);      // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenD0, dataLow8 & 0x01);
    digitalWrite(_pinScreenD1, dataLow8 & 0x02);
    digitalWrite(_pinScreenD2, dataLow8 & 0x04);
    digitalWrite(_pinScreenD3, dataLow8 & 0x08);
    digitalWrite(_pinScreenD4, dataLow8 & 0x10);
    digitalWrite(_pinScreenD5, dataLow8 & 0x20);
    digitalWrite(_pinScreenD6, dataLow8 & 0x40);
    digitalWrite(_pinScreenD7, dataLow8 & 0x80);
    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif

#endif
}

void Screen_K35_Parallel::_writeRegister(uint8_t command8, uint16_t data16)
{
//    _writeCommand16(command8);
//    _writeData16(data16);
_writeCommandAndData16(command8, data16 >> 8, data16);
}

void Screen_K35_Parallel::_orientCoordinates(uint16_t &x1, uint16_t &y1)
{
    switch (_orientation) {
        case 0:                                                                 // ok
            y1 = screenSizeY()-1 - y1;
            _swap(x1, y1);
            break;
        case 1:                                                                 // ok
            x1 = screenSizeX() - x1 -1;
            y1 = screenSizeY() - y1 -1;
            break;
        case 2:                                                                 // ok
            x1 = screenSizeX() - x1 -1;
            _swap(x1, y1);
            break;
        case 3:                                                                 // ok
            break;
    }
}

void Screen_K35_Parallel::_setPoint(uint16_t x1, uint16_t y1, uint16_t colour)           // compulsory
{
//    _setCursor(x1, y1);
    _orientCoordinates(x1, y1);
//    _writeRegister(SSD2119_X_RAM_ADDR_REG, x1);
    _writeCommandAndData16(SSD2119_X_RAM_ADDR_REG, x1 >> 8, x1);
//    _writeRegister(SSD2119_Y_RAM_ADDR_REG, y1);
    _writeCommandAndData16(SSD2119_Y_RAM_ADDR_REG, y1 >> 8, y1);
//    _writeCommand16(SSD2119_RAM_DATA_REG);
//    _writeData16(colour);
    _writeCommandAndData16(SSD2119_RAM_DATA_REG, colour >> 8, colour);
}

void Screen_K35_Parallel::_setCursor(uint16_t x1, uint16_t y1)
{
    _orientCoordinates(x1, y1);
//    _writeRegister(SSD2119_X_RAM_ADDR_REG, x1);
    _writeCommandAndData16(SSD2119_X_RAM_ADDR_REG, x1 >> 8, x1);
//    _writeRegister(SSD2119_Y_RAM_ADDR_REG, y1);
    _writeCommandAndData16(SSD2119_Y_RAM_ADDR_REG, y1 >> 8, y1);

    _writeCommand16(SSD2119_RAM_DATA_REG);
}

void Screen_K35_Parallel::_setWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    _orientCoordinates(x1, y1);
    _orientCoordinates(x2, y2);

//    _writeRegister(SSD2119_X_RAM_ADDR_REG, x1);
    _writeCommandAndData16(SSD2119_X_RAM_ADDR_REG, x1 >> 8, x1);
//    _writeRegister(SSD2119_Y_RAM_ADDR_REG, y1);
    _writeCommandAndData16(SSD2119_Y_RAM_ADDR_REG, y1 >> 8, y1);

    if (x1 > x2) _swap(x1, x2);
    if (y1 > y2) _swap(y1, y2);

//    _writeCommand16(SSD2119_V_RAM_POS_REG);
//    _writeData88(y2, y1);
    _writeCommandAndData16(SSD2119_V_RAM_POS_REG, y2, y1);
//    _writeRegister(SSD2119_H_RAM_START_REG, x1);
    _writeCommandAndData16(SSD2119_H_RAM_START_REG, x1 >> 8, x1);
//    _writeRegister(SSD2119_H_RAM_END_REG, x2);
    _writeCommandAndData16(SSD2119_H_RAM_END_REG, x2 >> 8, x2);

    _writeCommand16(SSD2119_RAM_DATA_REG);
}

void Screen_K35_Parallel::_closeWindow()
{
    _setWindow(0, 0, screenSizeX()-1, screenSizeY()-1);
}

inline uint16_t absDiff(uint16_t a, uint16_t b) { return (a > b) ? a-b : b-a; }

void Screen_K35_Parallel::_fastFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour)
{
    if (x1 > x2) _swap(x1, x2);
    if (y1 > y2) _swap(y1, y2);

    uint8_t highColour = highByte(colour);
    uint8_t lowColour  = lowByte(colour);

    _setWindow(x1, y1, x2, y2);
    for (uint32_t t=(uint32_t)(y2-y1+1)*(x2-x1+1); t>0; t--) {
        _writeData88(highColour, lowColour);
    }
}

// Touch
void Screen_K35_Parallel::_getRawTouch(uint16_t &x0, uint16_t &y0, uint16_t &z0)
{
    // --- 2015-08-04 _getRawTouch revised entirely
    // Tested against MSP432, F5529 and LM4F/TM4C
    // However, the calibrateTouch() may throw wrong results
    //
    int16_t a, b, c, d;
    bool flag;

#if defined(__MSP432P401R__)
    pinMode(TOUCH_YP, OUTPUT);
    pinMode(TOUCH_YN, OUTPUT);
    pinMode(TOUCH_XP, OUTPUT);
    pinMode(TOUCH_XN, OUTPUT);
    digitalWrite(TOUCH_YP, LOW);
    digitalWrite(TOUCH_YN, LOW);
    digitalWrite(TOUCH_XP, LOW);
    digitalWrite(TOUCH_XN, LOW);

    delayMicroseconds(1000); // delay(1);
#endif

    do {
        flag = false;

        // Read x
        // xp = +Vref
        // xn = ground
        // yp = measure
        // yn = open

        pinMode(TOUCH_YP, INPUT);
        pinMode(TOUCH_YN, INPUT);

        pinMode(TOUCH_XP, OUTPUT);
        pinMode(TOUCH_XN, OUTPUT);
        digitalWrite(TOUCH_XP, HIGH);
        digitalWrite(TOUCH_XN, LOW);

        delayMicroseconds(1000); // delay(1);
        a = analogRead(TOUCH_YP);
        delayMicroseconds(1000); // delay(1);
        b = analogRead(TOUCH_YP);

        flag |= (absDiff(a, b) > 8);
        x0  = ANALOG_RESOLUTION - a;

        // Read y
        // xp = measure
        // xn = open
        // yp = +Vref
        // yn = ground

        pinMode(TOUCH_XP, INPUT);
        pinMode(TOUCH_XN, INPUT);

        pinMode(TOUCH_YP, OUTPUT);
        pinMode(TOUCH_YN, OUTPUT);
        digitalWrite(TOUCH_YP, HIGH);
        digitalWrite(TOUCH_YN, LOW);

        delayMicroseconds(1000); // delay(1);
        c = analogRead(TOUCH_XP);
        delayMicroseconds(1000); // delay(1);
        d = analogRead(TOUCH_XP);

        flag |= (absDiff(c, d) > 8);
        y0  = ANALOG_RESOLUTION - c;

        // Read z
        // xp = ground
        // xn = measure
        // yp = measure
        // yn = +Vref
        pinMode(TOUCH_XP, OUTPUT);
        pinMode(TOUCH_YN, OUTPUT);
        digitalWrite(TOUCH_XP, LOW);
        digitalWrite(TOUCH_YN, HIGH);

        pinMode(TOUCH_XN, INPUT);
        pinMode(TOUCH_YP, INPUT);

        delayMicroseconds(1000); // delay(1);
        a = analogRead(TOUCH_XN);
        delayMicroseconds(1000); // delay(1);
        c = analogRead(TOUCH_YP);
        delayMicroseconds(1000); // delay(1);
        b = analogRead(TOUCH_XN);
        delayMicroseconds(1000); // delay(1);
        d = analogRead(TOUCH_YP);

        flag |= (absDiff(a, b) > 8);
        flag |= (absDiff(c, d) > 8);
        // z0 = (ANALOG_RESOLUTION - (c-a));
        // Because a = TOUCH_XN non analog, remove a
        z0 = ANALOG_RESOLUTION - c;
    }
    while (flag);
}

void Screen_K35_Parallel::_setBacklight(bool flag)
{
    if (flag)   _writeRegister(SSD2119_SLEEP_MODE_REG, 0);
    else        _writeRegister(SSD2119_SLEEP_MODE_REG, 1);
}

void Screen_K35_Parallel::_setIntensity(uint8_t intensity)
{
    analogWrite(_pinScreenBackLight, intensity);
}


//#endif // end __LM4F120H5QR__
