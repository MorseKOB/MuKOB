#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// I2C defines
// This example will use the default I2C on GPIO4 (SDA) and GPIO5 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c_default
#define I2C_SDA PICO_DEFAULT_I2C_SDA_PIN
#define I2C_SCL PICO_DEFAULT_I2C_SCL_PIN
// (above defines are used in this, so must define before including)
#include "lib/display/display.h"
#include "lib/sd_spi/sd_driver/sd_spi.h"
#include "lib/sd_spi/ff15/source/ff.h"

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    puts("Alarm occurred.");
    return 0;
}



int main()
{
    stdio_init_all();

    // // useful information for picotool
    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    // bi_decl(bi_program_description("Micro version of MorseKOB"));

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);


    // Timer example code - This example fires off the callback after 2000ms
    //add_alarm_in_ms(2000, alarm_callback, NULL, false);

    puts("MuKOB says hello!");

    // Initialize the display
    disp_init();

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test02.txt";

    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");
    while (true) {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n')) {
            break;
        }
        else {
            printf("\r\nSD card test. Press 'enter' to start.\r\n");
        }
    }

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }

    // Open file for writing ()
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Write something to file
    ret = f_printf(&fil, "This is another test\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }
    ret = f_printf(&fil, "of writing to an SD card.\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // Open file for reading
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Print every line in file over serial
    printf("Reading from file '%s':\r\n", filename);
    printf("---\r\n");
    while (f_gets(buf, sizeof(buf), &fil)) {
        printf(buf);
    }
    printf("\r\n---\r\n");

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // Unmount drive
    f_unmount("0:");


    while(true) {
//        disp_font_test();
//        sleep_ms(1000);
        disp_clear(true);
        char d = '\x01';
        for (unsigned int r = 0; r < 6; r++) {
            for (unsigned int c = 0; c < 14; c++) {
                disp_char(r, c, d++, false);
            }
            disp_row_paint(r);
        }
        sleep_ms(1000);
        // Test setting the invert on chars directly in the text buffer
        // then paint.
        for (int i = 0; i < 10; i++) {
            int offset = (5 * DISP_CHAR_COLS) + i;
            *(text_full_screen + offset) = (*(text_full_screen + offset) | DISP_CHAR_INVERT_BIT);
        }
        disp_update(true);
        sleep_ms(1000);
        // Test scrolling 3 rows up
        disp_rows_scroll_up(2, 4, true);
        sleep_ms(1000);
        // Test clearing a row
        disp_row_clear(1, true);
        sleep_ms(1000);
        // Test displaying a string
        disp_string(4, 2, "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS.", false, true);
        sleep_ms(1000);
        // Simulate the MuKOB screen
        disp_clear(true);
        const char status_bar[] = {0x0B,0x1B,'1','0','8',' ',0x1A,'2','5',' ',0x01,0x04,0x1F,0x1E,0x00};
        disp_string(0, 0, status_bar, true, false);
        disp_string(1,0,"HOW EATING OYSTERS COULD HELP PROTECT THE ", false, false);
        disp_string(5,0,"BBC WORLD NE \x11", false, true);
        sleep_ms(5000);
    }

    return 0;
}
