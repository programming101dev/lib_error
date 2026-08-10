/*
 * Copyright 2021-2025 D'Arcy Smith.
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

#include "p101_error/error.h"
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum
{
    P101_ERROR_MESSAGE_BUFFER_SIZE = 1024
};

struct p101_error
{
    const char         *const_message; /* non-owned, static or caller-managed */
    char               *message;       /* owned heap copy, if present */
    const char         *file_name;     /* non-owned */
    const char         *function_name; /* non-owned */
    int                 line_number;
    p101_error_type     type;
    p101_error_reporter reporter;

    union
    {
        errno_t errno_code;
        int     err_code;
    };
};

static void error_init(struct p101_error *err, p101_error_reporter reporter);
static void free_heap_message(struct p101_error *err);
static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg);
static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg);
static void setup_error_owned(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, char *msg);

struct p101_error *const p101_error_optional_sink = NULL;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables): exported immutable typed null sentinel.

struct p101_error *p101_error_create(bool report)
{
    void              *allocation;
    struct p101_error *err;

    allocation = malloc(sizeof *err);
    err        = (struct p101_error *)allocation;
    if(err != NULL)
    {
        p101_error_reporter reporter = NULL;

        if(report)
        {
            reporter = p101_error_default_error_reporter;
        }
        error_init(err, reporter);
    }
    return err;
}

void p101_error_destroy(struct p101_error *err)
{
    if(err != NULL)
    {
        p101_error_reset(err);
        free(err);
    }
}

static void error_init(struct p101_error *err, p101_error_reporter reporter)
{
    memset(err, 0, sizeof *err);
    err->reporter = reporter;
}

static void free_heap_message(struct p101_error *err)
{
    if(err && err->message)
    {
        free(err->message);
        err->message = NULL;
    }
    if(err)
    {
        err->const_message = NULL;
    }
}

void p101_error_reset(struct p101_error *err)
{
    if(err != NULL)
    {
        p101_error_reporter reporter = err->reporter;
        free_heap_message(err);
        error_init(err, reporter);
    }
}

bool p101_error_is_reporting(const struct p101_error *err)
{
    return (err != NULL && err->reporter != NULL) != 0;
}

void p101_error_set_reporting(struct p101_error *err, bool on)
{
    if(err != NULL)
    {
        if(on)
        {
            err->reporter = p101_error_default_error_reporter;
        }
        else
        {
            err->reporter = NULL;
        }
    }
}

const char *p101_error_get_message(const struct p101_error *err)
{
    const char *p101_single_result_;
    if(err == NULL)
    {
        p101_single_result_ = NULL;
        goto p101_single_exit_;
    }
    if(err->message)
    {
        p101_single_result_ = err->message;
        goto p101_single_exit_;
    }
    if(err->const_message)
    {
        p101_single_result_ = err->const_message;
        goto p101_single_exit_;
    }
    p101_single_result_ = NULL;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

void p101_error_default_error_reporter(const struct p101_error *err)
{
    if(err != NULL)
    {
        const char *msg;
        const char *reported_file;
        const char *reported_function;
        pid_t       process_id;
        long        pid;

        msg = p101_error_get_message(err);

        if(msg == NULL)
        {
            msg = "<no message>";
        }

        reported_file     = (err->file_name == NULL) ? "?" : err->file_name;
        reported_function = (err->function_name == NULL) ? "?" : err->function_name;
        process_id        = getpid();
        pid               = (long)process_id;

        if(err->type == P101_ERROR_ERRNO)
        {
            /* NOLINTNEXTLINE(cert-err33-c) */
            fprintf(stderr, "ERROR (pid=%ld): %s : %s : @ %d : (errno = %d) : %s\n", pid, reported_file, reported_function, err->line_number, err->errno_code, msg);
        }
        else
        {
            /* NOLINTNEXTLINE(cert-err33-c) */
            fprintf(stderr, "ERROR (pid=%ld): %s : %s : @ %d : (error code = %d) : %s\n", pid, reported_file, reported_function, err->line_number, err->err_code, msg);
        }
    }
}

static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg)
{
    const char *src = (msg != NULL) ? msg : "<No message>";
    size_t      len;
    void       *allocation;
    char       *dup;

    len        = strlen(src);
    allocation = malloc(len + 1);
    dup        = (char *)allocation;

    if(dup == NULL)
    {
        /* Never retain a caller-owned pointer on the allocation-failure path:
         * it may refer to a stack buffer that dies before the error is read. */
        setup_error_no_dup(err, type, file_name, function_name, line_number, "out of memory while recording error");
    }
    else
    {
        memcpy(dup, src, len + 1);
        free_heap_message(err);

        err->type          = type;
        err->file_name     = file_name;
        err->function_name = function_name;
        err->line_number   = line_number;
        err->const_message = NULL;
        err->message       = dup;
    }
}

static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg)
{
    free_heap_message(err);

    err->type          = type;
    err->file_name     = file_name;
    err->function_name = function_name;
    err->line_number   = line_number;
    err->message       = NULL;
    err->const_message = (msg != NULL) ? msg : "<No message>";
}

/* Takes ownership of msg, which must be a heap allocation. */
static void setup_error_owned(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, char *msg)
{
    free_heap_message(err);

    err->type          = type;
    err->file_name     = file_name;
    err->function_name = function_name;
    err->line_number   = line_number;
    err->message       = msg;
    err->const_message = NULL;
}

void p101_error_check(struct p101_error *err, const char *file_name, const char *function_name, int line_number)
{
    if(err != NULL)
    {
        setup_error(err, P101_ERROR_CHECK, file_name, function_name, line_number, "failed check");
        err->errno_code = -1;

        if(err->reporter)
        {
            err->reporter(err);
        }
    }
}

void p101_error_errno(struct p101_error *err, const char *file_name, const char *function_name, int line_number, errno_t err_code)
{
    if(err != NULL)
    {
        char message[P101_ERROR_MESSAGE_BUFFER_SIZE];
        int  result;

        /*
         * strerror_r has two incompatible spellings. XSI returns int and
         * always writes the buffer; the GNU variant returns char * and may
         * return static storage, leaving the buffer untouched. This library
         * builds with _XOPEN_SOURCE, but the p101 fact analyzers parse these
         * sources with _GNU_SOURCE on Linux, so both must compile.
         */
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
        {
            const char *gnu_message;

            gnu_message = strerror_r(err_code, message, sizeof(message));
            if(gnu_message == NULL)
            {
                result = EINVAL;
            }
            else
            {
                result = 0;
                if(gnu_message != message)
                {
                    size_t length;

                    length = strlen(gnu_message);
                    if(length >= sizeof(message))
                    {
                        length = sizeof(message) - 1U;
                    }
                    memcpy(message, gnu_message, length);
                    message[length] = '\0';
                }
            }
        }
#else
        result = strerror_r(err_code, message, sizeof(message));
#endif
        if(result != 0)
        {
            const char *static_msg;
            if(result == EINVAL)
            {
                static_msg = "bad errno";
            }
            else if(result == ERANGE)
            {
                static_msg = "out of memory";
            }
            else
            {
                static_msg = "unknown error";
            }
            setup_error_no_dup(err, P101_ERROR_ERRNO, file_name, function_name, line_number, static_msg);
            err->errno_code = err_code; /* best effort */
        }
        else
        {
            setup_error(err, P101_ERROR_ERRNO, file_name, function_name, line_number, message);
            err->errno_code = err_code;
        }

        if(err->reporter)
        {
            err->reporter(err);
        }
    }
}

void p101_error_system(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code)
{
    if(err != NULL)
    {
        setup_error(err, P101_ERROR_SYSTEM, file_name, function_name, line_number, msg);
        err->err_code = err_code;

        if(err->reporter)
        {
            err->reporter(err);
        }
    }
}

void p101_error_user(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code)
{
    if(err != NULL)
    {
        setup_error(err, P101_ERROR_USER, file_name, function_name, line_number, msg);
        err->err_code = err_code;

        if(err->reporter)
        {
            err->reporter(err);
        }
    }
}

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

void p101_error_user_printf(struct p101_error *err, const char *file_name, const char *function_name, int line_number, int err_code, const char *fmt, ...)
{
    va_list args;
    va_list args_copy;
    int     needed;
    void   *allocation;
    char   *buf;

    if(err == NULL)
    {
        goto p101_single_exit_;
    }

    if(fmt == NULL)
    {
        p101_error_user(err, file_name, function_name, line_number, NULL, err_code);
        goto p101_single_exit_;
    }

    va_start(args, fmt);
    va_copy(args_copy, args);
    needed = vsnprintf(NULL, 0, fmt, args);    // NOLINT(cert-err33-c)
    va_end(args);

    if(needed < 0)
    {
        /* Formatting failed; fall back to the raw format string as the message. */
        va_end(args_copy);
        p101_error_user(err, file_name, function_name, line_number, fmt, err_code);
        goto p101_single_exit_;
    }

    allocation = malloc((size_t)needed + 1);
    buf        = (char *)allocation;

    if(buf == NULL)
    {
        /* Out of memory; fall back to the raw format string as the message. */
        va_end(args_copy);
        p101_error_user(err, file_name, function_name, line_number, fmt, err_code);
        goto p101_single_exit_;
    }

    vsnprintf(buf, (size_t)needed + 1, fmt, args_copy);    // NOLINT(cert-err33-c)
    va_end(args_copy);
    setup_error_owned(err, P101_ERROR_USER, file_name, function_name, line_number, buf);
    err->err_code = err_code;

    if(err->reporter)
    {
        err->reporter(err);
    }

p101_single_exit_:
    return;
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

bool p101_error_has_error(const struct p101_error *err)
{
    return (err != NULL && err->type != P101_ERROR_NONE) != 0;
}

bool p101_error_has_no_error(const struct p101_error *err)
{
    return (err == NULL || err->type == P101_ERROR_NONE) != 0;
}

bool p101_error_is_errno(const struct p101_error *err, errno_t code)
{
    return (err != NULL && err->type == P101_ERROR_ERRNO && err->errno_code == code) != 0;
}

p101_error_type p101_error_get_type(const struct p101_error *err)
{
    p101_error_type p101_single_result_;
    if(err == NULL)
    {
        p101_single_result_ = P101_ERROR_NONE;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->type;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

int p101_error_get_code(const struct p101_error *err)
{
    int p101_single_result_;
    if(err == NULL || err->type == P101_ERROR_NONE)
    {
        p101_single_result_ = 0;
        goto p101_single_exit_;
    }

    if(err->type == P101_ERROR_ERRNO)
    {
        p101_single_result_ = err->errno_code;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->err_code;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

errno_t p101_error_get_errno(const struct p101_error *err)
{
    errno_t p101_single_result_;
    if(err == NULL || err->type != P101_ERROR_ERRNO)
    {
        p101_single_result_ = 0;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->errno_code;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

const char *p101_error_get_file_name(const struct p101_error *err)
{
    const char *p101_single_result_;
    if(err == NULL)
    {
        p101_single_result_ = NULL;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->file_name;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

const char *p101_error_get_function_name(const struct p101_error *err)
{
    const char *p101_single_result_;
    if(err == NULL)
    {
        p101_single_result_ = NULL;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->function_name;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

int p101_error_get_line_number(const struct p101_error *err)
{
    int p101_single_result_;
    if(err == NULL)
    {
        p101_single_result_ = 0;
        goto p101_single_exit_;
    }

    p101_single_result_ = err->line_number;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

/* Older name for p101_error_get_errno(); kept for compatibility. */
errno_t p101_errno_get_errno(const struct p101_error *err)
{
    errno_t result;

    result = p101_error_get_errno(err);
    return result;
}

bool p101_error_is_error(const struct p101_error *err, p101_error_type type, int code)
{
    int  actual_code;
    bool result;

    actual_code = p101_error_get_code(err);
    result      = (err != NULL && type != P101_ERROR_NONE && err->type == type && actual_code == code) != 0;
    return result;
}

/* Copy and move. Heap messages are copied; immutable emergency fallbacks are
 * safely shared. */

bool p101_error_copy(struct p101_error *dst, const struct p101_error *src)
{
    bool                p101_single_result_;
    bool                source_has_error;
    char               *new_msg;
    p101_error_reporter dst_reporter;

    if(dst == NULL)
    {
        p101_single_result_ = false;
        goto p101_single_exit_;
    }
    source_has_error = p101_error_has_error(src);
    if(src == NULL || !source_has_error)
    {
        p101_error_reset(dst);
        p101_single_result_ = true;
        goto p101_single_exit_;
    }
    if(dst == src)
    {
        p101_single_result_ = true;
        goto p101_single_exit_;
    }

    /* Prepare new message first, so dst remains unchanged on OOM. */
    new_msg = NULL;

    if(src->message)
    {
        size_t len = strlen(src->message);
        void  *allocation;

        allocation = malloc(len + 1);
        new_msg    = (char *)allocation;
        if(new_msg == NULL)
        {
            p101_single_result_ = false;
            goto p101_single_exit_;
        }
        memcpy(new_msg, src->message, len + 1);
    }

    /* Preserve destination's reporter configuration. */
    dst_reporter = dst->reporter;

    /* Clear any existing heap message to avoid leaks, then copy fields. */
    free_heap_message(dst);

    dst->type          = src->type;
    dst->file_name     = src->file_name;
    dst->function_name = src->function_name;
    dst->line_number   = src->line_number;
    if(src->type == P101_ERROR_ERRNO)
    {
        dst->errno_code = src->errno_code;
    }
    else
    {
        dst->err_code = src->err_code;
    }

    if(new_msg)
    {
        dst->message       = new_msg; /* dst now owns this */
        dst->const_message = NULL;
    }
    else
    {
        dst->message       = NULL;
        dst->const_message = src->const_message; /* alias, non-owned */
    }

    dst->reporter       = dst_reporter;
    p101_single_result_ = true;
    goto p101_single_exit_;

p101_single_exit_:
    return p101_single_result_;
}

void p101_error_move(struct p101_error *dst, struct p101_error *src)
{
    p101_error_reporter dst_reporter;

    if(dst != NULL && src != NULL && dst != src)
    {
        dst_reporter = dst->reporter;

        /* Drop any existing heap message owned by dst. */
        free_heap_message(dst);

        /* Transfer scalars and pointers. */
        dst->type          = src->type;
        dst->file_name     = src->file_name;
        dst->function_name = src->function_name;
        dst->line_number   = src->line_number;
        if(src->type == P101_ERROR_ERRNO)
        {
            dst->errno_code = src->errno_code;
        }
        else
        {
            dst->err_code = src->err_code;
        }
        dst->message       = src->message;       /* take ownership */
        dst->const_message = src->const_message; /* alias */
        dst->reporter      = dst_reporter;       /* keep dst reporting policy */

        /* Reset source to no error and relinquish ownership. */
        src->type          = P101_ERROR_NONE;
        src->file_name     = NULL;
        src->function_name = NULL;
        src->line_number   = 0;
        src->message       = NULL;
        src->const_message = NULL;
        /* union value is irrelevant when type == NONE */
    }
}
