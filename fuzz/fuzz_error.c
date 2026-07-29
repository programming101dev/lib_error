#include "p101_error/check.h"
#include "p101_error/error.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void require(int condition)
{
    if(!condition)
    {
        abort();
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct p101_error *copy;
    struct p101_error *err;
    char              *message;
    int                code;

    if(size > 65536U)
    {
        return 0;
    }

    message = (char *)malloc(size + 1U);
    if(message == NULL)
    {
        return 0;
    }
    memcpy(message, data, size);
    message[size] = '\0';

    err  = p101_error_create(false);
    copy = p101_error_create(false);
    if(err == NULL || copy == NULL)
    {
        p101_error_destroy(copy);
        p101_error_destroy(err);
        free(message);
        return 0;
    }

    code = size == 0U ? 0 : (int)data[0];
    p101_error_user(err, "fuzz.c", "LLVMFuzzerTestOneInput", 1, message, code);
    require(p101_error_has_error(err));
    require(p101_error_is_error(err, P101_ERROR_USER, code));
    require(strcmp(p101_error_get_message(err), message) == 0);
    require(p101_error_copy(copy, err));
    require(strcmp(p101_error_get_message(copy), message) == 0);

    p101_error_reset(err);
    require(!p101_error_is_error(err, P101_ERROR_NONE, 0));
    require(P101_CHECK_IN_RANGE(err, (long double)size, 0.0L, 65536.0L));

    p101_error_destroy(copy);
    p101_error_destroy(err);
    free(message);
    return 0;
}
