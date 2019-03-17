/*
 * File: refpersys/src/objid.h
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
 *-----------------
 *
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
 *
 *****/




        /* create header guard */
#if (!defined __REFPERSYS_OBJECTID_DEFINED)
#       define __REFPERSYS_OBJECTID_DEFINED

        /* include required header files */
#include <stdint.h>



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
typedef struct rps_object_st rps_object;

typedef struct rps_objid_st rps_objid;

/* TODO: did we really decide that objids are 128 bits? They might be a bit smaller (96 bits)! */
struct rps_objid_st { rps_serial63 oid_hi, oid_low; };

struct rps_object_st {
#warning TODO: rps_object_st is very incomplete
  /* some fields are needed before ob_id */
  const rps_objid ob_id;
  /* many fields are needed after ob_id */
};
 

/*
 *      rps_object_make() - make a fresh new object
 *        - obj: contextual object
 *
 *      The rps_object_make() interface function creates a new object instance
 *      @obj through the factory pattern.
 * 
 *       rps_object_make_with_objid is creating an object with a given
 *       oid and check that the given oid is valid and was not already
 *       used.
 *
 *    On out-of-memory condition (a very rare event) these functions panic and abort.
 *    But both functions are using MPS allocation.
 */
extern rps_object* rps_object_make(void);
extern rps_object* rps_object_make_with_objid(rps_objid oid);




#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

