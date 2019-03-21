/*******************************************************************************
 * File: refpersys/inc/iface.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      refpersys interface callable units.
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
 ******************************************************************************/


/* create header guard */
#if (!defined __REFPERSYS_IFACE_DEFINED)
#       define __REFPERSYS_IFACE_DEFINED


/* include required header files */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "types.h"


/*  open support for C++ */
#if (defined __cplusplus)
extern "C" {
#endif


  /****************************************************************************
   * Section: Object ID Serial (rps_serial63)
   ****************************************************************************/


  /*
   *      rps_serial63_make() - generates a new random object ID serial
   *
   *      TODO: elaborate detailed description
   *
   *      Return:
   *        - new random object ID serial
   *
   *      See:
   *        - randomserial63_BM() in Bismon's id_BM.h
   */
  extern rps_serial63 rps_serial63_make(void);


  /*
   *      rps_serial63_valid() - checks validity of object ID serial
   *        - s63: contextual object ID serial
   *
   *      TODO: elaborate detailed description
   *
   *      Return:
   *        - true if @s63 is valid
   *        - false if @s63 is invalid
   *
   *      See:
   *        - validserial63_BM() in Bismon's id_BM.h
   */
  static inline bool rps_serial63_valid(rps_serial63 s63)
  {
    /* @s63 must fall within the permissible maximum and minimum
     * values for object ID serials */
    return (s63 > RPS_SERIAL63_MIN && s63 < RPS_SERIAL63_MAX);
  }


  /* rps_serial63_bucket_fit() is analogous to bucknumserial63_BM(); I'm
   * guessing that this function returns the number of object buckets that
   * can fit within @s63; TODO: confirm from Dr. Basile */
  static inline size_t rps_serial63_bucket_fit(rps_serial63 s63)
  {
    return s63 / (RPS_SERIAL63_DELTA / RPS_BUCKET_MAX);
  }


  /* rps_serial63_bucket_no_fit() is analogous to buckoffnumserial63_BM();
   * again, I'm assuming that this function returns the number of object
   * buckets that **don't** fit within @s63, since the modulo operator
   * returns the remainder; TODO: confirm from Dr. Basile */
  static inline size_t rps_serial63_bucket_no_fit(rps_serial63 s63)
  {
    return s63 % (RPS_SERIAL63_DELTA / RPS_BUCKET_MAX);
  }


  /*
   *      rps_serial63_str() - stringifies an object ID serial
   *        - s63: contextual object ID serial
   *        - str: 16-byte string representation
   *
   *      TODO: elaborate detailed description, especially on static array size
   *      feature of C99
   *
   *      Return:
   *        - Number of bytes written to @str
   *
   *      See:
   *        - serial63tocbuf16_BM() in Bismon's id_BM.h
   */
  extern int rps_serial63_str(rps_serial63 s63, char str[]);


  /*
   *      rps_serial63_parse() - parses string to an object ID serial
   *        - bfr: TODO: figure it out
   *        - pend: TODO: figure it out
   *
   *      TODO: elaborate detailed description
   *
   *      Return:
   *        - parsed object ID serial
   *
   *      See:
   *        - parse_serial63_BM() in Bismon's id_BM.h
   */
  extern rps_serial63 rps_serial63_parse(const char *bfr, const char **pend);


  /****************************************************************************
   * Section: Object
   ****************************************************************************/


  /***
   * The `rps_object_make` function is making a fresh empty object with
   * a unique and random oid.
   *
   * The `rps_object_make_with_objid` is making an object of a given
   * valid oid. If trhe oid is not valid, it returns null; if the oid is
   * the objid of some existing object, it returns that existing object.
   *
   * Both functions are doing some MPS allocation, in an AMS pool.
   ***/
  extern rps_object* rps_object_make(void);
  extern rps_object* rps_object_make_with_objid(rps_objid oid);


  /***
   * The ̀rps_object_find_by_objid` function is finding an existing object of
   * given oid. When the oid is invalid, or when no such object exists,
   * null is returned. That function does not allocate anything.
   ***/
  extern rps_object* rps_object_find_by_objid(rps_objid oid);

#if (defined __cplusplus)
}
#endif


#endif /* (!defined __REFPERSYS_IFACE_DEFINED) */

