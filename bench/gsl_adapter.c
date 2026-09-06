#include "gsl_adapter.h"

#include <gsl/gsl_statistics_double.h>

#include <math.h>
#include <stdlib.h>

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

hm_status hm_gsl_mean(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = gsl_stats_mean(values, 1, count);
    return HM_STATUS_OK;
}

hm_status hm_gsl_min(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = gsl_stats_min(values, 1, count);
    return HM_STATUS_OK;
}

hm_status hm_gsl_max(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = gsl_stats_max(values, 1, count);
    return HM_STATUS_OK;
}

hm_status hm_gsl_range(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double minimum;
    double maximum;

    if (status != HM_STATUS_OK) {
        return status;
    }
    gsl_stats_minmax(&minimum, &maximum, values, 1, count);
    *out = maximum - minimum;
    return HM_STATUS_OK;
}

hm_status hm_gsl_median(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);
    double *copy;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count > (size_t)-1 / sizeof *copy) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    copy = malloc(count * sizeof *copy);
    if (copy == NULL) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    for (size_t i = 0; i < count; ++i) {
        copy[i] = values[i];
    }
    *out = gsl_stats_median(copy, 1, count);
    free(copy);
    return HM_STATUS_OK;
}

hm_status hm_gsl_variance_sample(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    *out = gsl_stats_variance(values, 1, count);
    return HM_STATUS_OK;
}

hm_status hm_gsl_stddev_sample(const double *values, size_t count, double *out)
{
    hm_status status = validate_values(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    *out = gsl_stats_sd(values, 1, count);
    return HM_STATUS_OK;
}

static hm_status validate_pair_values(const double *x, const double *y, size_t count)
{
    if (x == NULL || y == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(x[i]) || !isfinite(y[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
    }
    return HM_STATUS_OK;
}

hm_status hm_gsl_covariance_sample(const double *x, const double *y, size_t count, double *out)
{
    hm_status status;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    status = validate_pair_values(x, y, count);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    *out = gsl_stats_covariance(x, 1, y, 1, count);
    return HM_STATUS_OK;
}

hm_status hm_gsl_covariance_population(const double *x, const double *y, size_t count, double *out)
{
    hm_status status = hm_gsl_covariance_sample(x, y, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out *= (double)(count - 1) / (double)count;
    return HM_STATUS_OK;
}
