/*
 * File: refpersys/inc/object.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      object type and its interface.
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
#if (!defined __REFPERSYS_OBJECT_DEFINED)
#       define __REFPERSYS_OBJECT_DEFINED

        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
#endif




        /* include required header files */
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#       include <stdbool.h>
#else
#       error "building refpersys requires a C99 compiler"
#endif




        /* forward-declare the object type; this type is analogous to bismon's
         * object_stBM struct; I'm guessing that the object_stBM struct is
         * treated as an opaque type through the objectval_tyBM type since doing
         * a grep -r "object_stBM" on the bismon source code root directory
         * doesn't seem to indicate it being used in any file other than the
         * types_BM.h header; accordingly, I'm deferring (perhaps incorrectly)
         * the definition of the rps_object type to the src/object.c file which
         * will implement the object type interface's callable units */
typedef struct __rps_object rps_object;




        /* declare the rps_object_new() constructor; this would be analogous to
         * bismon's makeobj_BM() function which is defined in the object_BM.c
         * source file; I'm deliberately following the convention of returning
         * an error code from every function that may possibly fail; need to
         * confirm from Dr. Basile if this would be the right approach, and if
         * not, understand why so; I also have to figure out how this function
         * is related to the makeobjofid_BM() function declared in the
         * fundec1_BM.h header file */
extern bool rps_object_new(rps_object **obj);




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif

#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

