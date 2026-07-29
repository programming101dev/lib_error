#include "p101_error/check.h"
#include "p101_error/error.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define EXPECT(condition)                                                                                                                                                                                                                                          \
    do                                                                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                                              \
        if(!(condition))                                                                                                                                                                                                                                           \
        {                                                                                                                                                                                                                                                          \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);                                                                                                                                                                                   \
            failures++;                                                                                                                                                                                                                                            \
        }                                                                                                                                                                                                                                                          \
    } while(0)

static void test_error_state(void)
{
    struct p101_error *err;

    err = p101_error_create(false);
    EXPECT(err != NULL);
    EXPECT(p101_error_has_no_error(err));
    EXPECT(!p101_error_is_error(err, P101_ERROR_NONE, 0));

    P101_ERROR_RAISE_ERRNO(err, ENOENT);
    EXPECT(p101_error_is_errno(err, ENOENT));
    EXPECT(p101_error_is_error(err, P101_ERROR_ERRNO, ENOENT));
    EXPECT(p101_error_get_code(err) == ENOENT);

    p101_error_reset(err);
    P101_ERROR_RAISE_USER(err, "example", 42);
    EXPECT(p101_error_is_error(err, P101_ERROR_USER, 42));
    EXPECT(strcmp(p101_error_get_message(err), "example") == 0);
    p101_error_destroy(err);
}

static void test_copy_and_move(void)
{
    struct p101_error *dst;
    struct p101_error *src;

    dst = p101_error_create(false);
    src = p101_error_create(false);
    EXPECT(dst != NULL);
    EXPECT(src != NULL);

    P101_ERROR_RAISE_USER_PRINTF(src, 7, "value=%d", 19);
    EXPECT(p101_error_copy(dst, src));
    EXPECT(p101_error_is_error(dst, P101_ERROR_USER, 7));
    EXPECT(strcmp(p101_error_get_message(dst), "value=19") == 0);

    p101_error_reset(dst);
    p101_error_move(dst, src);
    EXPECT(p101_error_is_error(dst, P101_ERROR_USER, 7));
    EXPECT(p101_error_has_no_error(src));

    p101_error_destroy(src);
    p101_error_destroy(dst);
}

static void test_generic_checks(void)
{
    struct p101_error *err;
    long double        high;
    long double        low;

    err  = p101_error_create(false);
    high = 2147483648.0L;
    low  = 2147483647.0L;

    EXPECT(P101_CHECK_GREATER_THAN(err, high, low));
    EXPECT(P101_CHECK_LESS_THAN(err, low, high));
    EXPECT(P101_CHECK_IN_RANGE(err, high, low, high));
    EXPECT(p101_error_has_no_error(err));

    EXPECT(!P101_CHECK_GREATER_THAN(err, low, high));
    EXPECT(p101_error_has_error(err));
    p101_error_destroy(err);
}

int main(void)
{
    test_error_state();
    test_copy_and_move();
    test_generic_checks();

    if(failures != 0)
    {
        fprintf(stderr, "%d lib_error test(s) failed\n", failures);
        return 1;
    }

    return 0;
}
