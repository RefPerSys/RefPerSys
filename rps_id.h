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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define RPS_B62DIGITS 
    "0123456789"                          \
    "abcdefghijklmnopqrstuvwxyz"          \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


/**
 * Type rps_hashint_t
 */
typedef uint32_t rps_hashint_t;
typedef uint64_t rps_serial63_t; /* but the most significant bit is 0 */



/**
 * Represents an object ID.
 */
typedef struct __rps_id_st {
	rps_serial63_t id_hi;
	rps_serial63_t id_lo;
} rps_id_t;


inline rps_serial63_t
rps_id_hi(const rps_id_t *id)
{
	return id->id_hi;
}


inline rps_serial63_t
rps_id_lo(const rps_id_t *id)
{
	return id->lo;
}



// gives true if conversion from id to char-buffer cbuf succeeded:
extern bool rps_idtocbuf32(rps_rawid_t id, char cbuf[static 32]);

#define RPS_ID_AS_CBUF_CNTBIS(Id,Cnt) ({			\
      static thread_local cbuf_##Cnt[32];			\
      rps_rawid_t id_##Cnt = (Id);				\
      memset(cbuf_##Cnt, 0, sizeof(cbuf_##Cnt));		\
      rps_idtocbuf32(id_##Cnt, cbuf_##Cnt)?cbuf_##Cnt:"???"; })

#define RPS_ID_AS_CBUF_CNT(Id,Cnt)  RPS_ID_AS_CBUF_CNTBIS(Id,Cnt)  
#define RPS_ID_AS_CBUF(Id) RPS_ID_AS_CBUF_CNT(Id,__COUNT__)

#endif /*RPS_ID_INCLUDED*/
//////////////////////////////////////// end of file rps_id.h
