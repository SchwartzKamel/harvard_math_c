#include <harvard_math/stats.h>

#include <math.h>
#include <stdlib.h>

static hm_status validate_arguments(const double *values, size_t count, double *out)
{
    if (out == NULL || (count > 0 && values == NULL)) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    return HM_STATUS_OK;
}

static hm_status calculate_variance_extended(const double *values, size_t count, int sample, double *out)
{
    long double mean = 0.0L;
    long double sum_squared_deltas = 0.0L;

    /* Rare fallback for finite inputs whose shifted values overflow double. */
    for (size_t i = 0; i < count; ++i) {
        const double input = values[i];
        const long double value = (long double)input;
        long double delta;

        if (!isfinite(input)) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        delta = value - mean;
        mean += delta / (long double)(i + 1);
        sum_squared_deltas += delta * (value - mean);
    }
    if (sample && count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    if (sum_squared_deltas < 0.0L) {
        sum_squared_deltas = 0.0L;
    }
    *out = (double)(sum_squared_deltas / (long double)(sample ? count - 1 : count));
    return HM_STATUS_OK;
}

static hm_status calculate_variance(const double *values, size_t count, int sample, double *out)
{
    hm_status status = validate_arguments(values, count, out);
    double center;
    double sum_deltas = 0.0;
    double mean_delta;
    double sum_squared_deltas = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    center = values[0];

    /* Centering keeps large common offsets out of the fast double arithmetic. */
    for (size_t i = 0; i < count; ++i) {
        const double value = values[i];
        const double delta = value - center;

        if (!isfinite(value)) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        if (!isfinite(delta)) {
            return calculate_variance_extended(values, count, sample, out);
        }
        sum_deltas += delta;
        if (!isfinite(sum_deltas)) {
            return calculate_variance_extended(values, count, sample, out);
        }
    }
    mean_delta = sum_deltas / (double)count;
    for (size_t i = 0; i < count; ++i) {
        const double difference = (values[i] - center) - mean_delta;

        sum_squared_deltas += difference * difference;
    }
    /* Preserve the original status precedence for one-value sample input. */
    if (sample && count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    *out = sum_squared_deltas / (double)(sample ? count - 1 : count);
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

static double median_of_three(double first, double second, double third)
{
    if (first < second) {
        if (second < third) {
            return second;
        }
        return first < third ? third : first;
    }
    if (first < third) {
        return first;
    }
    return second < third ? third : second;
}

static void select_kth(double *values, size_t count, size_t target)
{
    size_t left = 0;
    size_t right = count - 1;
    size_t depth_limit = 0;

    for (size_t remaining = count; remaining > 1; remaining >>= 1) {
        depth_limit += 2;
    }
    while (left < right) {
        const size_t middle = left + (right - left) / 2;
        if (depth_limit == 0) {
            /* Introselect bounds adversarial pivots with a final local sort. */
            qsort(values + left, right - left + 1, sizeof *values, compare_doubles);
            return;
        }
        --depth_limit;
        const double pivot = median_of_three(values[left], values[middle], values[right]);
        size_t lower = left;
        size_t current = left;
        size_t upper = right;

        /* Three-way partition handles repeated values without quadratic duplicate work. */
        while (current <= upper) {
            if (values[current] < pivot) {
                const double temporary = values[lower];
                values[lower] = values[current];
                values[current] = temporary;
                ++lower;
                ++current;
            } else if (values[current] > pivot) {
                const double temporary = values[current];
                values[current] = values[upper];
                values[upper] = temporary;
                if (upper == 0) {
                    break;
                }
                --upper;
            } else {
                ++current;
            }
        }

        if (target < lower) {
            right = lower - 1;
        } else if (target > upper) {
            left = upper + 1;
        } else {
            return;
        }
    }
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
    hm_status status = validate_arguments(values, count, out);
    double sum = 0.0;

    if (status != HM_STATUS_OK) {
        return status;
    }
    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        sum += values[i];
    }
    *out = sum;
    return HM_STATUS_OK;
}

hm_status hm_mean(const double *values, size_t count, double *out)
{
    hm_status status = hm_sum(values, count, out);

    if (status != HM_STATUS_OK) {
        return status;
    }
    *out /= (double)count;
    return HM_STATUS_OK;
}

hm_status hm_min(const double *values, size_t count, double *out)
{
    hm_status status = validate_arguments(values, count, out);
    double result;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (!isfinite(values[0])) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    result = values[0];
    for (size_t i = 1; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        if (values[i] < result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_max(const double *values, size_t count, double *out)
{
    hm_status status = validate_arguments(values, count, out);
    double result;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (!isfinite(values[0])) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    result = values[0];
    for (size_t i = 1; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        if (values[i] > result) {
            result = values[i];
        }
    }
    *out = result;
    return HM_STATUS_OK;
}

hm_status hm_range(const double *values, size_t count, double *out)
{
    hm_status status = validate_arguments(values, count, out);
    double minimum;
    double maximum;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (!isfinite(values[0])) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    minimum = values[0];
    maximum = values[0];
    for (size_t i = 1; i < count; ++i) {
        if (!isfinite(values[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
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
    hm_status status = validate_arguments(values, count, out);
    double *working;
    size_t middle;

    if (status != HM_STATUS_OK) {
        return status;
    }
    if (count > (size_t)-1 / sizeof *working) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    working = malloc(count * sizeof *working);
    if (working == NULL) {
        return HM_STATUS_ALLOCATION_FAILED;
    }
    /* Copying and validating together removes a full read before ordering. */
    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(values[i])) {
            free(working);
            return HM_STATUS_NONFINITE_INPUT;
        }
        working[i] = values[i];
    }
    if (count <= 32) {
        qsort(working, count, sizeof *working, compare_doubles);
    } else {
        select_kth(working, count, count / 2);
    }

    middle = count / 2;
    if (count % 2 == 0) {
        if (count > 32) {
            select_kth(working, middle, middle - 1);
        }
        *out = working[middle - 1] / 2.0 + working[middle] / 2.0;
    } else {
        *out = working[middle];
    }
    free(working);
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
