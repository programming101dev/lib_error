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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct p101_error
{
    const char     *const_message; /* non-owned, static or caller-managed */
    char           *message;       /* owned heap copy, if present */
    const char     *file_name;     /* non-owned */
    const char     *function_name; /* non-owned */
    int             line_number;
    p101_error_type type;
    void (*reporter)(const struct p101_error *err);

    union
    {
        errno_t errno_code;
        int     err_code;
    };
};

static void error_init(struct p101_error *err, void (*reporter)(const struct p101_error *err));
static void free_heap_message(struct p101_error *err);
static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg);
static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg);

struct p101_error *p101_error_create(bool report)
{
    struct p101_error *err = (struct p101_error *)malloc(sizeof *err);
    if(err != NULL)
    {
        void (*reporter)(const struct p101_error *err) = NULL;

        if(report)
        {
            reporter = p101_error_default_error_reporter;
        }
        error_init(err, reporter);
    }
    return err;
}

static void error_init(struct p101_error *err, void (*reporter)(const struct p101_error *err))
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
    void (*reporter)(const struct p101_error *err) = err->reporter;
    free_heap_message(err);
    error_init(err, reporter);
}

bool p101_error_is_reporting(const struct p101_error *err)
{
    return err->reporter != NULL;
}

void p101_error_set_reporting(struct p101_error *err, bool on)
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

const char *p101_error_get_message(const struct p101_error *err)
{
    if(err->message)
    {
        return err->message;
    }
    if(err->const_message)
    {
        return err->const_message;
    }
    return NULL;
}

void p101_error_default_error_reporter(const struct p101_error *err)
{
    const char *msg = err->const_message ? err->const_message : err->message;
    int         pid;

    if(msg == NULL)
    {
        msg = "<no message>";
    }

    pid = (int)getpid();

    if(err->type == P101_ERROR_ERRNO)
    {
        /* NOLINTNEXTLINE(cert-err33-c) */
        fprintf(stderr, "ERROR (pid=%d): %s : %s : @ %d : (errno = %d) : %s\n", pid, err->file_name, err->function_name, err->line_number, err->errno_code, msg);
    }
    else
    {
        /* NOLINTNEXTLINE(cert-err33-c) */
        fprintf(stderr, "ERROR (pid=%d): %s : %s : @ %d : (error code = %d) : %s\n", pid, err->file_name, err->function_name, err->line_number, err->err_code, msg);
    }
}

static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg)
{
    const char *src = (msg != NULL) ? msg : "<No message>";
    size_t      len;
    char       *dup;

    len = strlen(src);
    dup = (char *)malloc(len + 1);

    if(dup == NULL)
    {
        /* Fall back to const message path on OOM. */
        setup_error_no_dup(err, type, file_name, function_name, line_number, src);
        return;
    }

    memcpy(dup, src, len + 1);

    err->type          = type;
    err->file_name     = file_name;
    err->function_name = function_name;
    err->line_number   = line_number;
    err->const_message = NULL;
    err->message       = dup;
}

static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, int line_number, const char *msg)
{
    err->type          = type;
    err->file_name     = file_name;
    err->function_name = function_name;
    err->line_number   = line_number;
    err->message       = NULL;
    err->const_message = (msg != NULL) ? msg : "<No message>";
}

void p101_error_check(struct p101_error *err, const char *file_name, const char *function_name, int line_number)
{
    setup_error(err, P101_ERROR_CHECK, file_name, function_name, line_number, "failed check");
    err->errno_code = -1;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

void p101_error_errno(struct p101_error *err, const char *file_name, const char *function_name, int line_number, errno_t err_code)
{
    /* POSIX strerror is not thread-safe; keep behavior aligned with existing code base. */
    const char *msg = strerror(err_code); /* NOLINT(concurrency-mt-unsafe) */

    if(msg == NULL)
    {
        const char *static_msg;
        if(errno == EINVAL)
        {
            static_msg = "bad errno";
        }
        else if(errno == ERANGE)
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
        setup_error(err, P101_ERROR_ERRNO, file_name, function_name, line_number, msg);
        err->errno_code = err_code;
    }

    if(err->reporter)
    {
        err->reporter(err);
    }
}

void p101_error_system(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code)
{
    setup_error(err, P101_ERROR_SYSTEM, file_name, function_name, line_number, msg);
    err->err_code = err_code;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

void p101_error_user(struct p101_error *err, const char *file_name, const char *function_name, int line_number, const char *msg, int err_code)
{
    setup_error(err, P101_ERROR_USER, file_name, function_name, line_number, msg);
    err->err_code = err_code;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

bool p101_error_has_error(const struct p101_error *err)
{
    return err->type != P101_ERROR_NONE;
}

bool p101_error_has_no_error(const struct p101_error *err)
{
    return err->type == P101_ERROR_NONE;
}

bool p101_error_is_errno(const struct p101_error *err, errno_t code)
{
    return (((err->type == P101_ERROR_ERRNO) && (err->errno_code == code)) != 0);
}

errno_t p101_errno_get_errno(const struct p101_error *err)
{
    return err->errno_code;
}

bool p101_error_is_error(const struct p101_error *err, p101_error_type type, int code)
{
    return (((err->type == type) && (err->err_code == code)) != 0);
}

/* New: deep copy and move */

bool p101_error_copy(struct p101_error *dst, const struct p101_error *src)
{
    char *new_msg;
    void (*dst_reporter)(const struct p101_error *err);

    if(dst == NULL)
    {
        return false;
    }
    if(src == NULL || !p101_error_has_error(src))
    {
        p101_error_reset(dst);
        return true;
    }
    if(dst == src)
    {
        return true;
    }

    /* Prepare new message first, so dst remains unchanged on OOM. */
    new_msg = NULL;

    if(src->message)
    {
        size_t len = strlen(src->message);
        new_msg    = (char *)malloc(len + 1);
        if(new_msg == NULL)
        {
            return false;
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

    dst->reporter = dst_reporter;
    return true;
}

void p101_error_move(struct p101_error *dst, struct p101_error *src)
{
    void (*dst_reporter)(const struct p101_error *err);

    if(dst == NULL || src == NULL || dst == src)
    {
        return;
    }

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
