#ifndef HARVARD_MATH_GSL_ADAPTER_H
#define HARVARD_MATH_GSL_ADAPTER_H

#include <harvard_math/stats.h>

hm_status hm_gsl_mean(const double *values, size_t count, double *out);
hm_status hm_gsl_min(const double *values, size_t count, double *out);
hm_status hm_gsl_max(const double *values, size_t count, double *out);
hm_status hm_gsl_range(const double *values, size_t count, double *out);
hm_status hm_gsl_median(const double *values, size_t count, double *out);
hm_status hm_gsl_variance_sample(const double *values, size_t count, double *out);
hm_status hm_gsl_stddev_sample(const double *values, size_t count, double *out);

#endif /* HARVARD_MATH_GSL_ADAPTER_H */
