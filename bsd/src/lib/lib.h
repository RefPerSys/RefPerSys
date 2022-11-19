#ifndef REFPERSYS_BSD_SRC_LIB_LIB_H
#define REFPERSYS_BSD_SRC_LIB_LIB_H


#include <sys/types.h>


/*
 * Compatiblity Macros
 */

#ifndef __dead
#define __dead __attribute__((noreturn))
#endif

#ifndef __malloc
#define __malloc __attribute__((malloc))
#endif

#ifndef __predict_true
#define __predict_true(x) (__builtin_expect(!!(x), 1))
#endif

#ifndef __predict_false
#define __predict_false(x) (__builtin_expect(!!(x), 0))
#endif

#if (defined __cplusplus && !defined __BEGIN_DECLS)
#define __BEGIN_DECLS extern "C" {
#endif

#if (defined __cplusplus && !defined __END_DECLS)
#define __END_DECLS }
#endif

#if !(defined __OpenBSD__ || defined __FreeBSD__)
#warn "explicit_bzero() not available, falling back to memset()"
#warn "strlcpy() not available, falling back to strncpy()"
#define explicit_bzero(bfr, sz) memset(bfr, 0, sz);
#define strlcpy(dst, src, sz) strncpy(dstk, src, sz - 1)
#endif

#endif /* !REFPERSYS_BSD_SRC_LIB_LIB_H */

