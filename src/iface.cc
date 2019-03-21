/*
 * File: refpersys/src/iface.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It implements the
 *      refpersys interface.
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


/* include required header files */
#include <string.h>
#include "util.h"
#include "iface.h"


/******************************************************************************
 * Section: Object ID Serial (rps_serial63)
 ******************************************************************************/


/*
 *      rps_serial63_make() - declared in refpersys/inc/iface.h
 */
extern rps_serial63 rps_serial63_make(void)
{
  rps_serial63 s63 = (rps_serial63) 0;

  /* keep polling rps_random_uint64() until a valid random object
   * ID serial is found; the original implementation used GNOME
   * library's g_random_int() function which returned a 32-bit
   * unsigned integer, and so a bitwise left shift << 32 operation
   * was required followed by a bitwise or mask; however, since
   * rps_random_uint64() returns a uint64_t, I don't think any
   * masking is required; TODO: confirm from Dr. Basile if any
   * bitwise masking operation is required for 63-bit alignment,
   * and whether we need to be concerned about endianness */
  do
    {
      s63 = (rps_serial63) rps_random_uint64();
    }
  while (!rps_serial63_valid(s63));


  /* we're guaranteed to have a valid serial now */
  return s63;
}


/*
 *      rps_serial63_str() - declared in refpersys/inc/iface.h
 */
extern int rps_serial63_str(rps_serial63 s63, char str[])
{
  const char b62d[] = "0123456789"
                      "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char *c;
  uint_fast32_t dpos;

  /* initialise all characters in @str to null  with a leading
   * underscore */
  memset(str, 0, 16);
  str[0] = '_';

  /* for the edge case when @s63 is null, @str is simply set to a
   * double underscore */
  if (!s63)
    {
      str[1] = '-';
      return 2;
    }

  /* convert base 10 representation of @s63 to base 62
   * representation in @str */
  c = str + RPS_SERIAL63_DIGITS;
  while (s63)
    {
      dpos = s63 % RPS_SERIAL63_BASE;
      s63 /= RPS_SERIAL63_BASE;
      *c-- = b62d[dpos];
    }

  /* pad unusused digit places with zero and terminate @str */
  while (c > str)
    {
      *(c--) = '0';
    }
  str[15] = '\0';

  /* return number of bytes written to @str; this is equal to the
   * 11 base 62 digits plus the leading underscore */
  return RPS_SERIAL63_DIGITS + 1;
}


