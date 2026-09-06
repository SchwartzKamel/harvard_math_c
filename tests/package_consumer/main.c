#include <math.h>
#include <stdio.h>
#include <stddef.h>

#include <harvard_math/harvard_math.h>

static hm_status midrange(const double *values, size_t count, double *out)
{
    double minimum;
    double maximum;
    hm_status status = hm_min(values, count, &minimum);

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

static int close_enough(double actual, double expected)
{
    return fabs(actual - expected) < 1e-12;
}

int main(void)
{
    const double values[] = {2.0, 4.0, 4.0, 8.0};
    const double predictor[] = {1.0, 2.0, 3.0, 4.0};
    const double response[] = {2.0, 4.0, 6.0, 8.0};
    double mean;
    double median;
    double sample_stddev;
    double zscore;
    double slope;
    double custom;
    hm_statistic_fn custom_statistic = midrange;

    if (hm_mean(values, 4, &mean) != HM_STATUS_OK ||
        hm_median(values, 4, &median) != HM_STATUS_OK ||
        hm_stddev_sample(values, 4, &sample_stddev) != HM_STATUS_OK ||
        hm_zscore(8.0, mean, sample_stddev, &zscore) != HM_STATUS_OK ||
        hm_slope(predictor, response, 4, &slope) != HM_STATUS_OK ||
        custom_statistic(values, 4, &custom) != HM_STATUS_OK) {
        return 1;
    }
    if (!close_enough(mean, 4.5) ||
        !close_enough(median, 4.0) ||
        !close_enough(sample_stddev, sqrt(19.0 / 3.0)) ||
        !close_enough(zscore, (8.0 - 4.5) / sqrt(19.0 / 3.0)) ||
        !close_enough(slope, 2.0) ||
        !close_enough(custom, 5.0)) {
        return 1;
    }

    printf("mean=%.2f median=%.2f sample_stddev=%.6f zscore=%.6f slope=%.2f midrange=%.2f\n",
           mean, median, sample_stddev, zscore, slope, custom);
    return 0;
}
