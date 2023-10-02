#ifndef LIBP101_ERROR_ERROR_H
#define LIBP101_ERROR_ERROR_H

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

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __STDC_LIB_EXT1__
    typedef int errno_t;
#endif

    /**
     * Error states.
     */
    typedef enum
    {
        P101_ERROR_NONE = 0, /**< There is no error */
        P101_ERROR_CHECK,    /**< The error is due to a failed check */
        P101_ERROR_ERRNO,    /**< The error is due to errno being set */
        P101_ERROR_SYSTEM,   /**< The error is from a library call that doesn't set errno */
        P101_ERROR_USER,     /**< The error is from a non-standard function */
    } p101_error_type;

    struct p101_error;

    struct p101_error *p101_error_create(bool report);

    /**
     *
     * @param err
     * @return
     */
    bool p101_error_is_reporting(struct p101_error *err);

    /**
     *
     * @param err
     * @param on
     */
    void p101_error_set_reporting(struct p101_error *err, bool on);

    const char *p101_error_get_message(const struct p101_error *err);

    /**
     *
     * @param err
     */
    void p101_error_reset(struct p101_error *err);

    /**
     *
     * @param err
     */
    void p101_error_default_error_reporter(const struct p101_error *err);

    /**
     *
     * @param err
     * @param file_name
     * @param function_name
     * @param line_number
     */
    void p101_error_check(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number);

    /**
     *
     * @param err
     * @param file_name
     * @param function_name
     * @param line_number
     * @param err_code
     */
    void p101_error_errno(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, errno_t err_code);

    /**
     *
     * @param err
     * @param file_name
     * @param function_name
     * @param line_number
     * @param msg
     * @param err_code
     */
    void p101_error_system(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, const char *msg, int err_code);

    /**
     *
     * @param err
     * @param file_name
     * @param function_name
     * @param line_number
     * @param msg
     * @param err_code
     */
    void p101_error_user(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, const char *msg, int err_code);

    /**
     *
     * @param err
     * @return
     */
    bool p101_error_has_error(const struct p101_error *err);

    /**
     *
     * @param err
     * @return
     */
    bool p101_error_has_no_error(const struct p101_error *err);

    /**
     *
     * @param err
     * @param code
     * @return
     */
    bool p101_error_is_errno(const struct p101_error *err, errno_t code);

    errno_t p101_errno_get_errno(const struct p101_error *err);

#define P101_ERROR_RAISE_CHECK(err) p101_error_check((err), __FILE__, __func__, __LINE__)
#define P101_ERROR_RAISE_ERRNO(err, code) p101_error_errno((err), __FILE__, __func__, __LINE__, (code))
#define P101_ERROR_RAISE_SYSTEM(err, msg, code) p101_error_system((err), __FILE__, __func__, __LINE__, (msg), (code))
#define P101_ERROR_RAISE_USER(err, msg, code) p101_error_user((err), __FILE__, __func__, __LINE__, (msg), (code))

#ifdef __cplusplus
}
#endif

#endif    // LIBP101_ERROR_ERROR_H
