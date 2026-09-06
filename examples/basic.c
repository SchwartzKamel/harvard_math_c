#include <stddef.h>
#include <stdio.h>

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

static int report_failure(const char *name, hm_status status)
{
    fprintf(stderr, "%s failed: %s\n", name, hm_status_message(status));
    return 1;
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
    double midrange_result;
    hm_status status;
    hm_statistic_fn custom_statistic = midrange;

    status = hm_mean(values, 4, &mean);
    if (status != HM_STATUS_OK) {
        return report_failure("mean", status);
    }
    status = hm_median(values, 4, &median);
    if (status != HM_STATUS_OK) {
        return report_failure("median", status);
    }
    status = hm_stddev_sample(values, 4, &sample_stddev);
    if (status != HM_STATUS_OK) {
        return report_failure("sample standard deviation", status);
    }
    status = hm_zscore(8.0, mean, sample_stddev, &zscore);
    if (status != HM_STATUS_OK) {
        return report_failure("z-score", status);
    }
    status = hm_slope(predictor, response, 4, &slope);
    if (status != HM_STATUS_OK) {
        return report_failure("slope", status);
    }
    status = custom_statistic(values, 4, &midrange_result);
    if (status != HM_STATUS_OK) {
        return report_failure("custom midrange", status);
    }

    printf("mean=%.2f median=%.2f sample_stddev=%.6f zscore=%.6f slope=%.2f midrange=%.2f\n",
           mean, median, sample_stddev, zscore, slope, midrange_result);
    return 0;
}
