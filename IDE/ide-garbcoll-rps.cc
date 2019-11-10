/****************************************************************
 * file ide-garbcoll-rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System IDE
 *
 *      It has code for a simple mark&sweep stop-the-world garbage
 *      collector
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

extern "C" const char rpside_garbcoll_gitid[];
extern "C" const char rpside_garbcoll_date[];

#ifndef RPS_GITID
#error RPS_GITID should be set in compilation command
#endif /*no RPS_GITID*/

const char rpside_garbcoll_gitid[] = RPS_GITID;
const char rpside_garbcoll_date[] = __DATE__;

std::vector<Rps_QuasiValueZ*> Rps_QuasiValueZ::_rps_ptrvector{nullptr};
std::mutex Rps_QuasiValueZ::_rps_mtxvector;

Rps_QuasiValueZ::Rps_QuasiValueZ(Rps_Type ty)
    : _rps_type(Rps_Type::None), _rps_qvrank(0)
{
    RPS_ASSERT(ty > Rps_Type::None);
    std::lock_guard<std::mutex> lock(_rps_mtxvector);
    auto vcap = _rps_ptrvector.capacity();
    auto vsiz = _rps_ptrvector.size();
    RPS_ASSERT(vsiz > 0);
    if (RPS_UNLIKELY(vsiz+2 >= vcap)) {
        auto vnewcap = rps_prime_above(vsiz + vsiz/3 + 10) & 0x1f;
        _rps_ptrvector.reserve(vnewcap);
    }
    // we first put this near some random index, or else at end
    const int delta = 8;
    if (RPS_LIKELY(vsiz > 2*delta)) {
        uint32_t maxix = rps_prime_below (vsiz-delta);
        uint32_t randix = Rps_Random::random_32u() % maxix;
        uint32_t upix=randix+delta;
        if (upix>vsiz-1) upix=vsiz-1;
        for (auto ix=randix; ix<upix; ix++)
            if (_rps_ptrvector[ix] == nullptr) {
                _rps_qvrank = ix;
                _rps_ptrvector[ix] = this;
                break;
            }
    }
    if (_rps_qvrank == 0) {
        _rps_ptrvector.push_back(this);
        _rps_qvrank = _rps_ptrvector.size() -1;
    }
    _rps_type = ty;
} // end of Rps_QuasiValueZ::Rps_QuasiValueZ


Rps_QuasiValueZ::~Rps_QuasiValueZ()
{
    std::lock_guard<std::mutex> lock(_rps_mtxvector);
    if (_rps_qvrank >0) {
        RPS_ASSERT(_rps_ptrvector[_rps_qvrank] == this);
        _rps_ptrvector[_rps_qvrank] = nullptr;
    }
} // end of Rps_QuasiValueZ::~Rps_QuasiValueZ


//////////////// end of file ide-garbcoll-rps.cc
