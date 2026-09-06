#include <harvard_math/regression.h>

#include <float.h>
#include <math.h>
#include <stdlib.h>

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

hm_status hm_multiple_regression_fit(
    const double *predictors,
    size_t rows,
    size_t predictor_count,
    const double *responses,
    double *coefficients)
{
    size_t coefficient_count;
    long double *normal;
    long double *rhs;
    long double *row;
    long double matrix_scale = 0.0L;
    const long double tolerance_multiplier = 1000.0L;

    if (coefficients == NULL || (rows > 0 && (predictors == NULL || responses == NULL))) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (rows == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    if (predictor_count == 0) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (predictor_count == (size_t)-1) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    coefficient_count = predictor_count + 1;
    if (rows < coefficient_count) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    if (coefficient_count > (size_t)-1 / coefficient_count ||
        rows > (size_t)-1 / predictor_count) {
        return HM_STATUS_ALLOCATION_FAILED;
    }

    normal = calloc(coefficient_count * coefficient_count, sizeof *normal);
    rhs = calloc(coefficient_count, sizeof *rhs);
    row = malloc(coefficient_count * sizeof *row);
    if (normal == NULL || rhs == NULL || row == NULL) {
        free(normal);
        free(rhs);
        free(row);
        return HM_STATUS_ALLOCATION_FAILED;
    }

    for (size_t r = 0; r < rows; ++r) {
        const double response = responses[r];

        if (!isfinite(response)) {
            free(normal);
            free(rhs);
            free(row);
            return HM_STATUS_NONFINITE_INPUT;
        }
        row[0] = 1.0L;
        for (size_t c = 0; c < predictor_count; ++c) {
            const double value = predictors[r * predictor_count + c];

            if (!isfinite(value)) {
                free(normal);
                free(rhs);
                free(row);
                return HM_STATUS_NONFINITE_INPUT;
            }
            row[c + 1] = (long double)value;
        }
        for (size_t i = 0; i < coefficient_count; ++i) {
            rhs[i] += row[i] * (long double)response;
            for (size_t j = 0; j < coefficient_count; ++j) {
                normal[i * coefficient_count + j] += row[i] * row[j];
            }
        }
    }

    for (size_t i = 0; i < coefficient_count * coefficient_count; ++i) {
        const long double magnitude = fabsl(normal[i]);

        if (!isfinite((double)magnitude)) {
            free(normal);
            free(rhs);
            free(row);
            return HM_STATUS_UNDEFINED;
        }
        if (magnitude > matrix_scale) {
            matrix_scale = magnitude;
        }
    }
    if (matrix_scale == 0.0L) {
        free(normal);
        free(rhs);
        free(row);
        return HM_STATUS_UNDEFINED;
    }

    for (size_t pivot_column = 0; pivot_column < coefficient_count; ++pivot_column) {
        size_t pivot_row = pivot_column;
        long double pivot_magnitude = 0.0L;

        for (size_t r = pivot_column; r < coefficient_count; ++r) {
            const long double magnitude = fabsl(normal[r * coefficient_count + pivot_column]);
            if (magnitude > pivot_magnitude) {
                pivot_magnitude = magnitude;
                pivot_row = r;
            }
        }
        if (pivot_magnitude <= matrix_scale * LDBL_EPSILON * tolerance_multiplier) {
            free(normal);
            free(rhs);
            free(row);
            return HM_STATUS_UNDEFINED;
        }
        if (pivot_row != pivot_column) {
            for (size_t c = 0; c < coefficient_count; ++c) {
                const size_t first = pivot_column * coefficient_count + c;
                const size_t second = pivot_row * coefficient_count + c;
                const long double temporary = normal[first];
                normal[first] = normal[second];
                normal[second] = temporary;
            }
            {
                const long double temporary = rhs[pivot_column];
                rhs[pivot_column] = rhs[pivot_row];
                rhs[pivot_row] = temporary;
            }
        }
        for (size_t r = pivot_column + 1; r < coefficient_count; ++r) {
            const long double factor = normal[r * coefficient_count + pivot_column] /
                                       normal[pivot_column * coefficient_count + pivot_column];
            normal[r * coefficient_count + pivot_column] = 0.0L;
            for (size_t c = pivot_column + 1; c < coefficient_count; ++c) {
                normal[r * coefficient_count + c] -= factor *
                    normal[pivot_column * coefficient_count + c];
            }
            rhs[r] -= factor * rhs[pivot_column];
        }
    }

    for (size_t i = coefficient_count; i-- > 0;) {
        long double value = rhs[i];

        for (size_t j = i + 1; j < coefficient_count; ++j) {
            value -= normal[i * coefficient_count + j] * rhs[j];
        }
        rhs[i] = value / normal[i * coefficient_count + i];
        if (!isfinite((double)rhs[i])) {
            free(normal);
            free(rhs);
            free(row);
            return HM_STATUS_UNDEFINED;
        }
    }

    for (size_t i = 0; i < coefficient_count; ++i) {
        coefficients[i] = (double)rhs[i];
    }
    free(normal);
    free(rhs);
    free(row);
    return HM_STATUS_OK;
}

hm_status hm_multiple_predict(
    const double *coefficients,
    size_t predictor_count,
    const double *predictors,
    double *out)
{
    double result;

    if (out == NULL || coefficients == NULL || predictors == NULL || predictor_count == 0) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (!isfinite(coefficients[0])) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    result = coefficients[0];
    for (size_t i = 0; i < predictor_count; ++i) {
        if (!isfinite(coefficients[i + 1]) || !isfinite(predictors[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        result += coefficients[i + 1] * predictors[i];
    }
    if (!isfinite(result)) {
        return HM_STATUS_UNDEFINED;
    }
    *out = result;
    return HM_STATUS_OK;
}
