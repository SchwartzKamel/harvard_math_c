#include <harvard_math/regression.h>

#include <math.h>

static hm_status validate_arguments(const double *x, const double *y, size_t count, double *out)
{
    if (out == NULL || (count > 0 && (x == NULL || y == NULL))) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    return HM_STATUS_OK;
}

static hm_status calculate_slope_extended(const double *x, const double *y,
                                          size_t count, double *out)
{
    long double mean_x = 0.0L;
    long double mean_y = 0.0L;
    long double numerator = 0.0L;
    long double denominator = 0.0L;

    for (size_t i = 0; i < count; ++i) {
        const double x_value = x[i];
        const double y_value = y[i];
        long double x_delta;
        long double y_delta;

        if (!isfinite(x_value) || !isfinite(y_value)) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        x_delta = (long double)x_value - mean_x;
        y_delta = (long double)y_value - mean_y;
        mean_x += x_delta / (long double)(i + 1);
        mean_y += y_delta / (long double)(i + 1);
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    for (size_t i = 0; i < count; ++i) {
        const long double x_delta = (long double)x[i] - mean_x;
        const long double y_delta = (long double)y[i] - mean_y;

        numerator += x_delta * y_delta;
        denominator += x_delta * x_delta;
    }
    if (denominator == 0.0L) {
        return HM_STATUS_UNDEFINED;
    }
    *out = (double)(numerator / denominator);
    if (!isfinite(*out)) {
        return HM_STATUS_UNDEFINED;
    }
    return HM_STATUS_OK;
}

hm_status hm_slope(const double *x, const double *y, size_t count, double *out)
{
    hm_status status = validate_arguments(x, y, count, out);
    double center_x;
    double center_y;
    double sum_x_deltas = 0.0;
    double sum_y_deltas = 0.0;
    double mean_x_delta;
    double mean_y_delta;
    double numerator = 0.0;
    double denominator = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    center_x = x[0];
    center_y = y[0];

    /* Centering keeps common offsets out of the fast double arithmetic. */
    for (size_t i = 0; i < count; ++i) {
        const double x_value = x[i];
        const double y_value = y[i];
        const double x_delta = x_value - center_x;
        const double y_delta = y_value - center_y;

        if (!isfinite(x_value) || !isfinite(y_value)) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        if (!isfinite(x_delta) || !isfinite(y_delta)) {
            return calculate_slope_extended(x, y, count, out);
        }
        sum_x_deltas += x_delta;
        sum_y_deltas += y_delta;
        if (!isfinite(sum_x_deltas) || !isfinite(sum_y_deltas)) {
            return calculate_slope_extended(x, y, count, out);
        }
    }
    mean_x_delta = sum_x_deltas / (double)count;
    mean_y_delta = sum_y_deltas / (double)count;
    for (size_t i = 0; i < count; ++i) {
        const double x_delta = (x[i] - center_x) - mean_x_delta;
        const double y_delta = (y[i] - center_y) - mean_y_delta;

        numerator += x_delta * y_delta;
        denominator += x_delta * x_delta;
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    if (denominator == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    *out = numerator / denominator;
    if (!isfinite(*out)) {
        return HM_STATUS_UNDEFINED;
    }
    return HM_STATUS_OK;
}
