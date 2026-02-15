/* Small portability wrapper providing QSORT_R()
 * Uses the standard qsort() under the hood and a static glue comparator.
 * Not reentrant/thread-safe, but keeps compatibility across platforms.
 */
#include <stdlib.h>
#include "qsort_r.h"

static qsort_r_compar_t qs_cmp = NULL;
static void *qs_arg = NULL;

static int qs_glue(const void *a, const void *b) {
    return qs_cmp(a, b, qs_arg);
}

void QSORT_R(void *base, size_t nmemb, size_t size, qsort_r_compar_t compar, void *arg) {
    /* fallback implementation: store comparator+arg into globals and call qsort */
    qs_cmp = compar;
    qs_arg = arg;
    qsort(base, nmemb, size, qs_glue);
    qs_cmp = NULL;
    qs_arg = NULL;
}
