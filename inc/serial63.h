/*
 * File: refpersys/inc/serial63.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      object ID serial type and its interface.
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




        /* create header guard */
#if (!defined __REFPERSYS_SERIAL63_DEFINED)
#       define __REFPERSYS_SERIAL63_DEFINED

        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
#endif




        /* include required header files */
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#       include <stdint.h>
#else
#       error "building refpersys requires a C99 compiler"
#endif




        /* declare the rps_serial63 type; this is analogous to the Bismon
         * serial64_tyBM type in the header file id_BM.h */
typedef uint64_t rps_serial63;




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif

#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

