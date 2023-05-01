/**
 * Some content Copyright (c) 2021 pmarques-dev @ github
 *
 * Modifications Copyright 2023 aesilky
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
//
// ---- quadrature encoder interface example (from pico-examples)
//
// the PIO program reads phase A/B of a quadrature encoder and increments or
// decrements an internal counter to keep the current absolute step count
// updated. At any point, the main code can query the current count by using
// the quadrature_encoder_*_count functions. The counter is kept in a full
// 32 bit register that just wraps around. Two's complement arithmetic means
// that it can be interpreted as a 32-bit signed or unsigned value, and it will
// work anyway.
//
// As an example, a two wheel robot being controlled at 100Hz, can use two
// state machines to read the two encoders and in the main control loop it can
// simply ask for the current encoder counts to get the absolute step count. It
// can also subtract the values from the last sample to check how many steps
// each wheel as done since the last sample period.
//
// One advantage of this approach is that it requires zero CPU time to keep the
// encoder count updated and because of that it supports very high step rates.
//

// Modifications and additions...
//
// Use interrupts to know when the encoder has moved.
// Add reading of the switch, also using the interrupt to know when it has changed.
//
#include "system_defs.h"
#include "rotary_encoder.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

#include "quadrature_encoder.pio.h"

#include <stdio.h>


#define _PIN_rotary_ENC_AB ROTARY_A_IN   // Base pin to connect the A phase of the encoder.
                                        // The B phase must be connected to the next pin

#define SM 0

static char event_str[128];
static int old_value = 0; 
static PIO pio_rotary_encoder = pio0;

static void _gpio_event_string(char *buf, uint32_t events);

void re_turn_irq_handler(uint gpio, uint32_t events) {
    int new_value, delta = 0;
    // note: thanks to two's complement arithmetic delta will always
    // be correct even when new_value wraps around MAXINT / MININT
    new_value = quadrature_encoder_get_count(pio_rotary_encoder, SM);
    delta = new_value - old_value;
    old_value = new_value;

    if (delta != 0) {
        printf("position %8d, delta %6d\n", new_value, delta);
    }
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

static void _gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}


void rotary_encoder_module_init() {
    // GPIO is initialized in `mkboard.c` with the rest of the board.
    uint offset = pio_add_program(pio_rotary_encoder, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio_rotary_encoder, SM, offset, _PIN_rotary_ENC_AB, 0);
}

