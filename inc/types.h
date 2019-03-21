/*
 * File: refpersys/inc/types.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      refpersys types and their associated constants.
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


/* create header guard */
#if (!defined __REFPERSYS_TYPES_DEFINED)
#       define __REFPERSYS_TYPES_DEFINED


/* include required header files */
#include <stdint.h>


/* open support for C++ */
#if (defined __cplusplus)
extern "C" {
#endif


  /****************************************************************************
   * Section: Object ID Serial (rps_serial63)
   ****************************************************************************/


  /* declare the rps_serial63 type; this is analogous to the Bismon
   * serial64_tyBM type in the header file id_BM.h */
  typedef uint64_t rps_serial63;


  /* number of digits in an object ID serial when represented the form of
   * base 62 digits */
#define RPS_SERIAL63_DIGITS 11


  /* object ID serials are represented compactly in base 62 */
#define RPS_SERIAL63_BASE 62


  /*
   *      RPS_SERIAL63_MIN - minimum object ID serial value
   *
   *      The RPS_SERIAL63_MIN symbolic constant defines the minimum value of an
   *      object ID serial.
   *
   *      TODO: explain why it is 62 * 62
   */
#define RPS_SERIAL63_MIN ((uint64_t) 3884)


  /*
   *      RPS_SERIAL63_MAX - maximum object ID serial value
   *
   *      The RPS_SERIAL63_MAX symbolic constant defines the maximum value of an
   *      object ID serial.
   *
   *      TODO: explain why it is 10 * 62 * (62* 62*62) * (62*62*62) * (62*62*62)
   */
#define RPS_SERIAL63_MAX ((uint64_t) 8392993658683402240)


  /*
   *      RPS_SERIAL63_DELTA - delta of object ID serial maxima and minima
   *
   *      The RPS_SERIAL63_DELTA symbolic constant defines the difference between
   *      the the maxiumum and minimum values of an object ID serial.
   *
   *      TODO: explain why it is RPS_SERIAL63_MAX - RPS_SERIAL63_MIN
   */
#define RPS_SERIAL63_DELTA (RPS_SERIAL63_MAX - RPS_SERIAL63_MIN)


  /****************************************************************************
   * Section: Object Bucket (rps_bucket)
   ****************************************************************************/


  /*
   *      RPS_BUCKET_MAX - maximum object buckets
   *
   *      The RPS_BUCKET_MAX symbolic constant defines the maximum object buckets.
   *      TODO: explain why it is 10 * 62, and its significance
   */
#define RPS_BUCKET_MAX ((size_t) 620)


  /****************************************************************************
   * Section: Object ID
   ****************************************************************************/


  /* TODO: object IDs are currently 128 bits, but may be reduced down to
   * 96 bits; accordingly, the rps_serial63 type will also need to be
   * redefined */
  typedef struct rps_objid_st
  {
    rps_serial63 hi;
    rps_serial63 lo;
  } rps_objid;


  /****************************************************************************
   * Section: Object
   ****************************************************************************/


  /* TODO: the object type is still incomplete, and its member fields are
   * not fully defined yet */
  typedef struct rps_object_st
  {
#warning "TODO: rps_object_st is still incomplete"
    /* some fields are needed before objid */
    const rps_objid objid;
    /* many fields are needed after objid */
  } rps_object;


  /****************************************************************************
   * Section: Typed Types (WIP)
   ****************************************************************************/

  /* corresponds to gctyenum_BM; enumerates garbage collected types of
   * refpersys */
  typedef enum RPS_GCTYPE_ENUM {
#warning TODO: RPS_GCTYPE_ENUM is still incomplete
        RPS_GCTYPE_INT = -1,
        RPS_GCTYPE_NONE = 0
        /* many more enumerators after this */
  } RPS_GCTYPE;


/* corresponds to typedhead_tyBM */
typedef struct rps_typedhead_st {
        RPS_GCTYPE gctype:24;
        uint32_t gchead:8;
        union {
                rps_hash hash;
                uint32_t rlen;
        };
} rps_typedhead;

  /* corresponds to typedsize_tyBM */
  typedef struct rps_typedsz_st {
          rps_typedhead head;
          uint32_t size;
  } rps_typedsz;


  /* corresponds to typedforward_tyBM */
  typedef struct rps_typedfwd_st {
          rps_typedsz size;
          void *forward;
  } rps_typedfwd;


  /****************************************************************************
   * Section: Object Value
   ****************************************************************************/


  /* corresponds to Bismon's seqobval_stBM */
  typedef struct rps_value_st
  {
    rps_typedfwd forward;
    const rps_object *objects[];
  } rps_value;


  /****************************************************************************
   * Section: Object Value Tuple
   ****************************************************************************/


  /* corresponds to tupleval_tyBM */
  typedef rps_value rps_valuetuple;


  /****************************************************************************
   * Section: Object Value Set
   ****************************************************************************/


  /* corresponds to setval_tyBM */
  typedef rps_value rps_valueset;


  /* close support for C++ */
#if (defined __cplusplus)
}
#endif


#endif /* (!defined __REFPERSYS_TYPES_DEFINED) */

