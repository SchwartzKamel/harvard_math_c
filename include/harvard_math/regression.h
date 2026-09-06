#ifndef HARVARD_MATH_REGRESSION_H
#define HARVARD_MATH_REGRESSION_H

#include "stats.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Compute the ordinary least-squares slope for paired predictor/response arrays. */
HM_API hm_status hm_slope(const double *x, const double *y, size_t count, double *out);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_REGRESSION_H */
