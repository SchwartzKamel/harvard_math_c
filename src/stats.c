#include <harvard_math/stats.h>

const char *hm_status_message(hm_status status)
{
    switch (status) {
    case HM_STATUS_OK:
        return "ok";
    case HM_STATUS_INVALID_ARGUMENT:
        return "invalid argument";
    case HM_STATUS_EMPTY_INPUT:
        return "empty input";
    case HM_STATUS_INSUFFICIENT_DATA:
        return "insufficient data";
    case HM_STATUS_NONFINITE_INPUT:
        return "non-finite input";
    case HM_STATUS_ALLOCATION_FAILED:
        return "allocation failed";
    default:
        return "unknown status";
    }
}
