# TFT-LCD Display Modules w/Touch and SD Card

There are a number of commonly found, different sized, SPI display modules
with a red PCB, ILITECH display controller, ADS7843/XPT2046 touch screen
controller, and a full-sized SD card slot, sold under different brand names
(HiLetgo, Adafruit, Hosyond, etc.). Regardless of the distributor selling it,
they are all designed and manufactured by:
Shenzhen Junxian Electronic Technology Co., Ltd
[QDTech China](https://szjunxian.en.alibaba.com/)

The specific ILITECH display controller the module uses depends on the
resolution and screen size of the TFT-LCD panel.

Resolution - Size = Controller:

* 240x320 - 2.8" = ILI9341
* 240x320 - 3.2" = ILI9341
* 320x480 - 3.5" = ILI9844
* 320x480 - 4.0" = ILI9486

Touch Screen Controller: TI (Burr-Brown) ADS7843 / SHENZHEN XPTEK XPT2046

SD Card: Full size card socket using 4-wire SPI interface mode.

Module Power (internal): 3.3V. From connector `VCC` or TOREX XC6206 Precicion Voltage Regulator with `VCC` as the source.

## SPI Interface

The SPI connections are seperate for the three functional areas with a common
power connection. The power, display, and touch connections are via one 14
pin connector (1x14 pin). The SD card connection is through a separate
(1x4 pin) connector.

The modules are available without the touch screen (generally, with a 1 digit
difference in model number).

## Power

There is a jumper (SMD resistor location), `J1` on the PCB that is shipped open
(not populated). Shorting it (populating with a 0 ohm resistor) bypasses the
TOREX XC6206 3.3v regulator that is sourced from the `VCC` pin on the 14 pin
connector.

`J1` **MUST** be open if the module is powered from 5v. It can be closed (wired or 0 ohm resistor/shunt populated) if powering the module from 3.3v to avoid the small (~250mV)
drop introduced by the regulator.

Note that **all logic signals are 3.3v**.

## Touch Screen Controller

The `Vref` of the ADS7843/XPT2046 touch screen controller is tied to (the internal)
`VCC3.3`. The `VCC3.3` is either the output from the on-module 3.3v regulator or the
`VCC` input of the module, depending on the state of `J1` (See: [Power](#markdown-header-power)).

If `J1` is left open, the locally generated `VCC3.3` is likely to be more stable than the
voltage of `VCC` provided by the connector. However, the (possibly) lower value of
Vref must be accounted for when reading ADC values.
