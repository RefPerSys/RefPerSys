/****************************************************************
 * file oid_rps.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its internal C++ header file for object-identifiers (oid-s).
 *
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2019 - 2025 The Reflective Persistent System Team
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



#ifndef OID_RPS_INCLUDED
#define OID_RPS_INCLUDED

class Rps_Id
{
#ifndef RPS_ONLY_ID_CODE
  RPS_FRIEND_CLASS(ObjectRef);
  RPS_FRIEND_CLASS(ObjectZone);
#endif /*RPS_ONLY_ID_CODE*/
  uint64_t _id_hi;
  uint64_t _id_lo;
public:
  static constexpr const char b62digits[] =
    "0123456789"                          \
    "abcdefghijklmnopqrstuvwxyz"          \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr unsigned buflen = 24;
  static constexpr unsigned base = sizeof(b62digits)-1;
  static constexpr uint64_t min_hi = 62*62*62;
  static constexpr uint64_t max_hi = /// 8392993658683402240, about 8.392994e+18
    (uint64_t)10 * 62 * (62 * 62 * 62) * (62 * 62 * 62) * (62 * 62 * 62);
  static constexpr unsigned nbdigits_hi = 11;
  static constexpr uint64_t delta_hi = max_hi - min_hi;
  static constexpr uint64_t min_lo = 62*62;
  static constexpr uint64_t max_lo = (uint64_t)62 * (62L * 62 * 62) * (62 * 62 * 62); /// about 3.52161e+12
  static constexpr uint64_t delta_lo = max_lo - min_lo;
  static constexpr unsigned nbdigits_lo = 7;
  static constexpr unsigned nbchars = nbdigits_hi + nbdigits_lo + 1;
  static constexpr unsigned maxbuckets = 10*62;
  static Rps_HashInt hash(uint64_t hi, uint64_t lo)
  {
    return (hi % 2147473837) + ((hi >> 32) ^ (lo * 17 + 201151));
  }
  Rps_HashInt hash() const
  {
    return hash(_id_hi, _id_lo);
  };
  static unsigned bucket_num(uint64_t hi)
  {
    unsigned b = hi / (max_hi / maxbuckets);
    RPS_ASSERTPRINTF(b<=maxbuckets, "b=%u", b);
    return b;
  };
  unsigned bucket_num(void) const
  {
    return bucket_num(_id_hi);
  };
  bool empty() const
  {
    return _id_hi == 0 && _id_lo == 0;
  };
  bool valid() const
  {
    return _id_hi >= min_hi && _id_hi < max_hi
           && _id_lo >= min_lo && _id_lo < max_lo
           && hash() != 0;
  }
  operator bool () const
  {
    return !empty();
  };
  bool operator ! () const
  {
    return empty();
  };
  bool operator == (const Rps_Id&oth) const
  {
    return _id_hi == oth._id_hi && _id_lo == oth._id_lo;
  };
  bool operator < (const Rps_Id&oth) const
  {
    if (_id_hi > oth._id_hi) return false;
    if (_id_hi < oth._id_hi) return true;
    return _id_lo < oth._id_lo;
  }
  bool operator <= (const Rps_Id&oth) const
  {
    if (_id_hi > oth._id_hi) return false;
    if (_id_hi < oth._id_hi) return true;
    return _id_lo <= oth._id_lo;
  }
  bool operator != (const Rps_Id&oth) const
  {
    return !(oth == *this);
  };
  bool operator > (const Rps_Id&oth) const
  {
    return oth < *this;
  };
  bool operator >= (const Rps_Id&oth) const
  {
    return oth <= *this;
  };
  uint64_t hi() const
  {
    return _id_hi;
  };
  uint64_t lo() const
  {
    return _id_lo;
  };
  Rps_Id(uint64_t h, uint32_t l=0) : _id_hi(h), _id_lo(l)
  {
    RPS_ASSERT((h==0 && l==0) || hash() != 0);
  };
#ifndef RPS_ONLY_ID_CODE
  static Rps_Id random()
  {
    for(;;)
      {
        auto hi = Rps_Random::random_64u() % max_hi;
        if (RPS_UNLIKELY(hi < min_hi)) continue;
        auto lo = Rps_Random::random_64u() % max_lo;
        if (RPS_UNLIKELY(lo < min_lo)) continue;
        if (RPS_UNLIKELY(hash(hi,lo) == 0)) continue;
        return Rps_Id(hi,lo);
      };
  };
  Rps_Id (std::nullptr_t) : Rps_Id(random()) {};
#endif /*RPS_ONLY_ID_CODE*/
  // rule of five
  Rps_Id (Rps_Id&& oth) :
    _id_hi(oth._id_hi), _id_lo(oth._id_lo) {};
  Rps_Id(const Rps_Id&oth)  :
    _id_hi(oth._id_hi), _id_lo(oth._id_lo) {};
  Rps_Id& operator = (const Rps_Id &oth)
  {
    _id_hi = oth._id_hi;
    _id_lo = oth._id_lo;
    return *this;
  };
  Rps_Id& operator = (Rps_Id&&oth)
  {
    std::swap(_id_hi, oth._id_hi);
    std::swap(_id_lo, oth._id_lo);
    return *this;
  }
  Rps_Id () : Rps_Id((uint64_t)0, (uint32_t)0) {};
  Rps_Id (const char*buf, const char**pend=nullptr, bool *pok=nullptr);
  Rps_Id (const std::string&str) : Rps_Id(str.c_str()) {};
  void to_cbuf24(char cbuf[/*24*/]) const;
  inline std::string to_string() const;
/// hashing, comparing, and equality testing operations on Rps_Id-s
  struct Hasher
  {
    std::size_t operator() (const Rps_Id& id) const
    {
      return (std::size_t)(id.hash());
    };
  };        // end Rps_Id::Hasher
  struct LessComparer
  {
    bool operator()  (const Rps_Id&id1, const Rps_Id&id2) const
    {
      return id1 < id2;
    }
  };        // end Rps_Id::LessComparer
  struct EqualTester
  {
    bool operator()  (const Rps_Id&id1, const Rps_Id&id2) const
    {
      return id1 == id2;
    }
  };        // end Rps_Id::EqualTester
};        // end class Rps_Id

#endif /*OID_RPS_INCLUDED*/

