/****************************************************************
 * File: rps_id.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Internal file for our object ids
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.org>
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


#ifndef RPS_ID_INCLUDED
#define RPS_ID_INCLUDED


/* enable access to non-standard GNU extension functions
 * see https://stackoverflow.com/questions/5582211/ */
/* We explicit want to use all GNU extensions */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdint.h>
#include <stdbool.h>
#include "rps_hints.h" /* TODO: merge this file later into refpersys.h */


#define RPS_B62DIGITS                     \
    "0123456789"                          \
    "abcdefghijklmnopqrstuvwxyz"          \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


/* represents an integer hash */
typedef uint32_t rps_hashint_t;



typedef uint64_t rps_serial63_t; /* but the most significant bit is 0 */

/* the minimum high order value of an rps_serial63_t type */
#define RPS_SERIAL63_HI_MIN ((rps_serial63_t) 62 * 62 * 62)

/* the maximum high order value of an rps_serial63_t type */
#define RPS_SERIAL63_HI_MAX                                         \
	((rps_serial63_t) 10 * 62 * (62 * 62 * 62) * (62 * 62 * 62) \
	 * (62 * 62 * 62))

/* the minimum low order value of an rps_serial63_t type */
#define RPS_SERIAL63_LO_MIN ((rps_serial63_t) 62 * 62)

/* the maximum low order value of an rps_serial63_t type */
#define RPS_SERIAL63_LO_MAX \
	((rps_serial63_t) 62 * (62 * 62 * 62) * (62 * 62 * 62))


/* the maximum number of buckets in an rps_serial63_t type */
/* TODO: explain buckets */
#define RPS_SERIAL63_BUCKET_MAX ((uint64_t) 10 * 62)


/* generates a new random rps_serial63_t value */
extern RPS_HOT rps_serial63_t
rps_serial63_random(void);


/* flag to indicate comparison result; something similar to strcmp result */
typedef int rps_cmpflag_t;

/* indicates a less-than comparison result ; any integer < 0 is ok*/
#define RPS_CMPFLAG_LT ((rps_cmpflag_t) -1)

/* indicates an equality comparison result */
#define RPS_CMPFLAG_EQ ((rps_cmpflag_t) 0)

/* indicates a greater-than comparison result ; any integer > 0 is ok */
#define RPS_CMPFLAG_GT ((rps_cmpflag_t) 1)


/* gets the number number of buckets in an rps_serial63_t type */
/* TODO: explain buckets */
inline RPS_PURE uint64_t
rps_serial63_buckets(const uint64_t s)
{
  return s / (RPS_SERIAL63_HI_MAX / RPS_SERIAL63_BUCKET_MAX);
}


/* represents an object ID; note that it's more efficient to pass and return
 * two-word structs than their pointers on the x86_64 Linux ABI. rps_serial63_t
 * is a native word on x86_64 since it is guaranteed to be 64-bits wide. */
typedef struct rps_id_st
{
  rps_serial63_t _hi;
  rps_serial63_t _lo;
} rps_id_t;


/* generates a new random object ID */
extern RPS_HOT rps_id_t
rps_id_new(void);


/* gets the high order bits of an object ID */
inline rps_serial63_t
rps_id_hi(const rps_id_t id)
{
  return id._hi;
}


/* gets the low order bits of an object ID */
static inline rps_serial63_t
rps_id_lo(const rps_id_t id)
{
  return id._lo;
}


/* gets the number of buckets for an object ID */
/* TODO: explain the significance of buckets */
inline uint64_t
rps_id_buckets(const rps_id_t id)
{
  return rps_serial63_buckets (id._hi);
}


/* checks whether an object ID is empty */
inline bool
rps_id_empty(const rps_id_t id)
{
  return id._hi == 0 && id._lo == 0;
}


/* gets the integer hash of an object ID */
/* The magic numbers below a prime, on purpose, but choosen
   arbitrarily. We want something fast! */
inline rps_hashint_t
rps_id_hash(const rps_id_t id)
{
  return (id._hi % 2147473837)
         + ((id._hi >> 32) ^ (id._lo * 17 + 201151));
}


/* checks whether an object ID is valid */
inline bool
rps_id_valid(const rps_id_t id)
{
  return id._hi >= RPS_SERIAL63_HI_MIN
         && id._hi < RPS_SERIAL63_HI_MAX
         && id._lo >= RPS_SERIAL63_LO_MIN
         && id._lo < RPS_SERIAL63_LO_MAX
         && rps_id_hash (id) != 0;
}


inline rps_cmpflag_t
rps_id_cmp(const rps_id_t lhs, const rps_id_t rhs)
{
  if (lhs._hi == rhs._hi)
    {
      if (lhs._lo == rhs._lo)
        return RPS_CMPFLAG_EQ;
      else if (lhs._lo < rhs._lo)
        return RPS_CMPFLAG_LT;
      else
        return RPS_CMPFLAG_GT;
    }

  else if (lhs._hi < rhs._hi)
    {
      return RPS_CMPFLAG_LT;
    }

  else
    {
      return RPS_CMPFLAG_GT;
    }
}


/* checks if an object ID is less than another one; this is a convenience
 * wrapper around rps_id_cmp() */
inline bool
rps_id_lt(const rps_id_t lhs, const rps_id_t rhs)
{
  return rps_id_cmp (lhs, rhs) < 0;
}


/* checks if an object ID is less than or equal to another one; this is a
 * convenience wrapper around rps_id_cmp() */
inline bool
rps_id_lteq(const rps_id_t lhs, const rps_id_t rhs)
{
  return rps_id_cmp (lhs, rhs) <= 0;
}


/* checks if an object ID is equal to another one; this is a convenience wrapper
 * around rps_id_cmp() */
inline bool
rps_id_eq(const rps_id_t lhs, const rps_id_t rhs)
{
  return rps_id_cmp (lhs, rhs) == RPS_CMPFLAG_EQ;
}


/* checks if an object ID is greater than another one; this is a convenience
 * wrapper around rps_id_cmp() */
inline bool
rps_id_gt(const rps_id_t lhs, const rps_id_t rhs)
{
  return rps_id_cmp (lhs, rhs) > 0;
}


/* checks if an object ID is greater than or equal to another one; this is a
 * convenience wrapper around rps_id_cmp() */
inline bool
rps_id_gteq(const rps_id_t lhs, const rps_id_t rhs)
{
  return rps_id_cmp (lhs, rhs) > 0;
}

// gives true if conversion from id to char-buffer cbuf succeeded:
extern bool rps_idtocbuf32(rps_id_t id, char cbuf[static 32]);

#define RPS_ID_AS_CBUF_CNTBIS(Id,Cnt) ({			\
      static thread_local cbuf_##Cnt[32];			\
      rps_rawid_t id_##Cnt = (Id);				\
      memset(cbuf_##Cnt, 0, sizeof(cbuf_##Cnt));		\
      rps_idtocbuf32(id_##Cnt, cbuf_##Cnt)?cbuf_##Cnt:"???"; })

#define RPS_ID_AS_CBUF_CNT(Id,Cnt)  RPS_ID_AS_CBUF_CNTBIS(Id,Cnt)
#define RPS_ID_AS_CBUF(Id) RPS_ID_AS_CBUF_CNT(Id,__COUNT__)

#endif /*RPS_ID_INCLUDED*/
//////////////////////////////////////// end of file rps_id.h
