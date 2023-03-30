# Pico-W Pins used for the μKOB device

The μKOB device uses most of the Pico-W's GPIO/PIO pins. Because both hardware
SPI's are used, as well as two pins for PIO (for the rotary encoder), the pins
assigned can't be changed too much. The definitions for the pins used are in
the 'system_defs.h' file.

As defined, the pin assignments are
 (Pins within single-quotes are fixed use. NU=Not Used):
| |        Use        | PIN  |  |  PIN |         Use       |
|-|-------------------|------|--|------|-------------------|
| |            TTY-TX |   1  |  |  40  | 'VBUS'            |
| |            TTY-RX |   2  |  |  39  | 'VSYS'            |
| |             'GND' |   3  |  |  38  | 'GND'             |
| | SD+Touch SPI0-SCK |   4  |  |  37  | '3V3_EN'          |
| |  SD+Touch SPI0-TX |   5  |  |  36  | '3V3 OUT'         |
| |  SD+Touch SPI0-RX |   6  |  |  35  | 'ADC_VREF'        |
| |         SD Select |   7  |  |  34  | GP-28 (NU)        |
| |             'GND' |   8  |  |  33  | 'GND/AGND'        |
| |      Touch Select |   9  |  |  32  | Display Backlight |
| |         Touch IRQ |  10  |  |  31  | Display Reset     |
| |  Display CMD/Data |  11  |  |  30  | 'RUN'             |
| |    Display Select |  12  |  |  29  | Speaker           |
| |             'GND' |  13  |  |  28  | 'GND'             |
| |  Display SPI1-SCK |  14  |  |  27  | Options SW - 4    |
| |   Display SPI1-TX |  15  |  |  26  | Options SW - 3    |
| |   Display SPI1-RX |  16  |  |  25  | Options SW - 2    |
| |    Rotary Push-SW |  17  |  |  24  | Options SW - 1    |
| |             'GND' |  18  |  |  23  | 'GND'             |
| |    Rotary Phase-A |  19  |  |  22  | Morse Key         |
| |    Rotary Phase-B |  20  |  |  21  | Morse Sounder     |
|_|___________________|______|__|______|___________________|
Notes:
 SPI0 and SPI1 use the hardware SPI's. Therefore, they have limited pins.
 The Rotary A:B use PIO. They must be 2 sequential GP pins for 'A' then 'B'.
 The 'Options SW' is to select 'hard' options used before the SD is
  initialized. They were selected as 4 GPIO's in a row to make it easy to
  breadboard (also picked these since I2C is not being used).
