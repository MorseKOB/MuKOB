# MuKOB (μKOB) - Micro-controller version MorseKOB client

Targeted at the Raspberry Pi Pico W (RP2040 CPU) with some specific
parapherals.

# References
* [MorseKOB Main Project](https://github.com/MorseKOB)
* [PyKOB - The full/computer MorseKOB Client Application, utilities and Python libraries](https://github.com/MorseKOB/PyKOB)
* [FatFs - Generic FAT Filesystem Module](http://elm-chan.org/fsw/ff/00index_e.html)
* [no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)

# Acknowledgements
The first, and probably most significant acknowledgement has to go to the architects, developers, technical writers, and other staff members responsible for the **Raspberry Pi Pico** and **Pico W**, the **Raspberry Pi Pico C/C++ SDK**, and the **Raspberry Pi Pico C/C++ Samples**.

This application implements a 'C' version of the American Morse encoding and decoding algorithms developed by *Les Kerr*.

The FAT file system and SD card libraries, with some small modifications (to configure for the MuKOB board and remove some printing), are from *ChaN* and *Carl Kugler* respectively.

As with every significant C project I've undertaken since the early '80s, my 'go-to' reference is **The C Programming Language** by *Brian Kernighan* and *Dennis Ritchie* (Prentice Hall, 1978 & 1988).
