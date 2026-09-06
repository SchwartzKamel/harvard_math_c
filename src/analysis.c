#include <harvard_math/analysis.h>

#include <math.h>

static const double HM_SQRT_TWO = 1.41421356237309504880;

static int valid_alternative(hm_alternative alternative)
{
    return alternative == HM_ALTERNATIVE_TWO_SIDED ||
           alternative == HM_ALTERNATIVE_LESS ||
           alternative == HM_ALTERNATIVE_GREATER;
}

static double normal_p_value(double z_score, hm_alternative alternative)
{
    switch (alternative) {
    case HM_ALTERNATIVE_TWO_SIDED:
        return erfc(fabs(z_score) / HM_SQRT_TWO);
    case HM_ALTERNATIVE_LESS:
        return 0.5 * erfc(-z_score / HM_SQRT_TWO);
    case HM_ALTERNATIVE_GREATER:
        return 0.5 * erfc(z_score / HM_SQRT_TWO);
    default:
        return NAN;
    }
}

/* Acklam's rational inverse-normal approximation, accurate enough for classroom intervals. */
static double inverse_normal(double probability)
{
    static const double a[] = {
        -3.969683028665376e+01, 2.209460984245205e+02,
        -2.759285104469687e+02, 1.383577518672690e+02,
        -3.066479806614716e+01, 2.506628277459239e+00
    };
    static const double b[] = {
        -5.447609879822406e+01, 1.615858368580409e+02,
        -1.556989798598866e+02, 6.680131188771972e+01,
        -1.328068155288572e+01
    };
    static const double c[] = {
        -7.784894002430293e-03, -3.223964580411365e-01,
        -2.400758277161838e+00, -2.549732539343734e+00,
        4.374664141464968e+00, 2.938163982698783e+00
    };
    static const double d[] = {
        7.784695709041462e-03, 3.224671290700398e-01,
        2.445134137142996e+00, 3.754408661907416e+00
    };
    const double low = 0.02425;
    const double high = 1.0 - low;
    double q;
    double r;

    if (probability < low) {
        q = sqrt(-2.0 * log(probability));
        return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
                ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    if (probability > high) {
        q = sqrt(-2.0 * log(1.0 - probability));
        return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
                 ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    q = probability - 0.5;
    r = q * q;
    return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
           (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
}

static hm_status write_ztest(double z_score, hm_alternative alternative, hm_ztest_result *out)
{
    const double p_value = normal_p_value(z_score, alternative);

    if (!isfinite(z_score) || !isfinite(p_value)) {
        return HM_STATUS_UNDEFINED;
    }
    out->z_score = z_score;
    out->p_value = p_value;
    return HM_STATUS_OK;
}

hm_status hm_ztest_one_sample(
    double sample_mean,
    double population_mean,
    double population_stddev,
    size_t count,
    hm_alternative alternative,
    hm_ztest_result *out)
{
    double standard_error;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    if (!isfinite(sample_mean) || !isfinite(population_mean) || !isfinite(population_stddev)) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    if (!valid_alternative(alternative) || population_stddev < 0.0) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (population_stddev == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    standard_error = population_stddev / sqrt((double)count);
    return write_ztest((sample_mean - population_mean) / standard_error, alternative, out);
}

hm_status hm_ztest_two_sample(
    double sample_mean_a,
    double population_stddev_a,
    size_t count_a,
    double sample_mean_b,
    double population_stddev_b,
    size_t count_b,
    hm_alternative alternative,
    hm_ztest_result *out)
{
    double standard_error;
    double variance_of_difference;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count_a == 0 || count_b == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    if (!isfinite(sample_mean_a) || !isfinite(population_stddev_a) ||
        !isfinite(sample_mean_b) || !isfinite(population_stddev_b)) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    if (!valid_alternative(alternative) || population_stddev_a < 0.0 ||
        population_stddev_b < 0.0) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (population_stddev_a == 0.0 || population_stddev_b == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    variance_of_difference = (population_stddev_a * population_stddev_a) / (double)count_a +
                             (population_stddev_b * population_stddev_b) / (double)count_b;
    standard_error = sqrt(variance_of_difference);
    return write_ztest((sample_mean_a - sample_mean_b) / standard_error, alternative, out);
}

hm_status hm_mean_confidence_interval(
    double sample_mean,
    double population_stddev,
    size_t count,
    double confidence,
    hm_confidence_interval *out)
{
    double critical;
    double margin;
    hm_confidence_interval result;

    if (out == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    if (!isfinite(sample_mean) || !isfinite(population_stddev) || !isfinite(confidence)) {
        return HM_STATUS_NONFINITE_INPUT;
    }
    if (population_stddev < 0.0 || confidence <= 0.0 || confidence >= 1.0) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (population_stddev == 0.0) {
        return HM_STATUS_UNDEFINED;
    }
    critical = inverse_normal(0.5 + confidence / 2.0);
    margin = critical * population_stddev / sqrt((double)count);
    if (!isfinite(critical) || !isfinite(margin)) {
        return HM_STATUS_UNDEFINED;
    }
    result.lower = sample_mean - margin;
    result.upper = sample_mean + margin;
    if (!isfinite(result.lower) || !isfinite(result.upper)) {
        return HM_STATUS_UNDEFINED;
    }
    *out = result;
    return HM_STATUS_OK;
}
