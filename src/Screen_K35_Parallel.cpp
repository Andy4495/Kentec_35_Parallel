// Screen_K35_Parallel.cpp
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

// Library header
#include "Screen_K35_Parallel.h"

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

#define GPIO_SLOW 0
#define GPIO_FAST 1         // GPIO_FAST is not supported by this library
#define GPIO_MODE GPIO_SLOW

#if defined(__MSP430F5529__)  // Use direct port access with F5529
#define F5529_DIRECT_IO
#endif

// Touch is not supported by this library

// Code
Screen_K35_Parallel::Screen_K35_Parallel()
{
    _pinScreenDataCommand =  9;
    _pinScreenReset       = 16;
    _pinScreenChipSelect  = 10;
    _pinScreenBackLight   = 40; // not connected -- DNP resistor R12 on schematic
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
#ifdef F5529_DIRECT_IO
    out3 = (volatile uint8_t *)(P3_BASE+OFS_P3OUT);
    out2 = (volatile uint8_t *)(P2_BASE+OFS_P2OUT);
    out1 = (volatile uint8_t *)(P1_BASE+OFS_P1OUT);
    out6 = (volatile uint8_t *)(P6_BASE+OFS_P6OUT);
    out4 = (volatile uint8_t *)(P4_BASE+OFS_P4OUT);
#endif
}

void Screen_K35_Parallel::begin()
{
  // Default values
    digitalWrite(_pinScreenDataCommand, HIGH);
    digitalWrite(_pinScreenReset, HIGH);
    digitalWrite(_pinScreenChipSelect, HIGH);
    digitalWrite(_pinScreenBackLight, HIGH);
    digitalWrite(_pinScreenWR, HIGH);
    digitalWrite(_pinScreenRD, HIGH);

    pinMode(_pinScreenDataCommand, OUTPUT);
    pinMode(_pinScreenReset, OUTPUT);
    pinMode(_pinScreenChipSelect, OUTPUT);
    pinMode(_pinScreenBackLight, OUTPUT);
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
    digitalWrite(_pinScreenReset, LOW);
    delayMicroseconds(20); // datasheet lists 15 us min reset pulse
    digitalWrite(_pinScreenReset, HIGH);

    //    //
    //    // Enter sleep mode
    //    //
    //    _writeCommand16(SSD2119_SLEEP_MODE_REG);
    //    _writeData16(0x0001);

    // Power parameters
    _writeCommand16(SSD2119_PWR_CTRL_5_REG);
    _writeData16(0x00BA);
    _writeCommand16(SSD2119_VCOM_OTP_1_REG);
    _writeData16(0x0006);

    // Start oscillator
    _writeCommand16(SSD2119_OSC_START_REG);
    _writeData16(0x0001);

    // Set pixel format and basic display orientation
    _writeCommand16(SSD2119_OUTPUT_CTRL_REG);
    _writeData16(0x30EF);
    _writeCommand16(SSD2119_LCD_DRIVE_AC_CTRL_REG);
    _writeData16(0x0600);

    // Exit sleep mode
    _writeCommand16(SSD2119_SLEEP_MODE_REG);
    _writeData16(0x0000);

    delay(31);              // 30 ms delay after exit sleep per datasheet

    // Pixel color format
    _writeCommand16(SSD2119_ENTRY_MODE_REG);
    _writeData16(ENTRY_MODE_DEFAULT);

    // Enable  display
    _writeCommand16(SSD2119_DISPLAY_CTRL_REG);
    _writeData16(0x0033);

    // Set VCIX2 voltage to 6.1V
    _writeCommand16(SSD2119_PWR_CTRL_2_REG);
    _writeData16(0x0005);

    // Gamma correction
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

    // Configure Vlcd63 and VCOMl
    _writeCommand16(SSD2119_PWR_CTRL_3_REG);
    _writeData16(0x0007);
    _writeCommand16(SSD2119_PWR_CTRL_4_REG);
    _writeData16(0x3100);

    // Display size and GRAM window
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


    _penSolid  = false;
    _fontSolid = true;
    _flagRead  = false;
    //    _flagIntensity = true;
    //    _fsmArea   = true;
    //    _touchTrim = 10;

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
    *out4 |=  BV(2);             // digitalWrite(_pinScreenDataCommand, HIGH)
    *out4 &= ~BV(1);             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (dataHigh8   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((dataHigh8) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((dataHigh8) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((dataHigh8) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((dataHigh8) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((dataHigh8) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((dataHigh8) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((dataHigh8) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (dataLow8   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((dataLow8) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((dataLow8) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((dataLow8) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((dataLow8) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((dataLow8) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((dataLow8) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((dataLow8) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  BV(1);             // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, HIGH);                                  // HIGH = data
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);

    digitalWrite(_pinScreenD0,     dataHigh8  & 0x01);
    digitalWrite(_pinScreenD1, (dataHigh8>>1) & 0x01);
    digitalWrite(_pinScreenD2, (dataHigh8>>2) & 0x01);
    digitalWrite(_pinScreenD3, (dataHigh8>>3) & 0x01);
    digitalWrite(_pinScreenD4, (dataHigh8>>4) & 0x01);
    digitalWrite(_pinScreenD5, (dataHigh8>>5) & 0x01);
    digitalWrite(_pinScreenD6, (dataHigh8>>6) & 0x01);
    digitalWrite(_pinScreenD7, (dataHigh8>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenWR, LOW);   // Get ready for next byte
    // Set output pins to data value - LSB
    digitalWrite(_pinScreenD0,     dataLow8  & 0x01);
    digitalWrite(_pinScreenD1, (dataLow8>>1) & 0x01);
    digitalWrite(_pinScreenD2, (dataLow8>>2) & 0x01);
    digitalWrite(_pinScreenD3, (dataLow8>>3) & 0x01);
    digitalWrite(_pinScreenD4, (dataLow8>>4) & 0x01);
    digitalWrite(_pinScreenD5, (dataLow8>>5) & 0x01);
    digitalWrite(_pinScreenD6, (dataLow8>>6) & 0x01);
    digitalWrite(_pinScreenD7, (dataLow8>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
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
    *out4 &= ~BV(2);             // digitalWrite(_pinScreenDataCommand, LOW)
    *out4 &= ~BV(1);             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (command16   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((command16) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((command16) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((command16) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((command16) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((command16) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((command16) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((command16) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  BV(1);             // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, LOW);                                   // LOW = command
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);

    // Set output pins to data value
    digitalWrite(_pinScreenD0,     command16  & 0x01);
    digitalWrite(_pinScreenD1, (command16>>1) & 0x01);
    digitalWrite(_pinScreenD2, (command16>>2) & 0x01);
    digitalWrite(_pinScreenD3, (command16>>3) & 0x01);
    digitalWrite(_pinScreenD4, (command16>>4) & 0x01);
    digitalWrite(_pinScreenD5, (command16>>5) & 0x01);
    digitalWrite(_pinScreenD6, (command16>>6) & 0x01);
    digitalWrite(_pinScreenD7, (command16>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif
}

void Screen_K35_Parallel::_writeCommandAndData16(uint16_t command16, uint8_t dataHigh8, uint8_t dataLow8)
{
#ifdef F5529_DIRECT_IO
    *out4 &= ~BV(2);             // digitalWrite(_pinScreenDataCommand, LOW)
    *out4 &= ~BV(1);             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (command16   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((command16) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((command16) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((command16) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((command16) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((command16) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((command16) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((command16) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  BV(1);             // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, LOW);                                   // LOW = command
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);

    // Set output pins to data value
    digitalWrite(_pinScreenD0,     command16  & 0x01);
    digitalWrite(_pinScreenD1, (command16>>1) & 0x01);
    digitalWrite(_pinScreenD2, (command16>>2) & 0x01);
    digitalWrite(_pinScreenD3, (command16>>3) & 0x01);
    digitalWrite(_pinScreenD4, (command16>>4) & 0x01);
    digitalWrite(_pinScreenD5, (command16>>5) & 0x01);
    digitalWrite(_pinScreenD6, (command16>>6) & 0x01);
    digitalWrite(_pinScreenD7, (command16>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif
#ifdef F5529_DIRECT_IO
    *out4 |=  BV(2);             // digitalWrite(_pinScreenDataCommand, HIGH)
    *out4 &= ~BV(1);             // digitalWrite(_pinScreenChipSelect, LOW);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (dataHigh8   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((dataHigh8) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((dataHigh8) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((dataHigh8) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((dataHigh8) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((dataHigh8) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((dataHigh8) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((dataHigh8) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out2 &= ~BV(7);             // digitalWrite(_pinScreenWR, LOW);

    (dataLow8   & 0x01) ? *out3 |= BV(4) : *out3 &= ~BV(4);
    ((dataLow8) & 0x02) ? *out3 |= BV(3) : *out3 &= ~BV(3);
    ((dataLow8) & 0x04) ? *out2 |= BV(0) : *out2 &= ~BV(0);
    ((dataLow8) & 0x08) ? *out1 |= BV(5) : *out1 &= ~BV(5);
    ((dataLow8) & 0x10) ? *out3 |= BV(2) : *out3 &= ~BV(2);
    ((dataLow8) & 0x20) ? *out6 |= BV(5) : *out6 &= ~BV(5);
    ((dataLow8) & 0x40) ? *out3 |= BV(1) : *out3 &= ~BV(1);
    ((dataLow8) & 0x80) ? *out3 |= BV(0) : *out3 &= ~BV(0);

    *out2 |=  BV(7);             // digitalWrite(_pinScreenWR, HIGH);
    *out4 |=  BV(1);             // digitalWrite(_pinScreenChipSelect, HIGH);
#else
    digitalWrite(_pinScreenDataCommand, HIGH);                                  // HIGH = data
    digitalWrite(_pinScreenChipSelect, LOW);
    digitalWrite(_pinScreenWR, LOW);

    digitalWrite(_pinScreenD0,     dataHigh8  & 0x01);
    digitalWrite(_pinScreenD1, (dataHigh8>>1) & 0x01);
    digitalWrite(_pinScreenD2, (dataHigh8>>2) & 0x01);
    digitalWrite(_pinScreenD3, (dataHigh8>>3) & 0x01);
    digitalWrite(_pinScreenD4, (dataHigh8>>4) & 0x01);
    digitalWrite(_pinScreenD5, (dataHigh8>>5) & 0x01);
    digitalWrite(_pinScreenD6, (dataHigh8>>6) & 0x01);
    digitalWrite(_pinScreenD7, (dataHigh8>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenWR, LOW);   // Get ready for next byte
    // Set output pins to data value - LSB
    digitalWrite(_pinScreenD0,     dataLow8  & 0x01);
    digitalWrite(_pinScreenD1, (dataLow8>>1) & 0x01);
    digitalWrite(_pinScreenD2, (dataLow8>>2) & 0x01);
    digitalWrite(_pinScreenD3, (dataLow8>>3) & 0x01);
    digitalWrite(_pinScreenD4, (dataLow8>>4) & 0x01);
    digitalWrite(_pinScreenD5, (dataLow8>>5) & 0x01);
    digitalWrite(_pinScreenD6, (dataLow8>>6) & 0x01);
    digitalWrite(_pinScreenD7, (dataLow8>>7) & 0x01);

    digitalWrite(_pinScreenWR, HIGH);  // Latch in the data
    digitalWrite(_pinScreenChipSelect, HIGH);
#endif
}

void Screen_K35_Parallel::_writeRegister(uint8_t command8, uint16_t data16)
{
//    _writeCommand16(command8);
////    _writeData16(data16);
//    _writeData88(data16 >> 8, data16);
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
//    _writeData88(colour >> 8, colour);
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

void Screen_K35_Parallel::_setBacklight(bool flag)
{
    if (flag)   _writeRegister(SSD2119_SLEEP_MODE_REG, 0);
    else        _writeRegister(SSD2119_SLEEP_MODE_REG, 1);
}

void Screen_K35_Parallel::_setIntensity(uint8_t intensity)
{
    analogWrite(_pinScreenBackLight, intensity);
}

// Touch -- Compulsory (virtual=0) method
// -- Not supported, empty definition
void Screen_K35_Parallel::_getRawTouch(uint16_t &x0, uint16_t &y0, uint16_t &z0) {

}
