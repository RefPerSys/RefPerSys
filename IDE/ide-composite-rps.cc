/****************************************************************
 * file ide-composite-rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System IDE
 *
 *      It has code for composite immutable values
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


#ifndef RPS_GITID
#error RPS_GITID should be set in compilation command
#endif /*no RPS_GITID*/

const char rpside_composite_gitid[] = RPS_GITID;
const char rpside_composite_date[] = __DATE__;


////////////////////////////////////////////////////////////////
//////////////// ordered immutable sets
Rps_ObjectZ* rps_set_class;
Rps_SetValueZ*
Rps_SetValueZ::make(const std::set<Rps_ObjectZ*,Rps_LessObjPtr>&setob)
{
    for (Rps_ObjectZ*ob : setob) {
        RPS_ASSERT(ob != nullptr);
    }
    auto card = (unsigned) setob.size();
    unsigned alsiz = rps_prime_above(card);
    std::vector<Rps_ObjectZ*>vecob(card);
    vecob.reserve(alsiz);
    for (Rps_ObjectZ*ob : setob)
        vecob.push_back(ob);
    Rps_SetValueZ*setv = new(Rps_Gap(alsiz*sizeof(Rps_ObjectZ*)))
    Rps_SetValueZ(alsiz, card, vecob.data());
    return setv;
} // end Rps_SetValueZ::make

void
Rps_SetValueZ::compute_hash(void) {
    uint32_t card = seqlength();
    rps_hashint_t h1=rps_prime_above(card), h2=rps_prime_below(card);
    RPS_FATAL("Rps_SetValueZ::compute_hash unimplemented card=%u",
              (unsigned)card);
#warning Rps_SetValueZ::compute_hash unimplemented
} // end Rps_SetValueZ::compute_hash
//////////////// end of file ide-composite-rps.cc
