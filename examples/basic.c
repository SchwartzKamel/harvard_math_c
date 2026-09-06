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
    const double multiple_predictors[] = {
        1.0, 0.0,
        0.0, 1.0,
        2.0, 1.0,
        1.0, 2.0
    };
    const double multiple_responses[] = {7.0, 2.0, 6.0, 1.0};
    const double prediction_row[] = {3.0, 4.0};
    double mean;
    double median;
    double q1;
    double q3;
    double sample_stddev;
    double zscore;
    double slope;
    double intercept;
    double correlation;
    double covariance_sample;
    double prediction;
    double coefficients[3];
    double multiple_prediction;
    double midrange_result;
    hm_ztest_result ztest;
    hm_confidence_interval interval;
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
    status = hm_quantile(values, 4, 0.25, &q1);
    if (status != HM_STATUS_OK) {
        return report_failure("first quartile", status);
    }
    status = hm_quantile(values, 4, 0.75, &q3);
    if (status != HM_STATUS_OK) {
        return report_failure("third quartile", status);
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
    status = hm_ztest_one_sample(104.0, 100.0, 10.0, 25,
                                 HM_ALTERNATIVE_TWO_SIDED, &ztest);
    if (status != HM_STATUS_OK) {
        return report_failure("one-sample z-test", status);
    }
    status = hm_mean_confidence_interval(100.0, 10.0, 25, 0.95, &interval);
    if (status != HM_STATUS_OK) {
        return report_failure("confidence interval", status);
    }
    status = hm_intercept(predictor, response, 4, &intercept);
    if (status != HM_STATUS_OK) {
        return report_failure("intercept", status);
    }
    status = hm_correlation(predictor, response, 4, &correlation);
    if (status != HM_STATUS_OK) {
        return report_failure("correlation", status);
    }
    status = hm_covariance_sample(predictor, response, 4, &covariance_sample);
    if (status != HM_STATUS_OK) {
        return report_failure("sample covariance", status);
    }
    status = hm_predict(5.0, intercept, slope, &prediction);
    if (status != HM_STATUS_OK) {
        return report_failure("simple prediction", status);
    }
    status = hm_multiple_regression_fit(
        multiple_predictors, 4, 2, multiple_responses, coefficients);
    if (status != HM_STATUS_OK) {
        return report_failure("multiple regression", status);
    }
    status = hm_multiple_predict(coefficients, 2, prediction_row, &multiple_prediction);
    if (status != HM_STATUS_OK) {
        return report_failure("multiple prediction", status);
    }
    status = custom_statistic(values, 4, &midrange_result);
    if (status != HM_STATUS_OK) {
        return report_failure("custom midrange", status);
    }

    printf("mean=%.2f q1=%.2f median=%.2f q3=%.2f sample_stddev=%.6f "
           "zscore=%.6f slope=%.2f ztest=%.2f p=%.6f ci=[%.6f,%.6f] "
           "intercept=%.2f correlation=%.2f covariance=%.2f prediction=%.2f "
           "multiple_prediction=%.2f midrange=%.2f\n",
           mean, q1, median, q3, sample_stddev, zscore, slope,
           ztest.z_score, ztest.p_value, interval.lower, interval.upper,
           intercept, correlation, covariance_sample, prediction, multiple_prediction,
           midrange_result);
    return 0;
}
