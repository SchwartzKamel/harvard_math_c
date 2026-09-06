#include <harvard_math/stats.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

static hm_status validate_values(const double *values, size_t count, double *out)
{
    size_t i;

    if (out == NULL || (count > 0 && values == NULL)) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    for (i = 0; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
    }
    return HM_STATUS_OK;
}

static hm_status calculate_variance(const double *values, size_t count, int sample, double *out)
{
    hm_status status;
    long double mean = 0.0L;
    long double sum_squared_deltas = 0.0L;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (sample && count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }

    /* Welford's update avoids forming large sums of nearly equal squares. */
    for (i = 0; i < count; ++i) {
        long double value = (long double)values[i];
        long double delta = value - mean;
        mean += delta / (long double)(i + 1);
        sum_squared_deltas += delta * (value - mean);
    }
    if (sum_squared_deltas < 0.0L) {
        sum_squared_deltas = 0.0L;
    }
    *out = (double)(sum_squared_deltas / (long double)(sample ? count - 1 : count));
    return HM_STATUS_OK;
}

static int compare_doubles(const void *left, const void *right)
{
    const double a = *(const double *)left;
    const double b = *(const double *)right;

    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

const char *hm_status_message(hm_status status)
{
    switch (status) {
    case HM_STATUS_OK:
        return "ok";
    case HM_STATUS_INVALID_ARGUMENT:
        return "invalid argument";
    case HM_STATUS_EMPTY_INPUT:
        return "empty input";
    case HM_STATUS_INSUFFICIENT_DATA:
        return "insufficient data";
    case HM_STATUS_NONFINITE_INPUT:
        return "non-finite input";
    case HM_STATUS_ALLOCATION_FAILED:
        return "allocation failed";
    default:
        return "unknown status";
    }
}

hm_status hm_sum(const double *values, size_t count, double *out)
{
    hm_status status;
    long double sum = 0.0L;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    for (i = 0; i < count; ++i) {
        sum += (long double)values[i];
    }
    *out = (double)sum;
    return HM_STATUS_OK;
}

hm_status hm_mean(const double *values, size_t count, double *out)
{
    hm_status status;
    long double mean = 0.0L;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    for (i = 0; i < count; ++i) {
        long double value = (long double)values[i];
        mean += (value - mean) / (long double)(i + 1);
    }
    *out = (double)mean;
    return HM_STATUS_OK;
}

hm_status hm_min(const double *values, size_t count, double *out)
{
    hm_status status;
    double result;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    result = values[0];
    for (i = 1; i < count; ++i) {
        if (values[i] < result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_max(const double *values, size_t count, double *out)
{
    hm_status status;
    double result;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    result = values[0];
    for (i = 1; i < count; ++i) {
        if (values[i] > result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_range(const double *values, size_t count, double *out)
{
    hm_status status;
    double minimum;
    double maximum;
    size_t i;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    minimum = values[0];
    maximum = values[0];
    for (i = 1; i < count; ++i) {
        if (values[i] < minimum) {
            minimum = values[i];
        }
        if (values[i] > maximum) {
            maximum = values[i];
        }
    }
    *out = maximum - minimum;
    return HM_STATUS_OK;
}

hm_status hm_median(const double *values, size_t count, double *out)
{
    hm_status status;
    double *sorted;
    size_t middle;

    status = validate_values(values, count, out);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count > (size_t)-1 / sizeof *sorted) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    sorted = malloc(count * sizeof *sorted);
    if (sorted == NULL) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    memcpy(sorted, values, count * sizeof *sorted);
    qsort(sorted, count, sizeof *sorted, compare_doubles);

    middle = count / 2;
    if (count % 2 == 0) {
        *out = sorted[middle - 1] / 2.0 + sorted[middle] / 2.0;
    } else {
        *out = sorted[middle];
    }
    free(sorted);
    return HM_STATUS_OK;
}

hm_status hm_variance_population(const double *values, size_t count, double *out)
{
    return calculate_variance(values, count, 0, out);
}

hm_status hm_variance_sample(const double *values, size_t count, double *out)
{
    return calculate_variance(values, count, 1, out);
}

static hm_status calculate_stddev(const double *values, size_t count, int sample, double *out)
{
    hm_status status = calculate_variance(values, count, sample, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = sqrt(*out);
    return HM_STATUS_OK;
}

hm_status hm_stddev_population(const double *values, size_t count, double *out)
{
    return calculate_stddev(values, count, 0, out);
}

hm_status hm_stddev_sample(const double *values, size_t count, double *out)
{
    return calculate_stddev(values, count, 1, out);
}
