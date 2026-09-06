#ifndef HARVARD_MATH_ANALYSIS_H
#define HARVARD_MATH_ANALYSIS_H

#include "stats.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum hm_alternative {
    HM_ALTERNATIVE_TWO_SIDED = 0,
    HM_ALTERNATIVE_LESS,
    HM_ALTERNATIVE_GREATER
} hm_alternative;

typedef struct hm_ztest_result {
    double z_score;
    double p_value;
} hm_ztest_result;

typedef struct hm_confidence_interval {
    double lower;
    double upper;
} hm_confidence_interval;

/* Known-population-standard-deviation z-tests for one or two sample means. */
HM_API hm_status hm_ztest_one_sample(
    double sample_mean,
    double population_mean,
    double population_stddev,
    size_t count,
    hm_alternative alternative,
    hm_ztest_result *out);

HM_API hm_status hm_ztest_two_sample(
    double sample_mean_a,
    double population_stddev_a,
    size_t count_a,
    double sample_mean_b,
    double population_stddev_b,
    size_t count_b,
    hm_alternative alternative,
    hm_ztest_result *out);

/* Known-population-standard-deviation confidence interval for a mean. */
HM_API hm_status hm_mean_confidence_interval(
    double sample_mean,
    double population_stddev,
    size_t count,
    double confidence,
    hm_confidence_interval *out);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_ANALYSIS_H */
