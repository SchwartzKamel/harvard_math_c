#include <assert.h>
#include <string.h>

#include <harvard_math/harvard_math.h>

int main(void)
{
    /* This smoke test proves the public header, target, and status contract link together. */
    assert(strcmp(hm_status_message(HM_STATUS_OK), "ok") == 0);
    assert(strcmp(hm_status_message(HM_STATUS_NONFINITE_INPUT), "non-finite input") == 0);
    assert(strcmp(hm_status_message((hm_status)-1), "unknown status") == 0);

    return 0;
}
