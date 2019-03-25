/*
 * File: refpersys/inc/util.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      helper utilities interface.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Niklas Rosencrantz <niklasro@gmail.com>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 * Copyright:
 *      (c) 2019 The Reflective Persistent System Team
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
 */

#ifndef REFPERSYS_UTIL_INCLUDED
#define REFPERSYS_UTIL_INCLUDED

/* include required header files */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* in generated _timestamp.c */
extern "C" const char rps_git_commit[];
extern "C" const char rps_build_timestamp[];

/* Mersenne twister */
extern "C" uint64_t rps_random_uint64(void);



// branch prediction macro to indicate unlikely condition
// adapted from MOM_UNLIKELY() macro in meltmoni.hh
#define rps_unlikely(p) __builtin_expect(!!(p),0)


// branch prediction macro to indicate likely condition
// adapted from MOM_LIKELY() macro in meltmoni.hh
#define rps_likely(p) !__builtin_expect(!(p),0)


// prints an error message indicating MPS reservation failure
static inline void rps_perror_mps_reserve(size_t size)
{
  char errmsg[100];

  snprintf(errmsg, sizeof(errmsg),
           "MPS error: unable to reserve %lu bytes",
           size);
  perror(errmsg);
}


#endif /* REFPERSYS_UTIL_INCLUDED */

