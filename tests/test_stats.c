#include <float.h>
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
    const double odd_values[] = {3.0, 1.0, 2.0};
    const double single[] = {7.0};
    const double offset_values[] = {1e16, 1e16 + 2.0, 1e16 + 4.0, 1e16 + 6.0};
    const double nonfinite[] = {1.0, NAN};
    const double overflow_then_nonfinite[] = {DBL_MAX, -DBL_MAX, NAN};
    double sorted_values[33];
    double reverse_values[33];
    double organ_pipe_values[33];
    double result;
    hm_statistic_fn statistic = custom_midrange;

    for (size_t i = 0; i < 33; ++i) {
        sorted_values[i] = (double)i;
        reverse_values[i] = (double)(32 - i);
        organ_pipe_values[i] = (double)(i < 17 ? i : 32 - i);
    }

    /* These checks prove the public status strings remain useful to callers. */
    CHECK(strcmp(hm_status_message(HM_STATUS_OK), "ok") == 0);
    CHECK(strcmp(hm_status_message(HM_STATUS_NONFINITE_INPUT), "non-finite input") == 0);
    CHECK(strcmp(hm_status_message((hm_status)-1), "unknown status") == 0);

    /* These checks prove each initial statistic uses the intended formula. */
    CHECK(hm_sum(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 18.0));
    CHECK(hm_mean(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 4.5));
    CHECK(hm_min(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 2.0));
    CHECK(hm_max(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 8.0));
    CHECK(hm_range(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 6.0));
    CHECK(hm_median(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 4.0));
    CHECK(memcmp(values, original, sizeof values) == 0);
    CHECK(hm_median(odd_values, 3, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 2.0));
    /* These checks cover sorted, reverse-sorted, and pivot-adversarial median inputs. */
    CHECK(hm_median(sorted_values, 33, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 16.0));
    CHECK(hm_median(reverse_values, 33, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 16.0));
    CHECK(hm_median(organ_pipe_values, 33, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 8.0));
    /* This check proves the selection path also leaves caller storage untouched. */
    for (size_t i = 0; i < 33; ++i) {
        CHECK(sorted_values[i] == (double)i);
        CHECK(reverse_values[i] == (double)(32 - i));
        CHECK(organ_pipe_values[i] == (double)(i < 17 ? i : 32 - i));
    }
    CHECK(hm_variance_population(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 4.75));
    CHECK(hm_variance_sample(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 19.0 / 3.0));
    CHECK(hm_stddev_population(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, sqrt(4.75)));
    CHECK(hm_stddev_sample(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, sqrt(19.0 / 3.0)));

    /* This check proves centered variance remains accurate for large common offsets. */
    CHECK(hm_variance_sample(offset_values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 20.0 / 3.0));

    /* These checks prove population accepts one value while sample needs two. */
    CHECK(hm_variance_population(single, 1, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 0.0));
    CHECK(hm_variance_sample(single, 1, &result) == HM_STATUS_INSUFFICIENT_DATA);

    /* This check proves a user function matching hm_statistic_fn works like a built-in. */
    CHECK(statistic(values, 4, &result) == HM_STATUS_OK);
    CHECK(close_enough(result, 5.0));

    /* These checks prove invalid input is rejected before it can be dereferenced. */
    CHECK(hm_mean(NULL, 4, &result) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_mean(values, 4, NULL) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_mean(NULL, 0, &result) == HM_STATUS_EMPTY_INPUT);
    CHECK(hm_mean(values, 0, &result) == HM_STATUS_EMPTY_INPUT);
    result = 123.0;
    CHECK(hm_mean(nonfinite, 2, &result) == HM_STATUS_NONFINITE_INPUT);
    CHECK(result == 123.0);
    CHECK(hm_variance_population(NULL, 4, &result) == HM_STATUS_INVALID_ARGUMENT);
    CHECK(hm_variance_population(overflow_then_nonfinite, 3, &result) == HM_STATUS_NONFINITE_INPUT);

    return 0;
}
