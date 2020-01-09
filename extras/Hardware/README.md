Specialized Interface Board
===========================
I created a custom interface board to change the mapping of the pins between the Kentec BoosterPack and the F5529 LaunchPad. This was done for two reasons:
- Directly map the data bit positions of the Kentec controller interface to the MSP port bit positions. This allows simplification of the library code to allow even faster screen refresh rates.
- Move the Kentec pins to the inner BOOSTXL header pins (J3 and J4), so that the standard BoosterPack I/O pins are available for other devices (e.g. SPI and I2C pins).

Interface Board Details
-----------------------
The interface board is specific to the MSP430F5529 LaunchPad and re-maps the Kentec BoosterPack pins as follows:

    Signal    F5529    LaunchPad
    ------    -----    ---------
    LCD_D0     P6.0       23
    LCD_D1     P6.1       24
    LCD_D2     P6.2       25
    LCD_D3     P6.3       26
    LCD_D4     P6.4       27
    LCD_D5     P3.5       30
    LCD_D6     P3.6       29
    LCD_D7     P3.7       32
    LCD_RD     P2.6       13
    LCD_WR     P1.5       38
    LCD_RS     P1.4       37       Also referred to as LCD_DS
    LCD_CS     P1.3       36

In addition, header pins are provided for the TOUCH functionality. These should be connected as follows if using TOUCH (headers for the corresponding LauncPad pins are provided next to the TOUCH headers on the PCB):

     Signal    F5529    LaunchPad
    --------   -----    ---------
    TOUCH_XP    P6.5        2       Analog pin A5
    TOUCH_YP    P6.6        6       Analog pin A6
    TOUCH_XN    P2.3       12
    TOUCH_YN    P8.1       11

Header pins are provided for the other LaunchPad pins, including:
- Vcc (3.3V and 5.0V)
- Ground
- I2C
- SPI
- Other unused I/O pins

Finally, space is provided for a 3.3V regulator (and caps), micro USB power connector, 3 general-purpose switches, and a reset switch. 


Library Support
---------------

The library has been modified to support the interface board. When using the library with the interface, the only code change needed is in the constructor to let the library know that the interface board is connected. By default, the TOUCH functionality is not connected when using the interface board:

    Screen_K35_Parallel myScreen(Screen_K35_Parallel::F5529_INTERFACE_BOARD_INSTALLED, Screen_K35_Parallel::TOUCH_DISABLED);

If, however, you connect the TOUCH sensor pins (XP->P2, YP->P6, XN->P12, YN->P11), then use the following constructor:

    Screen_K35_Parallel myScreen(Screen_K35_Parallel::F5529_INTERFACE_BOARD_INSTALLED, Screen_K35_Parallel::TOUCH_ENABLED);
