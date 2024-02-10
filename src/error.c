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

#include "p101_error/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct p101_error
{
    const char     *const_message;
    char           *message;
    const char     *file_name;
    const char     *function_name;
    size_t          line_number;
    p101_error_type type;
    void (*reporter)(const struct p101_error *err);

    union
    {
        errno_t errno_code;
        int     err_code;
    };
};

static void error_init(struct p101_error *err, void (*reporter)(const struct p101_error *err));
static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, size_t line_number, const char *msg);
static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, size_t line_number, const char *msg);

struct p101_error *p101_error_create(bool report)
{
    struct p101_error *err;

    err = (struct p101_error *)malloc(sizeof(struct p101_error));

    if(err != NULL)
    {
        void (*reporter)(const struct p101_error *err);

        if(report)
        {
            reporter = p101_error_default_error_reporter;
        }
        else
        {
            reporter = NULL;
        }

        error_init(err, reporter);
    }

    return err;
}

static void error_init(struct p101_error *err, void (*reporter)(const struct p101_error *err))
{
    memset(err, 0, sizeof(struct p101_error));
    err->reporter = reporter;
}

void p101_error_reset(struct p101_error *err)
{
    void (*reporter)(const struct p101_error *err);

    reporter = err->reporter;

    if(err->message)
    {
        free(err->message);
        err->const_message = NULL;
        err->message       = NULL;
    }

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
    const char *message;

    if(err->message)
    {
        message = err->message;
    }
    else if(err->const_message)
    {
        message = err->const_message;
    }
    else
    {
        message = NULL;
    }

    return message;
}

void p101_error_default_error_reporter(const struct p101_error *err)
{
    const char *msg;
    int         pid;    // the compiler doesn't let pid_t be used...

    if(err->const_message)
    {
        msg = err->const_message;
    }
    else
    {
        msg = err->message;
    }

    pid = getpid();

    if(err->type == P101_ERROR_ERRNO)
    {
        // NOLINTNEXTLINE(cert-err33-c)
        fprintf(stderr, "ERROR (pid=%d): %s : %s : @ %zu : (errno = %d) : %s\n", pid, err->file_name, err->function_name, err->line_number, err->errno_code, msg);
    }
    else
    {
        // NOLINTNEXTLINE(cert-err33-c)
        fprintf(stderr, "ERROR (pid=%d): %s : %s : @ %zu : (error code = %d) : %s\n", pid, err->file_name, err->function_name, err->line_number, err->err_code, msg);
    }
}

static void setup_error(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, size_t line_number, const char *msg)
{
    size_t      len;
    const char *tmp_msg;
    char       *saved_msg;

    errno = 0;

    if(msg == NULL)
    {
        tmp_msg = "<No message>";
    }
    else
    {
        tmp_msg = msg;
    }

    len       = strlen(tmp_msg);
    saved_msg = (char *)malloc(len + 1);

    if(saved_msg == NULL)
    {
        setup_error_no_dup(err, type, file_name, function_name, line_number, saved_msg);
    }
    else
    {
#ifndef __clang_analyzer__
        strcpy(saved_msg, tmp_msg);
#endif

        err->type          = type;
        err->file_name     = file_name;
        err->function_name = function_name;
        err->line_number   = line_number;
        err->const_message = NULL;
        err->message       = saved_msg;
    }
}

static void setup_error_no_dup(struct p101_error *err, p101_error_type type, const char *file_name, const char *function_name, size_t line_number, const char *msg)
{
    err->type          = type;
    err->file_name     = file_name;
    err->function_name = function_name;
    err->line_number   = line_number;
    err->message       = NULL;
    err->const_message = msg;
}

void p101_error_check(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number)
{
    const char *msg;

    // TODO: fix this, message should be passed in
    msg = "failed check";
    setup_error(err, P101_ERROR_CHECK, file_name, function_name, line_number, msg);
    err->errno_code = -1;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

void p101_error_errno(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, errno_t err_code)
{
    const char *msg;

    errno = 0;
    msg   = strerror(err_code);    // NOLINT(concurrency-mt-unsafe)

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

        setup_error_no_dup(err, P101_ERROR_ERRNO, __FILE__, __func__, __LINE__, static_msg);
        err->errno_code = errno;
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

void p101_error_system(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, const char *msg, int err_code)
{
    setup_error(err, P101_ERROR_SYSTEM, file_name, function_name, line_number, msg);
    err->err_code = err_code;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

void p101_error_user(struct p101_error *err, const char *file_name, const char *function_name, size_t line_number, const char *msg, int err_code)
{
    setup_error(err, P101_ERROR_USER, file_name, function_name, line_number, msg);
    err->err_code = err_code;

    if(err->reporter)
    {
        err->reporter(err);
    }
}

inline bool p101_error_has_error(const struct p101_error *err)
{
    return err->type != P101_ERROR_NONE;
}

inline bool p101_error_has_no_error(const struct p101_error *err)
{
    return err->type == P101_ERROR_NONE;
}

inline bool p101_error_is_errno(const struct p101_error *err, errno_t code)
{
    return (err->type == P101_ERROR_ERRNO) && (err->errno_code == code);
}

inline errno_t p101_errno_get_errno(const struct p101_error *err)
{
    return err->errno_code;
}
