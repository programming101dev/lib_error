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

#include "p101_error/check.h"
#include "p101_error/error.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

bool p101_check_not_null(struct p101_error *err, const void *ptr)
{
    if(ptr == NULL)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_null(struct p101_error *err, const void *ptr)
{
    if(ptr != NULL)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_greater_than_int(struct p101_error *err, int a, int b)
{
    if(a <= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_less_than_int(struct p101_error *err, int a, int b)
{
    if(a >= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_equals_int(struct p101_error *err, int a, int b)
{
    if(a != b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_equals_string(struct p101_error *err, const char *a, const char *b)
{
    if(a == NULL || b == NULL)
    {
        if(a != b)
        {
            P101_ERROR_RAISE_CHECK(err);

            return false;
        }

        return true;
    }

    if(strcmp(a, b) != 0)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_not_equals_int(struct p101_error *err, int a, int b)
{
    if(a == b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_not_equals_string(struct p101_error *err, const char *a, const char *b)
{
    if(a == NULL || b == NULL)
    {
        if(a == b)
        {
            P101_ERROR_RAISE_CHECK(err);

            return false;
        }

        return true;
    }

    if(strcmp(a, b) == 0)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_greater_than_intmax(struct p101_error *err, intmax_t a, intmax_t b)
{
    if(a <= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_greater_than_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b)
{
    if(a <= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_greater_than_double(struct p101_error *err, double a, double b)
{
    if(isnan(a) || isnan(b) || a <= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_greater_than_long_double(struct p101_error *err, long double a, long double b)
{
    if(isnan(a) || isnan(b) || a <= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_less_than_intmax(struct p101_error *err, intmax_t a, intmax_t b)
{
    if(a >= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_less_than_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b)
{
    if(a >= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_less_than_double(struct p101_error *err, double a, double b)
{
    if(isnan(a) || isnan(b) || a >= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_less_than_long_double(struct p101_error *err, long double a, long double b)
{
    if(isnan(a) || isnan(b) || a >= b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_equals_intmax(struct p101_error *err, intmax_t a, intmax_t b)
{
    if(a != b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_equals_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b)
{
    if(a != b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_not_equals_intmax(struct p101_error *err, intmax_t a, intmax_t b)
{
    if(a == b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_not_equals_uintmax(struct p101_error *err, uintmax_t a, uintmax_t b)
{
    if(a == b)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_in_range_int(struct p101_error *err, int value, int min_value, int max_value)
{
    if(value < min_value || value > max_value)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_in_range_intmax(struct p101_error *err, intmax_t value, intmax_t min_value, intmax_t max_value)
{
    if(value < min_value || value > max_value)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_in_range_uintmax(struct p101_error *err, uintmax_t value, uintmax_t min_value, uintmax_t max_value)
{
    if(value < min_value || value > max_value)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_in_range_double(struct p101_error *err, double value, double min_value, double max_value)
{
    if(isnan(value) || isnan(min_value) || isnan(max_value) || value < min_value || value > max_value)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}

bool p101_check_in_range_long_double(struct p101_error *err, long double value, long double min_value, long double max_value)
{
    if(isnan(value) || isnan(min_value) || isnan(max_value) || value < min_value || value > max_value)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}
