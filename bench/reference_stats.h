#ifndef HARVARD_MATH_REFERENCE_STATS_H
#define HARVARD_MATH_REFERENCE_STATS_H

#include <harvard_math/stats.h>

hm_status hm_reference_sum(const double *values, size_t count, double *out);
hm_status hm_reference_mean(const double *values, size_t count, double *out);
hm_status hm_reference_min(const double *values, size_t count, double *out);
hm_status hm_reference_max(const double *values, size_t count, double *out);
hm_status hm_reference_range(const double *values, size_t count, double *out);
hm_status hm_reference_median(const double *values, size_t count, double *out);
hm_status hm_reference_quantile_q1(const double *values, size_t count, double *out);
hm_status hm_reference_quantile_q3(const double *values, size_t count, double *out);
hm_status hm_reference_variance_population(const double *values, size_t count, double *out);
hm_status hm_reference_variance_sample(const double *values, size_t count, double *out);
hm_status hm_reference_stddev_population(const double *values, size_t count, double *out);
hm_status hm_reference_stddev_sample(const double *values, size_t count, double *out);

#endif /* HARVARD_MATH_REFERENCE_STATS_H */
