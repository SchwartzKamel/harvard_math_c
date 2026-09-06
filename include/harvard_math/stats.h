#ifndef HARVARD_MATH_STATS_H
#define HARVARD_MATH_STATS_H

#include <stddef.h>

#if defined(_WIN32) && defined(HM_BUILD_SHARED)
#    if defined(HM_BUILDING_LIBRARY)
#        define HM_API __declspec(dllexport)
#    else
#        define HM_API __declspec(dllimport)
#    endif
#else
#    define HM_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum hm_status {
    HM_STATUS_OK = 0,
    HM_STATUS_INVALID_ARGUMENT,
    HM_STATUS_EMPTY_INPUT,
    HM_STATUS_INSUFFICIENT_DATA,
    HM_STATUS_NONFINITE_INPUT,
    HM_STATUS_ALLOCATION_FAILED
} hm_status;

/* All built-in and user-defined batch statistics use this signature. */
typedef hm_status (*hm_statistic_fn)(const double *values, size_t count, double *out);

/* Return a stable, human-readable description of a status value. */
HM_API const char *hm_status_message(hm_status status);

/*
 * Each statistic reads count finite values without modifying them and writes to out on success.
 * Passing count == 0 returns HM_STATUS_EMPTY_INPUT; values may be NULL in that case.
 */
HM_API hm_status hm_sum(const double *values, size_t count, double *out);
HM_API hm_status hm_mean(const double *values, size_t count, double *out);
HM_API hm_status hm_min(const double *values, size_t count, double *out);
HM_API hm_status hm_max(const double *values, size_t count, double *out);
HM_API hm_status hm_range(const double *values, size_t count, double *out);

/* Median copies before sorting, so the caller's values are never reordered. */
HM_API hm_status hm_median(const double *values, size_t count, double *out);

/* Population uses divisor n; sample uses divisor n - 1 and requires count >= 2. */
HM_API hm_status hm_variance_population(const double *values, size_t count, double *out);
HM_API hm_status hm_variance_sample(const double *values, size_t count, double *out);
HM_API hm_status hm_stddev_population(const double *values, size_t count, double *out);
HM_API hm_status hm_stddev_sample(const double *values, size_t count, double *out);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_STATS_H */
