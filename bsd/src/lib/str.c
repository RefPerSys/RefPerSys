#include <assert.h> /* assert(3) */
#include <stdio.h>  /* fputs(3) */
#include <stdlib.h> /* malloc(3), exit(3) */
#include <string.h> /* strlen(3), strncpy(3), strlcpy(3), strcmp(3) */

#include "lib.h"


rps_str *
rps_str_new(const char *src)
{
	size_t sz;
	str *s;
	
	assert(src);
	sz = strlen(src) + 1;
	s = rps_mem_new(sz);

        rps_str_rawcp(s, src, sz);
	return s;
}


rps_str *
rps_str_new_raw(size_t sz)
{
	str *s;
	
	assert(sz);
	s = rps_mem_new(sz);
	return s;
}


void
rps_str_free(rps_str **s)
{
	if (__predict_true(s && *s)) {
		rps_mem_free(*s);
		*s = NULL;
	}
}


int
rps_str_cmp(const rps_str *s, const char *cmp)
{
	assert(s);
	assert(cmp);
	return strcmp(s, cmp);
}


size_t
rps_str_len(const rps_str *s)
{
	assert(s);
	return strlen(s);
}


size_t
rps_str_sz(const rps_str *s)
{
	assert(s);
	return strlen(s) + 1;
}

void
rps_str_rawcp(char *dst, const char *src, size_t sz)
{
	assert(dst);
	assert(src);
	assert(sz);
        (void)strlcpy(dst, src, sz);
}

