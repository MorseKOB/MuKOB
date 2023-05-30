# TFT-LCD Display Modules w/Touch and SD Card

All of the commonly found, different sized, SPI display modules
(the red PCB with the ILIxxxx controller, XPT2046 touch screen,
and full-sized SD card slot) sold under different brand names
(HiLetgo, Adafruit, Hosyond, etc.) are from
Shenzhen Junxian Electronic Technology Co., Ltd
[QDTech China](https://szjunxian.en.alibaba.com/)

Resolution - Size = Controller:
* 240x320 - 2.8" = ILI9341
* 240x320 - 3.2" = ILI9341
* 320x480 - 3.5" = ILI9844
* 320x480 - 4.0" = ILI9486

Touch Screen Controller: XPT2046 (TI-ADS7843)

SD Card: Full size using 4-wire SPI interface mode.

The SPI connections are seperate for the three functional areas with a common
power connection. The power, display, and touch connections are via one 14
pin connector (1x14 pin). The SD card connection is through a separate
connector (1x4). The modules are available without the touch screen (generally,
with a 1 digit difference in model number).

There is a jumper on the PCB (J1) that is shipped open. Shorting it bypasses
a 3.3v regulator that is sourced from the `VCC` pin on the 14 pin connector.
The jumper **MUST** be open if the module is powered from 5v. It can be
solder jumpered if powering the module from 3.3v to avoid the drop introduced
by the regulator. All logic signals are at 3.3v.
