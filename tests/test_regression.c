#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <harvard_math/harvard_math.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "check failed at %s:%d: %s\n",                    \
                    __FILE__, __LINE__, #condition);                           \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static int close_enough(double actual, double expected)
{
    return fabs(actual - expected) < 1e-12;
}

int main(void)
{
    const double x[] = {1.0, 2.0, 3.0, 4.0};
    const double y[] = {2.0, 4.0, 6.0, 8.0};
    const double x_offset[] = {1e16, 1e16 + 2.0, 1e16 + 4.0, 1e16 + 6.0};
    const double y_offset[] = {5.0, 7.0, 9.0, 11.0};
    const double constant_x[] = {3.0, 3.0, 3.0};
    const double nonfinite[] = {1.0, NAN};
    double x_copy[4];
    double y_copy[4];
    double result;

    memcpy(x_copy, x, sizeof x);
    memcpy(y_copy, y, sizeof y);

    /* This proves the ordinary least-squares formula on an exact line. */
    CHECK(hm_slope(x, y, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 2.0));
    CHECK(memcmp(x, x_copy, sizeof x) == 0);
    CHECK(memcmp(y, y_copy, sizeof y) == 0);

    /* This proves centering handles a large predictor offset. */
    CHECK(hm_slope(x_offset, y_offset, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 1.0));

    /* These checks prove mathematical and pointer boundary statuses. */
    result = 123.0;
    CHECK(hm_slope(constant_x, y, 3, &result) == HM_STATUS_UNDEFINED);
    CHECK(result == 123.0);
    CHECK(hm_slope(x, y, 1, &result) == HM_STATUS_INSUFFICIENT_DATA);
    CHECK(hm_slope(NULL, y, 4, &result) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_slope(x, y, 4, NULL) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_slope(nonfinite, y, 2, &result) == HM_STATUS_NONFINITE_INPUT);

    return 0;
}
