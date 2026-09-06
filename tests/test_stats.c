#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include <harvard_math/harvard_math.h>

static void assert_close(double actual, double expected)
{
    assert(fabs(actual - expected) < 1e-12);
}

static hm_status custom_midrange(const double *values, size_t count, double *out)
{
    double minimum;
    double maximum;
    hm_status status;

    status = hm_min(values, count, &minimum);
    if (status != HM_STATUS_OK) {
        return status;
    }
    status = hm_max(values, count, &maximum);
    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = minimum / 2.0 + maximum / 2.0;
    return HM_STATUS_OK;
}

int main(void)
{
    const double values[] = {8.0, 2.0, 4.0, 4.0};
    const double original[] = {8.0, 2.0, 4.0, 4.0};
    const double single[] = {7.0};
    const double nonfinite[] = {1.0, NAN};
    double result;
    hm_statistic_fn statistic = custom_midrange;

    /* These assertions prove the public status strings remain useful to callers. */
    assert(strcmp(hm_status_message(HM_STATUS_OK), "ok") == 0);
    assert(strcmp(hm_status_message(HM_STATUS_NONFINITE_INPUT), "non-finite input") == 0);
    assert(strcmp(hm_status_message((hm_status)-1), "unknown status") == 0);

    /* These assertions prove each initial statistic uses the intended formula. */
    assert(hm_sum(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 18.0);
    assert(hm_mean(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 4.5);
    assert(hm_min(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 2.0);
    assert(hm_max(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 8.0);
    assert(hm_range(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 6.0);
    assert(hm_median(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 4.0);
    assert(memcmp(values, original, sizeof values) == 0);
    assert(hm_variance_population(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 4.75);
    assert(hm_variance_sample(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 19.0 / 3.0);
    assert(hm_stddev_population(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, sqrt(4.75));
    assert(hm_stddev_sample(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, sqrt(19.0 / 3.0));

    /* This proves population statistics accept one value while sample statistics do not. */
    assert(hm_variance_population(single, 1, &result) == HM_STATUS_OK);
    assert_close(result, 0.0);
    assert(hm_variance_sample(single, 1, &result) == HM_STATUS_INSUFFICIENT_DATA);

    /* This proves a user function matching hm_statistic_fn can be called like a built-in. */
    assert(statistic(values, 4, &result) == HM_STATUS_OK);
    assert_close(result, 5.0);

    /* These assertions prove invalid input is rejected before it can be dereferenced. */
    assert(hm_mean(NULL, 4, &result) == HM_STATUS_INVALID_ARGUMENT);
    assert(hm_mean(values, 4, NULL) == HM_STATUS_INVALID_ARGUMENT);
    assert(hm_mean(NULL, 0, &result) == HM_STATUS_EMPTY_INPUT);
    assert(hm_mean(values, 0, &result) == HM_STATUS_EMPTY_INPUT);
    assert(hm_mean(nonfinite, 2, &result) == HM_STATUS_NONFINITE_INPUT);

    return 0;
}
