# Kentec EB-LM4F120-L35 BoosterPack Library

[![Arduino Compile Sketches](https://github.com/Andy4495/Kentec_35_Parallel/actions/workflows/arduino-compile-sketches.yml/badge.svg)](https://github.com/Andy4495/Kentec_35_Parallel/actions/workflows/arduino-compile-sketches.yml)
[![Check Markdown Links](https://github.com/Andy4495/Kentec_35_Parallel/actions/workflows/check-links.yml/badge.svg)](https://github.com/Andy4495/Kentec_35_Parallel/actions/workflows/check-links.yml)

This library supports the Kentec [EB-LM4F120-L35][4] 3.5" QVGA TFT LCD BoosterPack with Parallel I/O.

The EB-LM4F120-L35 uses the same display and controller chip as the SPI-based [BOOSTXL-K350QVG-S1][5] BoosterPack.

The parallel I/O of the LM4F120 version of the BoosterPack allows faster screen drawing with the disadvantage of requiring more I/O pins. Although the name implies that it was designed for the Stellaris ARM-based LaunchPad, it will work with MSP430-based LaunchPads that have the 40-pin BOOSTXL interface and enough RAM to support using a large display.

This library is optimized for use with the [MSP430F5529][9] and [MSP432P401R][10] LaunchPads, but will also work with any other MSP or Tiva LaunchPad with the BOOSTXL interface. Display refresh rates on boards other than the F5529 or MSP432 will be much slower due to the use of the generic `DigitalWrite()` for I/O control. When used with the F5529 or MSP432, the library uses a much faster direct register control for I/O.

This library is based on the [Kentec_35_SPI library][8] which is included with Energia.

The screen touch functionality was carried over from the SPI version of the library and the I/O pin number definitions were updated accordingly. However, touch functionality has not been tested with this library.

## Usage

See the sketch included in the `examples` folder.

The parallel library works the same as the built-in SPI version of the library. See the [Reference Manual][12] included with the library.

## Specialized Interface Board

I created a custom interface board to change the mapping of the pins between the Kentec BoosterPack and the F5529 LaunchPad. This was done for two reasons:

- Directly map the data bit positions of the Kentec controller interface to the MSP port bit positions. This allows simplification of the library code to allow even faster screen refresh rates.
- Move the Kentec pins to the inner BOOSTXL header pins (J3 and J4), so that the standard BoosterPack I/O pins are available for other devices (e.g. SPI and I2C pins).

Details on the board design can be found in the [extras/Hardware][11] folder.

The library has been modified to support the interface board. When using the library with the interface, the only code change needed is in the constructor to let the library know that the interface board is connected. By default, the TOUCH functionality is not connected when using the interface board:

    Screen_K35_Parallel myScreen(Screen_K35_Parallel::F5529_INTERFACE_BOARD_INSTALLED, Screen_K35_Parallel::TOUCH_DISABLED);

If, however, you connect the TOUCH sensor pins (XP->P2, YP->P6, XN->P12, YN->P11), then use the following constructor:

    Screen_K35_Parallel myScreen(Screen_K35_Parallel::F5529_INTERFACE_BOARD_INSTALLED, Screen_K35_Parallel::TOUCH_ENABLED);

## References

- EB-LM4F120-L35 BoosterPack [product page][4]
- EB-LM4F120-L35 BoosterPack [user guide][1]
- SSD2119 driver chip [datasheet][2]
- [Review][3] of the EB-LM4F120-L35 from Rei Vilo's Embedded Computing site.
- Kentec_35_SPI library suite [description][8] and [source][6].
- BOOSTXL-K350QVG-S1 (SPI) [product page][7]

## License

This library is based off of code by Rei VILO, <https://embedxcode.weebly.com>, <https://github.com/energia/msp430-lg-core/tree/master/libraries/Kentec_35_SPI>.

The software and other files in this repository are licensed for hobbyist and personal use under the Creative Commons License [CC BY-NC-SA 3.0][100]. See the file [`LICENSE`][101] in this repository.

[1]: ./extras/Kentec%20Parallel%20LCD%20EB-LM4F120-L35_UserGuide_04.pdf
[2]: https://support.newhavendisplay.com/hc/en-us/article_attachments/4414860088727
[3]: https://embeddedcomputing.weebly.com/kentec-35-lcd-with-touch-boosterpack-for-stellaris.html
[4]: https://www.newark.com/kentec-display/eb-lm4f120-l35/exp-board-lcd-boosterpack-stellaris/dp/48W2063
[5]: http://www.ti.com/tool/BOOSTXL-K350QVG-S1
[6]: https://github.com/energia/msp430-lg-core/tree/master/libraries/Kentec_35_SPI
[7]: http://www.ti.com/tool/BOOSTXL-K350QVG-S1
[8]: https://embeddedcomputing.weebly.com/lcd_screen-library-suite.html
[9]: http://www.ti.com/tool/MSP-EXP430F5529LP
[10]: http://www.ti.com/tool/MSP-EXP432P401R
[11]: ./extras/Hardware
[12]: ./extras/LCD_screen-ReferenceManual.pdf
[100]: https://creativecommons.org/licenses/by-nc-sa/3.0/
[101]: ./LICENSE
[//]: # ([200]: https://github.com/Andy4495/Kentec_35_Parallel)
[//]: # ( http://www.kentecdisplay.com/uploads/soft/Products_spec/EB-LM4F120-L35_UserGuide_04.pdf )
[//]: # ( http://www.kentecdisplay.com/uploads/soft/Datasheet/SSD2119_1.4.pdf )
[//]: # ( http://www.kentecdisplay.com/plus/view.php?aid=71 )
