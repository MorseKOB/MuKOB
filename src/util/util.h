/**
 * Utility functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UTIL_H_
#define _UTIL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "pico/types.h"

typedef enum _STRDATETIME_CTRL_ {
    SDTC_TIME               = 0x0001,
    SDTC_TIME_SECONDS       = 0x0003, // indlude seconds (implies time)
    SDTC_TIME_AMPM          = 0x0005, // add 'AM/PM' indicator (implies time)
    SDTC_TIME_2DIGITS       = 0x0101, // 2 digit time numbers (implies time)
    SDTC_TIME_24HOUR        = 0x0201, // 24 hour format (implies time)
    SDTC_TIME_2CHAR_HOUR    = 0x8001, // Use a leading space for hours 1-9 (implies time)
    SDTC_TIME_BEFORE_DATE   = 0x4009, // put time before date (implies date and time)
    SDTC_DATE               = 0x0008,
    SDTC_DATE_SLASH         = 0x0018, // use '/' rather than '-' (implies date)
    SDTC_DATE_2DIGITS       = 0x0408, // 2 digit month/day numbers (implies date)
    SDTC_DATE_ORDER_DM      = 0x0808, // use day/month rather than month/day
    SDTC_DATE_SHORT_DM      = 0x2088, // use short day and month names (implies long text date)
    SDTC_LONG_TXT           = 0x0088, // date sentence (implies date)
    SDTC_LONG_TXT_AT        = 0x00C9, // date 'at' time (implies long text date and time)
    SDTC_LONG_TXT_ON        = 0x40A9, // time 'on' date (implies long text date, time befor date)
    SDTC_YEAR_2DIGITS       = 0x1008, // 2 digit year (implies date)
} strdatetime_ctrl_t;

/**
 * @brief Return precisely 0 or 1 from a zero / non-zero value.
 *
 * @param b An int/bool value to be converted
 * @return uint8_t 0 for 0, 1 for non-zero.
 */
extern uint8_t binary_from_bool(int b);

/**
 * @brief Get a bool (true/false) value from a string.
 * @ingroup util
 *
 * A `true` string is any of (all ignore case):
 *  "1"
 *  "on"
 *  "true"
 *  "yes"
 * Anything else is `false`
 *
 * @param str The string to check
 * @return true If any of "1", "on", "true", "yes"
 * @return false If any other value
 */
extern bool bool_from_str(const char* str);

/**
 * @brief Get the number of days in a month.
 * @ingroup util
 *
 * @param month The month (1 - 12) 1 is January.
 * @param year  The year the month is in (needed to know if it is leap year). Passing 0 is a non-leap year.
 * @return int8_t The number of days.
 */
extern int8_t days_in_month(int8_t month, int16_t year);

/**
 * @brief Get the day of the year giving the year, month, and day (of the month).
 * @ingroup util
 *
 * @param day Day of the month (1 - 31,30,29 or 28)
 * @param month The month (1 - 12, 1 is January)
 * @param year  The year (if unknown, 0 works as a non-leap year)
 * @return int16_t The day in the year
 */
extern int16_t day_of_year(int8_t day, int8_t month, int16_t year);

/**
 * @brief Is the year a leap year.
 *
 * @param year The year to check.
 * @return true The year is a leap year
 * @return false The year is not a leap year
 */
extern bool is_leap_year(int16_t year);

/**
 * @brief Return the ordinal ('st', 'nd', 'rd', 'th') for a number.
 *
 * @param num The number
 * @return const char* The ordinal (NULL terminated)
 */
extern const char* num_ordinal(int num);

/**
 * @brief Format a date-time into a string.
 * @ingroup util
 *
 * Formats a Pico RTC datetime_t into a string buffer. The control flags are used
 * to control result.
 *
 * @param buf The string buffer to format the date-time into
 * @param bufsize The size of the buffer
 * @param dt The datetime to format
 * @param ctrl Control the format (one or more or'ed together)
 */
extern void strdatetime(char* buf, uint bufsize, datetime_t* dt, strdatetime_ctrl_t ctrl);

/**
 * @brief Replace newline characters with '\000'.
 * @ingroup util
 *
 * @param str The string to modify.
 * @return char* A pointer to the same string that was passed in.
 */
extern char* strnltonull(char* str);

/**
 * @brief Skip leading whitespace (space and tab).
 * @ingroup util
 *
 * @param str String to process.
 * @return char* Pointer to the first non-whitespace character. It is possible it will
 *               be the terminating '\000' if the string was empty or all whitespace. The
 *               pointer returned is an index into the string that was passed in (not a copy).
 */
extern char* strskipws(char* str);

/**
 * @brief Uppercase a string.
 * @ingroup util
 *
 * @param dest Character buffer large enough for the uppercased string plus a terminating NULL.
 * @param str The string to uppercase.
 */
extern void strtoupper(char* dest, const char* str);

#ifdef __cplusplus
    }
#endif
#endif // _UTIL_H_
