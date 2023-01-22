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
#include "lib/oled1306_i2c/oled1306_i2c.h"

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    puts("Alarm occurred.");
    return 0;
}



int main()
{
    stdio_init_all();

    // useful information for picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_program_description("Micro version of MorseKOB"));

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

    // run through the complete initialization process
    oled_init();

    // init render buffer
    struct render_area area = {start_col: 0, end_col : OLED_WIDTH - 1, start_page : 0, end_page : OLED_NUM_PAGES - 1};
    calc_render_area_buflen(&area);
    
    // test font display
    uint8_t *ptr = display_buf;
    uint16_t mask = 0x00FF;
    uint8_t shift = 0;
    for (int j = 0; j < 8; j++) {
        if (j % 2 == 0) {
            mask = 0x00FF;
            shift = 0;
        }
        else {
            mask = 0xFF00;
            shift = 8;
        }
        // The display is 128 columns, but 14 characters is 126. 
        // So we have 2 blank columns.
        *(ptr++) = 0x00;  // make first col blank 
        for (int i = 0; i < 14 * 9; i++) {
            uint16_t d = Font_Table[i + ((j/2)*(14*9))];
            uint8_t bl = (uint8_t)((d & mask) >> shift);
            *(ptr++) = bl;
        }
        *(ptr++) = 0x00;  // make last col blank
    }
    render(display_buf, &area);

    sleep_ms(5000);

    return 0;
}
