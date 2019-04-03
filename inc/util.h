/*
 * File: refpersys/inc/util.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      helper utilities interface.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Niklas Rosencrantz <niklasro@gmail.com>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 * Copyright:
 *      (c) 2019 The Reflective Persistent System Team
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
 */

#ifndef REFPERSYS_UTIL_INCLUDED
#define REFPERSYS_UTIL_INCLUDED


/* include required header files */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <random>


/* in generated _timestamp.c */
extern "C" const char rps_git_commit[];
extern "C" const char rps_build_timestamp[];





// branch prediction macro to indicate unlikely condition
// adapted from MOM_UNLIKELY() macro in meltmoni.hh
#define rps_unlikely(p) __builtin_expect(!!(p),0)


// branch prediction macro to indicate likely condition
// adapted from MOM_LIKELY() macro in meltmoni.hh
#define rps_likely(p) !__builtin_expect(!(p),0)


// prints an error message indicating MPS reservation failure
static inline void rps_perror_mps_reserve(size_t size)
{
  char errmsg[100];

  snprintf(errmsg, sizeof(errmsg),
           "MPS error: unable to reserve %lu bytes",
           size);
  perror(errmsg);
}





// Singleton to generate random numbers through the Mersenne Twister algorithm
class RpsRandom
{
public:

  static uint32_t generate32(void)
  {
    return _instance.generate32_here();
  };

  static uint32_t generate32_nonzero(void)
  {
    uint32_t rand;

    do
      {
        rand = generate32();
      }
    while (rand == 0);

    return rand;
  }

  static uint64_t generate64(void)
  {
    return _instance.generate64_here();
  };

  static uint64_t generate64_nonzero(void)
  {
    uint64_t rand;

    do
      {
        rand = generate64();
      }
    while (rand == 0);

    return rand;
  }

private:
  // thread-safe global singleton instance
  static thread_local RpsRandom _instance;

  // Mersenne Twister
  std::mt19937 _twister;

  // random number counter
  uint_fast64_t _counter;

  // private default constructor
  // TODO: check that random_device is used once
  RpsRandom()
    : _twister(std::mt19937(std::random_device{}()))
    , _counter(0)
  { }

  // generates 32-bit random number by seeding a Mersenne Twister with a random
  // entropy pool
  uint32_t generate32_here(void)
  {
    // we post-increment, since we need to seed when used first
    if (rps_unlikely (_counter++ % 65536 == 0))
      {
        std::random_device rd;
        std::seed_seq seeds { rd(), rd(), rd(), rd(), rd(), rd(), rd() };
        _twister.seed(seeds);
      }

    return _twister();
  }

  // generates 64-bit random number by combining two 32-bit random
  // numbers through bit manipulation
  uint64_t generate64_here(void)
  {
    return (static_cast<uint64_t>(generate32()) << 32)
           | static_cast<uint64_t>(generate32());
  }
}; // end class RpsRandom


// 96-bit object ID represented in base 62
class RpsObjectId
{
private:
  uint64_t _hi; // most significant 64-bits
  uint32_t _lo; // least significant 32-bits
}; // end class RpsObjectId


#endif /* REFPERSYS_UTIL_INCLUDED */

