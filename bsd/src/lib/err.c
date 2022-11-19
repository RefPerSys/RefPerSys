#include <assert.h> /* assert(3) */
#include <errno.h>  /* errno(3)  */
#include <stdio.h>  /* perror(3) */
#include <stdlib.h> /* exit(3)   */

#include "lib.h"


void
rps_err_fatal(int erno, const char *msg)
{
	assert(erno);
	assert(msg);
	assert(*msg);

	errno = erno;
	perror(msg);
	exit(erno);	
}

