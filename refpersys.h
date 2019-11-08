/****************************************************************
 * file refpersys.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its only C header file.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef REFPERSYS_INCLUDED

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <math.h>
#include <dlfcn.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <netdb.h>
#include <locale.h>
#include <crypt.h>
#include <string.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <backtrace.h>
#include <regex.h>

#include <unistr.h>
/* IMPORTANT TODO: Uncomment later after fixing issue */
/* #include <glib.h> */

/// Glibc dont have yet threads.h
#define thread_local _Thread_local

/// naming conventions: all our API names start with RPS in upper,
/// lower, or mixed cases; the GTK naming conventions are very nice to
/// read, and are inspirational. However, we explicitly require a C11
/// compiler, so a recent GCC like at least GCC-8.


#include "rps_hints.h"


/* aborts after displaying a fatal error message */
#define RPS_FATAL(msg, ...)                                  \
do {                                                         \
  fprintf (stderr, "FATAL ERROR in %s [%s:%d]: " msg "\n\n", \
	   __func__, __FILE__, __LINE__, ##__VA_ARGS__);     \
  abort ();                                                  \
} while (0)


#include "rps_id.h"
#warning code should be written here.

#endif /*REFPERSYS_INCLUDED*/
//////////////////////////////////////// end of file refpersys.h
