#ifndef HARVARD_MATH_STATS_H
#define HARVARD_MATH_STATS_H

#include <stddef.h>

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

const char *hm_status_message(hm_status status);

#ifdef __cplusplus
}
#endif

#endif /* HARVARD_MATH_STATS_H */
