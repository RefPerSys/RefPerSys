#ifndef REFPERSYS_BSD_SRC_LIB_LIB_H
#define REFPERSYS_BSD_SRC_LIB_LIB_H


#include <sys/types.h>
#include <stdio.h>


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


/*
 * Log interface
 */

__BEGIN_DECLS
extern FILE	*rps_log_file(void);
extern void	 rps_log_file_set(FILE *);
extern void 	 rps_log_ok(const char *, ...);
extern void 	 rps_log_info(const char *, ...);
extern void 	 rps_log_debug(const char *, ...);
extern void 	 rps_log_warn(const char *, ...);
extern void	 rps_log_fail(int, const char *, ...) __dead;
__END_DECLS


/*
 * Memory interface and string interface
 * Will be replaced by RefPerSys's own GC
 */

typedef void rps_mem;

__BEGIN_DECLS
extern rps_mem *rps_mem_new(size_t) __malloc;
extern size_t rps_mem_sz(const rps_mem *);
extern void rps_mem_free(rps_mem *);
__END_DECLS


/*
 * String interface
 */

typedef char rps_str;

__BEGIN_DECLS
extern rps_str *rps_str_new(const char *) __malloc;
extern rps_str *rps_str_new_raw(size_t) __malloc;
extern void rps_str_free(rps_str **);
extern int rps_str_cmp(const rps_str *, const char *);
extern size_t rps_str_len(const rps_str *);
extern size_t rps_str_sz(const rps_str *);
extern void rps_str_rawcp(char *, const char *, size_t);
__END_DECLS



#endif /* !REFPERSYS_BSD_SRC_LIB_LIB_H */

