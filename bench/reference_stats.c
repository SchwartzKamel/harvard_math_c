#include "reference_stats.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static hm_status validate_values(const double *values, size_t count, double *out)
{
    if (out == NULL || (count > 0 && values == NULL)) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
    }
    return HM_STATUS_OK;
}

hm_status hm_reference_sum(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double sum = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    for (size_t i = 0; i < count; ++i) {
        sum += values[i];
    }
    *out = sum;
    return HM_STATUS_OK;
}

hm_status hm_reference_mean(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double sum = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    for (size_t i = 0; i < count; ++i) {
        sum += values[i];
    }
    *out = sum / (double)count;
    return HM_STATUS_OK;
}

hm_status hm_reference_min(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double result;

    if (status != HM_STATUS_OK) {
        return status;
    }
    result = values[0];
    for (size_t i = 1; i < count; ++i) {
        if (values[i] < result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_reference_max(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double result;

    if (status != HM_STATUS_OK) {
        return status;
    }
    result = values[0];
    for (size_t i = 1; i < count; ++i) {
        if (values[i] > result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_reference_range(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double minimum;
    double maximum;

    if (status != HM_STATUS_OK) {
        return status;
    }
    minimum = values[0];
    maximum = values[0];
    for (size_t i = 1; i < count; ++i) {
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

static int compare_doubles(const void *left, const void *right)
{
    const double a = *(const double *)left;
    const double b = *(const double *)right;

    return (a > b) - (a < b);
}

hm_status hm_reference_median(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double *sorted;
    size_t middle;

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

static hm_status reference_variance(const double *values, size_t count, int sample, double *out)
{
    hm_status status = validate_values(values, count, out);
    double mean = 0.0;
    double sum_squared_differences = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (sample && count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    for (size_t i = 0; i < count; ++i) {
        mean += values[i];
    }
    mean /= (double)count;
    for (size_t i = 0; i < count; ++i) {
        const double difference = values[i] - mean;
        sum_squared_differences += difference * difference;
    }
    *out = sum_squared_differences / (double)(sample ? count - 1 : count);
    return HM_STATUS_OK;
}

hm_status hm_reference_variance_population(const double *values, size_t count, double *out)
{
    return reference_variance(values, count, 0, out);
}

hm_status hm_reference_variance_sample(const double *values, size_t count, double *out)
{
    return reference_variance(values, count, 1, out);
}

static hm_status reference_stddev(const double *values, size_t count, int sample, double *out)
{
    hm_status status = reference_variance(values, count, sample, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = sqrt(*out);
    return HM_STATUS_OK;
}

hm_status hm_reference_stddev_population(const double *values, size_t count, double *out)
{
    return reference_stddev(values, count, 0, out);
}

hm_status hm_reference_stddev_sample(const double *values, size_t count, double *out)
{
    return reference_stddev(values, count, 1, out);
}
