/*
 * File: refpersys/src/serial63.c
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It implements the
 *      object ID serial type interface.
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
 *      Released under the GNU General Public License version 3 (GPLv3)
 *      <http://opensource.org/licenses/GPL-3.0>. See the accompanying LICENSE
 *      file for complete licensing details.
 *
 *      BY CONTINUING TO USE AND/OR DISTRIBUTE THIS FILE, YOU ACKNOWLEDGE THAT
 *      YOU HAVE UNDERSTOOD THESE LICENSE TERMS AND ACCEPT THEM.
 */




        /* include required header files */
#include "../inc/serial63.h"




        /* this extern will be removed once the rps_random_uint64() function is
         * ready; as of now, it's needed for rps_stream_new()  */
extern uint64_t rps_random_uint64(void);




/*
 *      rps_serial63_new() - declared in refpersys/src/serial63.h
 */
extern rps_serial63 rps_serial63_new(void)
{
        register rps_serial63 s63 = (rps_serial63) 0;

                /* keep polling rps_random_uint64() until a valid random object
                 * ID serial is found; the original implementation used GNOME
                 * library's g_random_int() function which returned a 32-bit
                 * unsigned integer, and so a bitwise left shift << 32 operation
                 * was required followed by a bitwise or mask; however, since
                 * rps_random_uint64() returns a uint64_t, I don't think any
                 * masking is required; TODO: confirm from Dr. Basile if any
                 * bitwise masking operation is required for 63-bit alignment,
                 * and whether we need to be concerned about endianness */
        do {
                s63 = (rps_serial63) rps_random_uint64();
        } while (!rps_serial63_valid(s63));


                /* we're guaranteed to have a valid serial now */
        return s63;
}

