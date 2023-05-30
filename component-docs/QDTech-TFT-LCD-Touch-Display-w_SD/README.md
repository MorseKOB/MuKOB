All of the different sized SPI display modules (the red PCB with the ILIxxxx controller, XPT2046 touch screen, and SD card slot) are from Shenzhen Junxian Electronic Technology Co., Ltd (QDTech China).

Resolution - Size = Controller:
 240x320 - 2.8" = ILI9341
 240x320 - 3.2" = ILI9341
 320x480 - 3.5" = ILI9844
 320x480 - 4.0" = ILI9486

Touch Screen Controller: XPT2046 (TI-ADS7843)

SD Card: Full size using 4-wire SPI interface mode.

The SPI connections are seperate for the three functional areas with a common
power connection. The power, display, and touch connections are via one 14
pin connector (1x14 pin). The SD card connection is through a separate
connector (1x4). The modules are available without the touch screen (with a 1
digit difference on model number).
