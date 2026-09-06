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

/*
 * Fit a row-major predictor matrix with an automatically added intercept.
 * predictors has rows * predictor_count values; coefficients has predictor_count + 1 values.
 */
HM_API hm_status hm_multiple_regression_fit(
    const double *predictors,
    size_t rows,
    size_t predictor_count,
    const double *responses,
    double *coefficients);

/* Predict from an intercept-first coefficient vector and one predictor row. */
HM_API hm_status hm_multiple_predict(
    const double *coefficients,
    size_t predictor_count,
    const double *predictors,
    double *out);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_REGRESSION_H */
