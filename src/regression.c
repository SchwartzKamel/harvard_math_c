#include <harvard_math/regression.h>

#include <math.h>

struct pair_summary {
    double mean_x;
    double mean_y;
    double sum_xx;
    double sum_xy;
    double sum_yy;
};

static hm_status validate_pair_arguments(const double *x, const double *y, size_t count)
{
    if (count > 0 && (x == NULL || y == NULL)) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    return HM_STATUS_OK;
}

static hm_status calculate_pair_summary_extended(const double *x, const double *y,
                                                 size_t count, struct pair_summary *out)
{
    long double mean_x = 0.0L;
    long double mean_y = 0.0L;
    long double sum_xx = 0.0L;
    long double sum_xy = 0.0L;
    long double sum_yy = 0.0L;

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

        sum_xx += x_delta * x_delta;
        sum_xy += x_delta * y_delta;
        sum_yy += y_delta * y_delta;
    }
    out->mean_x = (double)mean_x;
    out->mean_y = (double)mean_y;
    out->sum_xx = (double)sum_xx;
    out->sum_xy = (double)sum_xy;
    out->sum_yy = (double)sum_yy;
    if (!isfinite(out->mean_x) || !isfinite(out->mean_y) ||
        !isfinite(out->sum_xx) || !isfinite(out->sum_xy) ||
        !isfinite(out->sum_yy)) {
        return HM_STATUS_UNDEFINED;
    }
    return HM_STATUS_OK;
}

static hm_status calculate_pair_summary(const double *x, const double *y,
                                        size_t count, struct pair_summary *out)
{
    hm_status status = validate_pair_arguments(x, y, count);
    double center_x;
    double center_y;
    double sum_x_deltas = 0.0;
    double sum_y_deltas = 0.0;
    double mean_x_delta;
    double mean_y_delta;

    if (status != HM_STATUS_OK) {
        return status;
    }
    center_x = x[0];
    center_y = y[0];
    for (size_t i = 0; i < count; ++i) {
        const double x_value = x[i];
        const double y_value = y[i];
        const double x_delta = x_value - center_x;
        const double y_delta = y_value - center_y;

        if (!isfinite(x_value) || !isfinite(y_value)) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        if (!isfinite(x_delta) || !isfinite(y_delta)) {
            return calculate_pair_summary_extended(x, y, count, out);
        }
        sum_x_deltas += x_delta;
        sum_y_deltas += y_delta;
        if (!isfinite(sum_x_deltas) || !isfinite(sum_y_deltas)) {
            return calculate_pair_summary_extended(x, y, count, out);
        }
    }
    mean_x_delta = sum_x_deltas / (double)count;
    mean_y_delta = sum_y_deltas / (double)count;
    out->mean_x = center_x + mean_x_delta;
    out->mean_y = center_y + mean_y_delta;
    out->sum_xx = 0.0;
    out->sum_xy = 0.0;
    out->sum_yy = 0.0;
    for (size_t i = 0; i < count; ++i) {
        const double x_delta = (x[i] - center_x) - mean_x_delta;
        const double y_delta = (y[i] - center_y) - mean_y_delta;

        out->sum_xx += x_delta * x_delta;
        out->sum_xy += x_delta * y_delta;
        out->sum_yy += y_delta * y_delta;
    }
    if (!isfinite(out->mean_x) || !isfinite(out->mean_y) ||
        !isfinite(out->sum_xx) || !isfinite(out->sum_xy) ||
        !isfinite(out->sum_yy)) {
        return calculate_pair_summary_extended(x, y, count, out);
    }
    if (count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    return HM_STATUS_OK;
}

hm_status hm_slope(const double *x, const double *y, size_t count, double *out)
{
    struct pair_summary summary;
    hm_status status;
    double result;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    status = calculate_pair_summary(x, y, count, &summary);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (summary.sum_xx == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    result = summary.sum_xy / summary.sum_xx;
    if (!isfinite(result)) {
        return HM_STATUS_UNDEFINED;
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_intercept(const double *x, const double *y, size_t count, double *out)
{
    struct pair_summary summary;
    hm_status status;
    double slope;
    double result;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    status = calculate_pair_summary(x, y, count, &summary);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (summary.sum_xx == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    slope = summary.sum_xy / summary.sum_xx;
    result = summary.mean_y - slope * summary.mean_x;
    if (!isfinite(result)) {
        return HM_STATUS_UNDEFINED;
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_correlation(const double *x, const double *y, size_t count, double *out)
{
    struct pair_summary summary;
    hm_status status;
    double result;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    status = calculate_pair_summary(x, y, count, &summary);
    if (status != HM_STATUS_OK) {
        return status;
    }
    if (summary.sum_xx == 0.0 || summary.sum_yy == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    result = summary.sum_xy / sqrt(summary.sum_xx * summary.sum_yy);
    if (!isfinite(result)) {
        return HM_STATUS_UNDEFINED;
    }
    if (result > 1.0) {
        result = 1.0;
    } else if (result < -1.0) {
        result = -1.0;
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_predict(double x_value, double intercept, double slope, double *out)
{
    double result;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (!isfinite(x_value) || !isfinite(intercept) || !isfinite(slope)) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    result = intercept + slope * x_value;
    if (!isfinite(result)) {
        return HM_STATUS_UNDEFINED;
    }
    *out = result;
    return HM_STATUS_OK;
}
