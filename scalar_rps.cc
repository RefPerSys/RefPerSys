/****************************************************************
 * file scalar_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to scalar values
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
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

#include "refpersys.hh"

extern "C" const char rps_scalar_gitid[];
const char rps_scalar_gitid[]= RPS_GITID;

extern "C" const char rps_scalar_date[];
const char rps_scalar_date[]= __DATE__;

Rps_QuasiZone::~Rps_QuasiZone()
{
}

int rps_compute_cstr_two_64bits_hash(int64_t ht[2], const char*cstr, int len)
{
  if (!ht || !cstr)
    return 0;
  if (len < 0)
    len = strlen(cstr);
  ht[0] = 0;
  ht[1] = 0;
  if (len == 0)
    return 0;
  int64_t h0=len, h1=60899;
  const char*end = cstr + len;
  int utf8cnt = 0;
  for (const char*pc = cstr; pc < end; )
    {
      ucs4_t uc1=0, uc2=0, uc3=0, uc4=0;
      int l1 = u8_mbtouc(&uc1, (const uint8_t*)pc, end - pc);
      if (l1<0)
        return 0;
      utf8cnt ++;
      pc += l1;
      if (pc >= end)
        break;
      h0 = (h0 * 60869) ^ (uc1 * 5059 + (h1 & 0xff));
      int l2 = u8_mbtouc(&uc2, (const uint8_t*)pc, end - pc);
      if (l2<0)
        return 0;
      h1 = (h1 * 53087) ^ (uc2 * 43063 + utf8cnt + (h0 & 0xff));
      utf8cnt ++;
      pc += l2;
      if (pc >= end)
        break;
      int l3 = u8_mbtouc(&uc3, (const uint8_t*)pc, end - pc);
      if (l3<0)
        return 0;
      h1 = (h1 * 73063) ^ (uc3 * 53089 + (h0 & 0xff));
      utf8cnt ++;
      pc += l3;
      if (pc >= end)
        break;
      int l4 = u8_mbtouc(&uc4, (const uint8_t*)pc, end - pc);
      if (l4<0)
        return 0;
      h0 = (h0 * 73019) ^ (uc4 * 23057 + 11 * (h1 & 0x1ff));
      utf8cnt ++;
      pc += l4;
      if (pc >= end)
        break;
    }
  ht[0] = h0;
  ht[1] = h1;
  return utf8cnt;
} // end of rps_compute_cstr_two_64bits_hash


Rps_String*
Rps_String::make(const char*cstr, int len)
{
  cstr = normalize_cstr(cstr);
  len = normalize_len(cstr, len);
  if (u8_check(reinterpret_cast<const uint8_t*>(cstr), len))
    throw std::domain_error("invalid UTF-8 string");
  Rps_String* str
    = rps_allocate_with_wordgap<Rps_String> (len/sizeof(void*)+1, cstr, len);
  return str;
} // end of Rps_String::make


//////////////////////////////////////////////// end of file scalar_rps.cc
