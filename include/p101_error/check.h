#ifndef LIBP101_ERROR_CHECK_H
#define LIBP101_ERROR_CHECK_H

/*
 * Copyright 2021-2024 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "error.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* The comparison macros dispatch on the type of the FIRST operand, so wide,
 * unsigned, and floating types work without casts under -Wconversion. Note
 * that EQUALS/NOT_EQUALS deliberately have no floating variants: comparing
 * floats for equality is almost always a bug (-Wfloat-equal agrees). */
#define P101_CHECK_NOT_NULL(err, ptr) p101_check_not_null((err), (ptr))
#define P101_CHECK_NULL(err, ptr) p101_check_null((err), (ptr))
#define P101_CHECK_GREATER_THAN(err, a, b)                                                                                                                                                                                                                         \
    _Generic((a),                                                                                                                                                                                                                                                  \
        float: p101_check_greater_than_double,                                                                                                                                                                                                                     \
        double: p101_check_greater_than_double,                                                                                                                                                                                                                    \
        unsigned int: p101_check_greater_than_uintmax,                                                                                                                                                                                                             \
        unsigned long: p101_check_greater_than_uintmax,                                                                                                                                                                                                            \
        unsigned long long: p101_check_greater_than_uintmax,                                                                                                                                                                                                       \
        long: p101_check_greater_than_intmax,                                                                                                                                                                                                                      \
        long long: p101_check_greater_than_intmax,                                                                                                                                                                                                                 \
        default: p101_check_greater_than_int)((err), (a), (b))
#define P101_CHECK_LESS_THAN(err, a, b)                                                                                                                                                                                                                            \
    _Generic((a),                                                                                                                                                                                                                                                  \
        float: p101_check_less_than_double,                                                                                                                                                                                                                        \
        double: p101_check_less_than_double,                                                                                                                                                                                                                       \
        unsigned int: p101_check_less_than_uintmax,                                                                                                                                                                                                                \
        unsigned long: p101_check_less_than_uintmax,                                                                                                                                                                                                               \
        unsigned long long: p101_check_less_than_uintmax,                                                                                                                                                                                                          \
        long: p101_check_less_than_intmax,                                                                                                                                                                                                                         \
        long long: p101_check_less_than_intmax,                                                                                                                                                                                                                    \
        default: p101_check_less_than_int)((err), (a), (b))
#define P101_CHECK_EQUALS(err, a, b)                                                                                                                                                                                                                               \
    _Generic((a),                                                                                                                                                                                                                                                  \
        char *: p101_check_equals_string,                                                                                                                                                                                                                          \
        const char *: p101_check_equals_string,                                                                                                                                                                                                                    \
        unsigned int: p101_check_equals_uintmax,                                                                                                                                                                                                                   \
        unsigned long: p101_check_equals_uintmax,                                                                                                                                                                                                                  \
        unsigned long long: p101_check_equals_uintmax,                                                                                                                                                                                                             \
        long: p101_check_equals_intmax,                                                                                                                                                                                                                            \
        long long: p101_check_equals_intmax,                                                                                                                                                                                                                       \
        default: p101_check_equals_int)((err), (a), (b))
#define P101_CHECK_NOT_EQUALS(err, a, b)                                                                                                                                                                                                                           \
    _Generic((a),                                                                                                                                                                                                                                                  \
        char *: p101_check_not_equals_string,                                                                                                                                                                                                                      \
        const char *: p101_check_not_equals_string,                                                                                                                                                                                                                \
        unsigned int: p101_check_not_equals_uintmax,                                                                                                                                                                                                               \
        unsigned long: p101_check_not_equals_uintmax,                                                                                                                                                                                                              \
        unsigned long long: p101_check_not_equals_uintmax,                                                                                                                                                                                                         \
        long: p101_check_not_equals_intmax,                                                                                                                                                                                                                        \
        long long: p101_check_not_equals_intmax,                                                                                                                                                                                                                   \
        default: p101_check_not_equals_int)((err), (a), (b))
#define P101_CHECK_IN_RANGE(err, value, min_value, max_value)                                                                                                                                                                                                      \
    _Generic((value),                                                                                                                                                                                                                                              \
        float: p101_check_in_range_double,                                                                                                                                                                                                                         \
        double: p101_check_in_range_double,                                                                                                                                                                                                                        \
        unsigned int: p101_check_in_range_uintmax,                                                                                                                                                                                                                 \
        unsigned long: p101_check_in_range_uintmax,                                                                                                                                                                                                                \
        unsigned long long: p101_check_in_range_uintmax,                                                                                                                                                                                                           \
        long: p101_check_in_range_intmax,                                                                                                                                                                                                                          \
        long long: p101_check_in_range_intmax,                                                                                                                                                                                                                     \
        default: p101_check_in_range_int)((err), (value), (min_value), (max_value))

    bool p101_check_not_null(struct p101_error *err, const void *ptr);
    bool p101_check_null(struct p101_error *err, const void *ptr);
    bool p101_check_greater_than_int(struct p101_error *err, int a, int b);
    bool p101_check_greater_than_intmax(struct p101_error *err, intmax_t a, intmax_t b);
    bool p101_check_greater_than_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b);
    bool p101_check_greater_than_double(struct p101_error *err, double a, double b);
    bool p101_check_less_than_int(struct p101_error *err, int a, int b);
    bool p101_check_less_than_intmax(struct p101_error *err, intmax_t a, intmax_t b);
    bool p101_check_less_than_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b);
    bool p101_check_less_than_double(struct p101_error *err, double a, double b);
    bool p101_check_equals_int(struct p101_error *err, int a, int b);
    bool p101_check_equals_intmax(struct p101_error *err, intmax_t a, intmax_t b);
    bool p101_check_equals_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b);
    bool p101_check_equals_string(struct p101_error *err, const char *a, const char *b);
    bool p101_check_not_equals_int(struct p101_error *err, int a, int b);
    bool p101_check_not_equals_intmax(struct p101_error *err, intmax_t a, intmax_t b);
    bool p101_check_not_equals_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b);
    bool p101_check_not_equals_string(struct p101_error *err, const char *a, const char *b);
    bool p101_check_in_range_int(struct p101_error *err, int value, int min_value, int max_value);
    bool p101_check_in_range_intmax(struct p101_error *err, intmax_t value, intmax_t min_value, intmax_t max_value);
    bool p101_check_in_range_uintmax(struct p101_error *err, uintmax_t value, uintmax_t min_value, uintmax_t max_value);
    bool p101_check_in_range_double(struct p101_error *err, double value, double min_value, double max_value);

#ifdef __cplusplus
}
#endif

#endif    // LIBP101_ERROR_CHECK_H
