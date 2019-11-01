/****************************************************************
 * file rps_id.h
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


#ifndef RPS_ID_INCLUDED

/* enable access to non-standard GNU extension functions
 * see https://stackoverflow.com/questions/5582211/ */
/* TODO: ask about which extension functions would be required */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

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


/* flag to indicate comparison result */
typedef int rps_cmpflag;

/* indicates a less-than comparison result */
#define RPS_CMPFLAG_LT ((rps_cmpflag) -1)

/* indicates an equality comparison result */
#define RPS_CMPFLAG_EQ ((rps_cmpflag) 0)

/* indicates a greater-than comparison result */
#define RPS_CMPFLAG_GT ((rps_cmpflag) 1)


/* represents and object ID */
typedef struct __rps_id_st {
	rps_serial63_t _hi;
	rps_serial63_t _lo;
} rps_id_t;


/* gets the high order bits of an object ID */
inline rps_serial63_t
rps_id_hi(const rps_id_t *id)
{
	return id->_hi;
}


/* gets the low order bits of an object ID */
inline rps_serial63_t
rps_id_lo(const rps_id_t *id)
{
	return id->_lo;
}


/* checks whether an object ID is empty */
/* TODO: is rps_id_null better terminology? */
inline bool
rps_id_empty(const rps_id_t *id)
{
	return id->_hi == 0 && id->_lo == 0;
}


/* gets the integer hash of an object ID */
/* TODO: remove magic numbers or explain them */
inline rps_hashint_t
rps_id_hash(const rps_id_t *id)
{
	return (id->_hi % 2147473837) 
		+ ((id->_hi >> 32) ^ (id->_lo * 17 + 201151));
}


/* checks whether an object ID is valid */
inline bool
rps_id_valid(const rps_id_t *id)
{
	return id->_hi >= RPS_SERIAL63_HI_MIN
		&& id->_hi < RPS_SERIAL63_HI_MAX
		&& id->_lo >= RPS_SERIAL63_LO_MIN
		&& id->_lo < RPS_SERIAL63_LO_MAX
		&& rps_id_hash (id) != 0;
}


/* compares two object IDs */
inline rps_cmpflag
rps_id_cmp(const rps_id_t *lhs, const rps_id_t *rhs)
{
	if (lhs->_hi == rhs->_hi) {
		if (lhs->_lo == rhs->_lo)
			return RPS_CMPFLAG_EQ;
		else if (lhs->_lo < rhs->_lo)
			return RPS_CMPFLAG_LT;
		else
			return RPS_CMPFLAG_GT;
	}

	else if (lhs->_hi < rhs->_hi) {
		return RPS_CMPFLAG_LT;
	}

	else {
		return RPS_CMPFLAG_GT;
	}
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
