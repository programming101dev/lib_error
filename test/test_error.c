#include "p101_error/check.h"
#include "p101_error/error.h"
#include <errno.h>
#include <pthread.h>
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

struct errno_thread
{
    errno_t code;
    char    expected[128];
    int     failed;
};

static void *raise_errno_repeatedly(void *context)
{
    struct errno_thread *thread;

    thread = (struct errno_thread *)context;
    for(size_t index = 0U; index < 10000U; index++)
    {
        struct p101_error *err;

        err = p101_error_create(false);
        if(err == NULL)
        {
            thread->failed = 1;
            break;
        }
        p101_error_errno(err, __FILE__, __func__, __LINE__, thread->code);
        if(!p101_error_is_errno(err, thread->code) || p101_error_get_message(err) == NULL || strcmp(p101_error_get_message(err), thread->expected) != 0)
        {
            thread->failed = 1;
        }
        p101_error_destroy(err);
    }
    return NULL;
}

static void test_concurrent_errno_messages(void)
{
    struct errno_thread contexts[] = {
        {ENOENT, "", 0},
        {EACCES, "", 0},
        {EINVAL, "", 0},
        {ERANGE, "", 0}
    };
    int       created[sizeof(contexts) / sizeof(contexts[0])] = {0};
    pthread_t threads[sizeof(contexts) / sizeof(contexts[0])];

    for(size_t index = 0U; index < sizeof(threads) / sizeof(threads[0]); index++)
    {
        int result;

        /*
         * These tests build with _POSIX_C_SOURCE, so the XSI spelling (int)
         * is what runs. The GNU spelling (char *) is here only so the p101
         * fact analyzers, which parse with _GNU_SOURCE on Linux, can compile
         * this translation unit.
         */
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
        EXPECT(strerror_r(contexts[index].code, contexts[index].expected, sizeof(contexts[index].expected)) != NULL);
#else
        EXPECT(strerror_r(contexts[index].code, contexts[index].expected, sizeof(contexts[index].expected)) == 0);
#endif
        result         = pthread_create(&threads[index], NULL, raise_errno_repeatedly, &contexts[index]);
        created[index] = result == 0;
        EXPECT(result == 0);
    }
    for(size_t index = 0U; index < sizeof(threads) / sizeof(threads[0]); index++)
    {
        if(created[index] != 0)
        {
            EXPECT(pthread_join(threads[index], NULL) == 0);
            EXPECT(contexts[index].failed == 0);
        }
    }
}

int main(void)
{
    test_error_state();
    test_copy_and_move();
    test_generic_checks();
    test_concurrent_errno_messages();

    if(failures != 0)
    {
        fprintf(stderr, "%d lib_error test(s) failed\n", failures);
        return 1;
    }

    return 0;
}
