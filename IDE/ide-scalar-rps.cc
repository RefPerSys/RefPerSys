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


////////////////////////////////////////////////////////////////
//////////////// strings
Rps_ObjectZ* rps_string_class;
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

////////////////////////////////////////////////////////////////
//////////////// slow boxed integers
Rps_ObjectZ* rps_int_class;
Rps_IntegerValueZ*
Rps_IntegerValueZ::make(int64_t ival) {
    return new(rps_plain_tag{}) Rps_IntegerValueZ(ival);
} // end of Rps_IntegerValueZ::make

Hjson::Value
Rps_IntegerValueZ::serialize(void) {
    //    return Hjson::Value(ival());
} // end Rps_IntegerValueZ::serialize

Rps_IntegerValueZ::~Rps_IntegerValueZ() {
} // end of Rps_IntegerValueZ::~Rps_IntegerValueZ

void Rps_IntegerValueZ::display(Rps_Displayer&disp, unsigned depth)
{
    RPS_FATAL("unimplemented Rps_IntegerValueZ::display");
#warning unimplemented Rps_IntegerValueZ::display
} // end of Rps_IntegerValueZ::display

////////////////////////////////////////////////////////////////
//////////////// slow boxed doubles
Rps_ObjectZ* rps_double_class;
Rps_DoubleValueZ*
Rps_DoubleValueZ::make(double dval) {
    if (std::isnan (dval)) return nullptr;
    return new(rps_plain_tag{}) Rps_DoubleValueZ(dval);
} // end of Rps_DoubleValueZ::make

rps_hashint_t
Rps_DoubleValueZ::hash() const {
    RPS_ASSERT(!std::isnan(_dval));
    std::size_t hs = std::hash<double> {}(_dval);
    rps_hashint_t h = (rps_hashint_t) hs; // the lowest 32 bits
    if (RPS_UNLIKELY(h == 0)) {
        h = (hs % 16766119) + 24;
    }
    return h;
} // end of Rps_DoubleValueZ::hash

Hjson::Value
Rps_DoubleValueZ::serialize(void) {
    return Hjson::Value(dval());
} // end Rps_DoubleValueZ::serialize

Rps_DoubleValueZ::~Rps_DoubleValueZ() {
} // end of Rps_DoubleValueZ::~Rps_DoubleValueZ

void Rps_DoubleValueZ::display(Rps_Displayer&disp, unsigned depth)
{
    RPS_FATAL("unimplemented Rps_DoubleValueZ::display");
#warning unimplemented Rps_DoubleValueZ::display
} // end of Rps_DoubleValueZ::display

//////////////// end of file ide-scalar-rps.cc
