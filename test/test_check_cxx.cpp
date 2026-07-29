#include "p101_error/check.h"
#include "p101_error/error.h"

int main()
{
    p101_error *err;
    long double high;
    long double low;
    int         status;

    err    = p101_error_create(false);
    high   = 4.0L;
    low    = 3.0L;
    status = 0;

    if(err == nullptr || !P101_CHECK_GREATER_THAN(err, high, low) || !P101_CHECK_LESS_THAN(err, 1, 2) || !P101_CHECK_EQUALS(err, "same", "same") || !P101_CHECK_IN_RANGE(err, 2U, 1U, 3U))
    {
        status = 1;
    }

    p101_error_destroy(err);
    return status;
}
