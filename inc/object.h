/*
 * File: refpersys/inc/objid.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      object ype and its interface.
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
#if (!defined __REFPERSYS_OBJECTID_DEFINED)
#       define __REFPERSYS_OBJECTID_DEFINED




        /* include required header files */
#include <stdint.h>




        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
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




/*
 *      rps_object_spawn() - spawns new object
 *        - obj: contextual object
 *
 *      The rps_object_spawn() interface function creates a new object instance
 *      @obj through the factory pattern.
 *
 *      See:
 *        - bismon makeobj_BM()
 *        - bismon_makeobjofid_BM()
 */
extern void rps_object_spawn(rps_object *obj);




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

