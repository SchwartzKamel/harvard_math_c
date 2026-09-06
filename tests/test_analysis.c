#include <math.h>
#include <stdio.h>

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
    return fabs(actual - expected) < 1e-6;
}

int main(void)
{
    hm_ztest_result result;
    hm_confidence_interval interval;

    /* These checks prove the one-sample z statistic and two-sided p-value. */
    CHECK(hm_ztest_one_sample(104.0, 100.0, 10.0, 25,
                              HM_ALTERNATIVE_TWO_SIDED, &result) == HM_STATUS_OK);
    CHECK(close_enough(result.z_score, 2.0));
    CHECK(close_enough(result.p_value, 0.0455002639));
    CHECK(hm_ztest_one_sample(104.0, 100.0, 10.0, 25,
                              HM_ALTERNATIVE_GREATER, &result) == HM_STATUS_OK);
    CHECK(close_enough(result.p_value, 0.0227501319));

    /* This proves the two-sample known-variance standard error. */
    CHECK(hm_ztest_two_sample(110.0, 10.0, 25, 100.0, 10.0, 25,
                              HM_ALTERNATIVE_GREATER, &result) == HM_STATUS_OK);
    CHECK(close_enough(result.z_score, 3.535533906));

    /* This proves the common 95% known-sigma confidence interval. */
    CHECK(hm_mean_confidence_interval(100.0, 10.0, 25, 0.95, &interval) == HM_STATUS_OK);
    CHECK(close_enough(interval.lower, 96.080072));
    CHECK(close_enough(interval.upper, 103.919928));

    /* These checks prove undefined and malformed inference inputs preserve output. */
    result.z_score = 123.0;
    result.p_value = 456.0;
    CHECK(hm_ztest_one_sample(104.0, 100.0, 0.0, 25,
                              HM_ALTERNATIVE_TWO_SIDED, &result) == HM_STATUS_UNDEFINED);
    CHECK(result.z_score == 123.0 && result.p_value == 456.0);
    CHECK(hm_ztest_one_sample(104.0, 100.0, 10.0, 25,
                              (hm_alternative)99, &result) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_ztest_one_sample(110.0, 100.0, 10.0, 0,
                              HM_ALTERNATIVE_TWO_SIDED, &result) == HM_STATUS_EMPTY_INPUT);
    CHECK(hm_mean_confidence_interval(100.0, 10.0, 25, 1.0, &interval) == HM_STATUS_INVALID_ARGUMENT);

    return 0;
}
