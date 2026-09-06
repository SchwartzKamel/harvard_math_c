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
    return fabs(actual - expected) < 1e-6;
}

int main(void)
{
    const double values[] = {2.0, 4.0, 4.0, 8.0};
    const double predictor[] = {1.0, 2.0, 3.0, 4.0};
    const double response[] = {2.0, 4.0, 6.0, 8.0};
    const double multiple_predictors[] = {1.0, 0.0, 0.0, 1.0, 2.0, 1.0, 1.0, 2.0};
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
    double prediction;
    double coefficients[3];
    double multiple_prediction;
    double custom;
    hm_ztest_result ztest;
    hm_confidence_interval interval;
    hm_statistic_fn custom_statistic = midrange;

    if (hm_mean(values, 4, &mean) != HM_STATUS_OK ||
        hm_median(values, 4, &median) != HM_STATUS_OK ||
        hm_quantile(values, 4, 0.25, &q1) != HM_STATUS_OK ||
        hm_quantile(values, 4, 0.75, &q3) != HM_STATUS_OK ||
        hm_stddev_sample(values, 4, &sample_stddev) != HM_STATUS_OK ||
        hm_zscore(8.0, mean, sample_stddev, &zscore) != HM_STATUS_OK ||
        hm_slope(predictor, response, 4, &slope) != HM_STATUS_OK ||
        hm_ztest_one_sample(104.0, 100.0, 10.0, 25, HM_ALTERNATIVE_TWO_SIDED, &ztest) != HM_STATUS_OK ||
        hm_mean_confidence_interval(100.0, 10.0, 25, 0.95, &interval) != HM_STATUS_OK ||
        hm_intercept(predictor, response, 4, &intercept) != HM_STATUS_OK ||
        hm_correlation(predictor, response, 4, &correlation) != HM_STATUS_OK ||
        hm_predict(5.0, intercept, slope, &prediction) != HM_STATUS_OK ||
        hm_multiple_regression_fit(multiple_predictors, 4, 2, multiple_responses, coefficients) != HM_STATUS_OK ||
        hm_multiple_predict(coefficients, 2, prediction_row, &multiple_prediction) != HM_STATUS_OK ||
        custom_statistic(values, 4, &custom) != HM_STATUS_OK) {
        return 1;
    }
    if (!close_enough(mean, 4.5) ||
        !close_enough(median, 4.0) ||
        !close_enough(q1, 3.5) ||
        !close_enough(q3, 5.0) ||
        !close_enough(sample_stddev, sqrt(19.0 / 3.0)) ||
        !close_enough(zscore, (8.0 - 4.5) / sqrt(19.0 / 3.0)) ||
        !close_enough(slope, 2.0) ||
        !close_enough(ztest.z_score, 2.0) ||
        !close_enough(ztest.p_value, 0.0455002639) ||
        !close_enough(interval.lower, 96.080072) ||
        !close_enough(interval.upper, 103.919928) ||
        !close_enough(intercept, 0.0) ||
        !close_enough(correlation, 1.0) ||
        !close_enough(prediction, 10.0) ||
        !close_enough(coefficients[0], 5.0) ||
        !close_enough(coefficients[1], 2.0) ||
        !close_enough(coefficients[2], -3.0) ||
        !close_enough(multiple_prediction, -1.0) ||
        !close_enough(custom, 5.0)) {
        return 1;
    }

    printf("mean=%.2f q1=%.2f median=%.2f q3=%.2f sample_stddev=%.6f zscore=%.6f slope=%.2f "
           "ztest=%.2f p=%.6f ci=[%.6f,%.6f] intercept=%.2f correlation=%.2f "
           "prediction=%.2f multiple_prediction=%.2f midrange=%.2f\n",
           mean, q1, median, q3, sample_stddev, zscore, slope,
           ztest.z_score, ztest.p_value, interval.lower, interval.upper,
           intercept, correlation, prediction, multiple_prediction, custom);
    return 0;
}
