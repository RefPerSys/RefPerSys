/****************************************************************
 * file ide-scalar-rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System IDE
 *
 *      It has code for scalar values
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org
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

#include "ide-refpersys.hh"


Rps_StringValueZ*
Rps_StringValueZ::make(const char*str)
{
    if (RPS_UNLIKELY(str==nullptr || str==RPS_EMPTYSLOT)) return nullptr;
    size_t bsiz = strlen(str);
    if (RPS_UNLIKELY(nullptr != u8_check((uint8_t*)str, bsiz)))
        RPS_FATAL("invalid UTF-8 string starting with %.20s", str);
    rps_hashint_t h = rps_cstring_hash(str);
    return new(Rps_Gap(uint64_t(rps_prime_above(bsiz+1)&~3))) Rps_StringValueZ(h,str);
} // end of Rps_StringValueZ::make

Hjson::Value Rps_StringValueZ::serialize(void) {
    return Hjson::Value(cbytes());
} // end Rps_StringValueZ::serialize

Rps_StringValueZ::~Rps_StringValueZ() {
} // end of Rps_StringValueZ::~Rps_StringValueZ



void Rps_StringValueZ::display(Rps_Displayer&disp, unsigned depth)
{
    RPS_FATAL("unimplemented Rps_StringValueZ::display");
#warning unimplemented Rps_StringValueZ::display
} // end of Rps_StringValueZ::display

//////////////// end of file ide-scalar-rps.cc
