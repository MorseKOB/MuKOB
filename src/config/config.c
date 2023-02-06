/**
 * KOB Configuration functionaly
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
 * MuKOB Configuration functionality
 */

#include <stdio.h>
#include "config.h"
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"

const char* __cfg_filename = "MuKOB.cfg";

int config_init(void) {
    // See if we can read the config from the '.cfg' file...

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret = 0;
    char buf[100];

    // Initialize SD card
    do {  // loop structure to provide breaking out at any step during init...
        if (sd_init_driver()) {
            // Mount drive
            fr = f_mount(&fs, "0:", 1);
            if (fr != FR_OK) {
                ret = 1;
                printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
                break;
            }

            // Open file for reading
            fr = f_open(&fil, __cfg_filename, FA_READ);
            if (fr != FR_OK) {
                ret = 2;
                printf("ERROR: Could not open file (%d)\r\n", fr);
                break;
            }

            // Print every line in file over serial
            printf("Reading from file '%s':\r\n", __cfg_filename);
            printf("---\r\n");
            while (f_gets(buf, sizeof(buf), &fil)) {
                printf(buf);
            }
            printf("\r\n---\r\n");

            // Close file
            fr = f_close(&fil);
            if (fr != FR_OK) {
                ret = 3;
                printf("ERROR: Could not close file (%d)\r\n", fr);
                break;
            }

            // Unmount drive
            f_unmount("0:");
            break;
        }
    } while(true); // Loop structure to allow break out to following code at any point

    return (ret);
}