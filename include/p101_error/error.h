#ifndef LIBP101_ERROR_ERROR_H
#define LIBP101_ERROR_ERROR_H

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

/*
 * Thread-safety: a struct p101_error is NOT thread-safe and must not be
 * shared between threads. The convention is one error object per thread,
 * matching the one-env-per-thread convention in p101_env.
 */

#include <errno.h>
#include <p101_error/attributes.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Annex K is optional and absent on glibc, FreeBSD libc, and Apple libc.
 * So a fallback typedef is required on every platform we support. cppcheck
 * already knows errno_t as a built-in type and reports a syntax error on
 * the redefinition, so the fallback is hidden from it (__CPPCHECK__).
 * Newer cppcheck (>= 2.14) parses inactive preprocessor branches too and
 * still trips on the typedef despite the guard, hence the suppression. */
#if !defined(__STDC_LIB_EXT1__) && !defined(__CPPCHECK__)
    /* cppcheck-suppress syntaxError */
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
    typedef void (*p101_error_reporter)(const struct p101_error *err);

    /*
     * Explicitly mark a best-effort call that intentionally has no error
     * sink. This is a typed constant, rather than a hidden function call, so
     * using it as an argument preserves the one-call-per-expression contract.
     */
    extern struct p101_error *const p101_error_optional_sink P101_ATTR_SEMANTIC_ROLE("p101:optional-error");    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables): the pointer value is immutable; the wrapper API requires a non-const pointee type.
#define P101_ERROR_OPTIONAL p101_error_optional_sink

    struct p101_error *p101_error_create(bool report) P101_ATTR_MALLOC P101_ATTR_WARN_UNUSED_RESULT P101_ATTR_SEMANTIC_ROLE("p101:ownership:error:acquire");
    void               p101_error_destroy(struct p101_error *err) P101_ATTR_SEMANTIC_ROLE("p101:ownership:error:release");

    bool        p101_error_is_reporting(const struct p101_error *err);
    void        p101_error_set_reporting(struct p101_error *err, bool on);
    const char *p101_error_get_message(const struct p101_error *err);
    void        p101_error_reset(struct p101_error *err);
    void        p101_error_default_error_reporter(const struct p101_error *err);
    void        p101_error_check(struct p101_error *err, const char *file_name, const char *function_name, int line_number);
    void        p101_error_errno(struct p101_error *err, const char *file_name, const char *function_name, int line_number, errno_t err_code);
    void        p101_error_system(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code);
    void        p101_error_user(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code);
    void        p101_error_user_printf(struct p101_error *err, const char *file_name, const char *function_name, int line_number, int err_code, const char *fmt, ...) P101_ATTR_PRINTF(6, 7);
    bool        p101_error_has_error(const struct p101_error *err) P101_ATTR_SEMANTIC_ROLE("p101:error-state-query:positive");
    bool        p101_error_has_no_error(const struct p101_error *err) P101_ATTR_SEMANTIC_ROLE("p101:error-state-query:negative");
    bool        p101_error_is_errno(const struct p101_error *err, errno_t code) P101_ATTR_SEMANTIC_ROLE("p101:error-state-query");
    bool        p101_error_is_error(const struct p101_error *err, p101_error_type type, int code) P101_ATTR_SEMANTIC_ROLE("p101:error-state-query");
    bool        p101_error_copy(struct p101_error *dst, const struct p101_error *src);
    void        p101_error_move(struct p101_error *dst, struct p101_error *src);

    /* Inspection: these only read an error the caller already received through
     * the normal (env, err) channel -- there is deliberately no way to install
     * custom handling at raise time. */
    p101_error_type p101_error_get_type(const struct p101_error *err);
    int             p101_error_get_code(const struct p101_error *err);
    errno_t         p101_error_get_errno(const struct p101_error *err);
    const char     *p101_error_get_file_name(const struct p101_error *err);
    const char     *p101_error_get_function_name(const struct p101_error *err);
    int             p101_error_get_line_number(const struct p101_error *err);

    /* Older name for p101_error_get_errno(); kept for compatibility. */
    errno_t p101_errno_get_errno(const struct p101_error *err);

#define P101_ERROR_RAISE_CHECK(err) p101_error_check((err), __FILE__, __func__, __LINE__)
#define P101_ERROR_RAISE_ERRNO(err, code) p101_error_errno((err), __FILE__, __func__, __LINE__, (code))
#define P101_ERROR_RAISE_SYSTEM(err, msg, code) p101_error_system((err), __FILE__, __func__, __LINE__, (msg), (code))
#define P101_ERROR_RAISE_USER(err, msg, code) p101_error_user((err), __FILE__, __func__, __LINE__, (msg), (code))
#define P101_ERROR_RAISE_USER_PRINTF(err, code, ...) p101_error_user_printf((err), __FILE__, __func__, __LINE__, (code), __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif    // LIBP101_ERROR_ERROR_H
