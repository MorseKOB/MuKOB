# MuKOB (μKOB) - Micro-controller based MorseKOB Client Device

Raspberry Pi Pico W (RP2040 CPU) based device with a built-in display and
a terminal interface. Uses built-in wifi to connect to the MorseKOB Server.

It interfaces to a KOB, Key + Sounder, Key + Tone, Paddle + Tone, etc. to provide Morse
input and output. The device can operate completely stand-alone (using the
display-touchscreen) or it can connect to a VT-220 terminal (or emulator like Putty)
to provide a larger interface and allow keyboard input.

It supports both American (railroad) and International code with sounder or tone
output, and allows use of Farnsworth word/character timing to promote improving
copying speed.

This repo also includes the **MuKOB Interface** which can be used with **MKOB** and **MorseKOB** through a USB to a PC.

# References
* [MorseKOB Main Project](https://github.com/MorseKOB)
* [PyKOB - The full/computer MorseKOB Client Application, utilities and Python libraries](https://github.com/MorseKOB/PyKOB)
* [Raspberry Pi Pico W](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)
* [FatFs - Generic FAT Filesystem Module](http://elm-chan.org/fsw/ff/00index_e.html)
* [no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)

# Acknowledgements
The first, and probably most significant acknowledgement has to go to the architects, developers, technical writers, and other staff members responsible for the **Raspberry Pi Pico** and **Pico W**, the **Raspberry Pi Pico C/C++ SDK**, and the **Raspberry Pi Pico C/C++ Samples**.

This application implements a 'C' version of the American Morse encoding and decoding algorithms developed by *Les Kerr*.

The FAT file system and SD card libraries, with some small modifications (to configure for the MuKOB board and remove some printing), are from *ChaN* and *Carl Kugler* respectively.

As with every significant C project I've undertaken since the early '80s, my 'go-to' reference is **The C Programming Language** by *Brian Kernighan* and *Dennis Ritchie* (Prentice Hall, 1978 & 1988).
