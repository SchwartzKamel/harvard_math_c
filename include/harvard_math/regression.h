#ifndef HARVARD_MATH_REGRESSION_H
#define HARVARD_MATH_REGRESSION_H

#include "stats.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Compute ordinary least-squares statistics for paired predictor/response arrays. */
HM_API hm_status hm_slope(const double *x, const double *y, size_t count, double *out);
HM_API hm_status hm_intercept(const double *x, const double *y, size_t count, double *out);
HM_API hm_status hm_correlation(const double *x, const double *y, size_t count, double *out);

/* Predict one response from a fitted simple-regression intercept and slope. */
HM_API hm_status hm_predict(double x_value, double intercept, double slope, double *out);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_REGRESSION_H */
