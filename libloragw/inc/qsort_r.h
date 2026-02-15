/* QSORT_R portability wrapper header (renamed) */
#ifndef _QSORT_R_H_
#define _QSORT_R_H_

#include <stdlib.h>

typedef int (*qsort_r_compar_t)(const void *a, const void *b, void *arg);

/* QSORT_R: stable wrapper signature
 * base, nmemb, size: as qsort
 * compar: function (const void*, const void*, void*)
 * arg: user-provided pointer forwarded to comparator
 */
void QSORT_R(void *base, size_t nmemb, size_t size, qsort_r_compar_t compar, void *arg);

#endif /* _QSORT_R_H_ */
