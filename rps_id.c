/****************************************************************
 * File: rps_id.c
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Internal implementation file for our object ids
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

#include "rps_id.h"


/******************************************************************************
 * Inline rps_serial63_t Declarations
 */


extern inline rps_pure uint64_t
rps_serial63_buckets(const uint64_t s);


/******************************************************************************
 * Inline rps_id_t Declarations
 */


extern inline rps_serial63_t
rps_id_hi(const rps_id_t *id);

extern inline rps_serial63_t
rps_id_lo(const rps_id_t *id);

extern inline uint64_t
rps_id_buckets(const rps_id_t *id);

extern inline bool
rps_id_empty(const rps_id_t *id);

extern inline rps_hashint_t
rps_id_hash(const rps_id_t *id);

extern inline rps_cmpflag
rps_id_cmp(const rps_id_t *lhs, const rps_id_t *rhs);

extern inline bool
rps_id_lt(const rps_id_t *lhs, const rps_id_t *rhs);

extern inline bool
rps_id_lteq(const rps_id_t *lhs, const rps_id_t *rhs);

extern inline bool
rps_id_eq(const rps_id_t *lhs, const rps_id_t *rhs);

extern inline bool
rps_id_gt(const rps_id_t *lhs, const rps_id_t *rhs);

extern inline bool
rps_id_gteq(const rps_id_t *lhs, const rps_id_t *rhs);


/******************************************************************************
 * rps_id_t Implementation
 */


extern rps_hot rps_cmpflag
rps_id_cmp(const rps_id_t *lhs, const rps_id_t *rhs)
{
  if (lhs->_hi == rhs->_hi)
    {
      if (lhs->_lo == rhs->_lo)
        return RPS_CMPFLAG_EQ;
      else if (lhs->_lo < rhs->_lo)
        return RPS_CMPFLAG_LT;
      else
        return RPS_CMPFLAG_GT;
    }

  else if (lhs->_hi < rhs->_hi)
    {
      return RPS_CMPFLAG_LT;
    }

  else
    {
      return RPS_CMPFLAG_GT;
    }
}

