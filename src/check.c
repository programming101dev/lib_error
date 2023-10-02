/*
 * Copyright 2021-2022 D'Arcy Smith.
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
    if(strcmp(a, b) == 0)
    {
        P101_ERROR_RAISE_CHECK(err);

        return false;
    }

    return true;
}
