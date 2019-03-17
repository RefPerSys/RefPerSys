#define __STDC_FORMAT_MACROS 1
#include "mt64.h"
#include <inttypes.h>
int main (void) {
    printf("%" PRIu64 "\n", rps_random_uint64());
    return 0;
}