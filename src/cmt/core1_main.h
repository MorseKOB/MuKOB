/**
 * MuKOB Core 1 main start-up and management.
 *
 * This containes the main routine (the entry point) for operations on Core 1.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _CORE1MAIN_H_
#define _CORE1MAIN_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file core1_main.h
 * @defgroup mukob_core1 mukob_core1
 * Core 1 supervisor and main entry point.
 *
 * NOTE: Fifo use
 * MuKOB uses the Fifo for base (core (ha!)) level communication between
 * the cores for the supervisor/system functionality. The Pico SDK warns about
 * using the Fifo, as it is used by the Pico runtime to support multicore operation.
 *
 * MuKOB acknowledges the warnings, and will take care to assure that the Fifo and
 * the MuKOB code is in an appropriate state when operations are performed that will
 * cause the Pico SDK/runtime to use them.
 *
 * For general purpose application communication between the functionality running on
 * the two cores queues will be used.
 *
 * @addtogroup mk_multicore
 * @include multicore.c
 *
*/

/**
 * @brief Main entry point for the Core 1 functionality.
 * @ingroup mk_multicore
 *
 * The core 0 `main` (or a helper) will initialize the core 1 base environment
 * and the inter-core communication and then cause this to be executed on
 * core 1.
 *
 * Once this is running on core 1, it will finish setting up the core 1
 * environment and then start the core 1 displatchr.
 *
 * MuKOB runs the User Interface (UI, aka Frontend/FE) on Core 1, while it
 * runs the Backend (BE) on Core 0. The application functionality contained in
 * the Backend includes the Net/Wire, KOB, and Morse.
 *
 */
void core1_main(void);

#ifdef __cplusplus
    }
#endif
#endif // _CORE1MAIN_H_
