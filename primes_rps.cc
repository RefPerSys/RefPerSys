/****************************************************************
 * file primes_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has some array of primes and functions to get a prime
 *      above, or less, than some given number.  Some parts have been
 *      computer generated.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 *      © Copyright (C) 2019 - 2025 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io> and refpersys.org
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

#include <cstdint>

extern "C" const char rps_primes_gitid[];
const char rps_primes_gitid[]= RPS_GITID;

extern "C" const char rps_primes_date[];
const char rps_primes_date[]= __DATE__;

extern "C" const char rps_primes_shortgitid[];
const char rps_primes_shortgitid[]= RPS_SHORTGITID;

extern "C" int64_t rps_prime_above (int64_t n);
extern "C" int64_t rps_prime_below (int64_t n);

// an array of primes, gotten with something similar to
//   /usr/games/primes 3  | awk '($1>p+p/10){print $1, ","; p=$1}'

// we actually use:
// https://github.com/bstarynk/misc-basile/blob/master/makeprimes.c
// (basically, something replacing awk and running an equivalent pipe)
// and most importantly:
// https://github.com/kimwalisch/primesieve which is a very clever program.


//// the actual numbers have been gotten with
//// makeprimes  2333444555666 10 'primesieve -t18 -p'
//// which took 13439 seconds to run on a AMD Ryzen Threadripper 2970WX
static const int64_t rps_primes_tab[] =
{
//// piping primesieve -t18 -p 2 2333444555666
  2, 3, 5, 7,
  11, 13, 17, 19,
  23, 29, 37, 41,
  47, 53, 59, 67,
  79, 89, 101, 113,
  127, 149, 167, 191,
  211, 233, 257, 283,
  313, 347, 383, 431,
  479, 541, 599, 659,
  727, 809, 907, 1009,
  1117, 1229, 1361, 1499,
  1657, 1823, 2011, 2213,
  2437, 2683, 2953, 3251,
  3581, 3943, 4339, 4783,
  5273, 5801, 6389, 7039,
  7753, 8537, 9391, 10331,
  11369, 12511, 13763, 15149,
  16673, 18341, 20177, 22229,
  24469, 26921, 29629, 32603,
  35869, 39461, 43411, 47777,
  52561, 57829, 63617, 69991,
  76991, 84691, 93169, 102497,
  112757, 124067, 136481, 150131,
  165161, 181693, 199873, 219871,
  241861, 266051, 292661, 321947,
//#100 of 27763
  354143, 389561, 428531, 471389,
  518533, 570389, 627433, 690187,
  759223, 835207, 918733, 1010617,
  1111687, 1222889, 1345207, 1479733,
  1627723, 1790501, 1969567, 2166529,
  2383219, 2621551, 2883733, 3172123,
  3489347, 3838283, 4222117, 4644329,
  5108767, 5619667, 6181639, 6799811,
  7479803, 8227787, 9050599, 9955697,
  10951273, 12046403, 13251047, 14576161,
  16033799, 17637203, 19400929, 21341053,
  23475161, 25822679, 28404989, 31245491,
  34370053, 37807061, 41587807, 45746593,
  50321261, 55353391, 60888739, 66977621,
  73675391, 81042947, 89147249, 98061979,
  107868203, 118655027, 130520531, 143572609,
  157929907, 173722907, 191095213, 210204763,
  231225257, 254347801, 279782593, 307760897,
  338536987, 372390691, 409629809, 450592801,
  495652109, 545217341, 599739083, 659713007,
  725684317, 798252779, 878078057, 965885863,
  1062474559, 1168722059, 1285594279, 1414153729,
  1555569107, 1711126033, 1882238639, 2070462533,
  2277508787, 2505259681, 2755785653, 3031364227,
  3334500667, 3667950739, 4034745863, 4438220467,
//#200 of 209734681
  4882042547, 5370246803, 5907271567, 6497998733,
  7147798607, 7862578483, 8648836363, 9513720011,
  10465092017, 11511601237, 12662761381, 13929037523,
  15321941293, 16854135499, 18539549051, 20393503969,
  22432854391, 24676139909, 27143753929, 29858129341,
  32843942389, 36128336639, 39741170353, 43715287409,
  48086816161, 52895497877, 58185047677, 64003552493,
  70403907883, 77444298689, 85188728633, 93707601497,
  103078361647, 113386197853, 124724817647, 137197299431,
  150917029411, 166008732391, 182609605691, 200870566261,
  220957622911, 243053385209, 267358723741, 294094596143,
  323504055803, 355854461419, 391439907569, 430583898359,
  473642288209, 521006517137, 573107168903, 630417885871,
  693459674461, 762805641919, 839086206131, 922994826779,
  1015294309507, 1116823740479, 1228506114527, 1351356725987,
  1486492398631, 1635141638587, 1798655802451, 1978521382723,
  2176373521033,
/// end, read 85041558143 primes, printed 265 primes, so 0.00000% cpu 13439.95 s
};

int64_t
rps_prime_ranked (int rk)
{
  constexpr unsigned numprimes = sizeof (rps_primes_tab) / sizeof (rps_primes_tab[0]);
  if (rk < 0)
    return 0;
  if (rk < (int)numprimes)
    return rps_primes_tab[rk];
  return 0;
} // end of rps_prime_ranked

int64_t
rps_prime_above (int64_t n)
{
  constexpr unsigned numprimes = sizeof (rps_primes_tab) / sizeof (rps_primes_tab[0]);
  int lo = 0, hi = numprimes;
  if (n >= rps_primes_tab[numprimes - 1])
    return 0;
  if (n < 2)
    return 2;
  while (lo + 8 < hi)
    {
      int md = (lo + hi) / 2;
      if (rps_primes_tab[md] > n)
        hi = md;
      else
        lo = md;
    };
  if (hi < (int) numprimes - 1)
    hi++;
  if (hi < (int) numprimes - 1)
    hi++;
  for (int ix = lo; ix < hi; ix++)
    if (rps_primes_tab[ix] > n)
      return rps_primes_tab[ix];
  return 0;
}

int64_t
rps_prime_greaterequal_ranked (int64_t n, int*prank)
{
  constexpr unsigned numprimes = sizeof (rps_primes_tab) / sizeof (rps_primes_tab[0]);
  if (prank) *prank = -1;
  int lo = 0, hi = numprimes;
  if (n >= rps_primes_tab[numprimes - 1])
    return 0;
  if (n < 2)
    return 2;
  while (lo + 8 < hi)
    {
      int md = (lo + hi) / 2;
      if (rps_primes_tab[md] > n)
        hi = md;
      else
        lo = md;
    };
  if (hi < (int) numprimes - 1)
    hi++;
  if (hi < (int) numprimes - 1)
    hi++;
  for (int ix = lo; ix < hi; ix++)
    if (rps_primes_tab[ix] >= n)
      {
        if (prank)
          *prank =  ix;
        return rps_primes_tab[ix];
      }
  return 0;
} // end of rps_prime_greaterequal_ranked



int64_t
rps_prime_below (int64_t n)
{
  constexpr unsigned numprimes = sizeof (rps_primes_tab) / sizeof (rps_primes_tab[0]);
  int lo = 0, hi = numprimes;
  if (n >= rps_primes_tab[numprimes - 1])
    return 0;
  if (n < 2)
    return 2;
  while (lo + 8 < hi)
    {
      int md = (lo + hi) / 2;
      if (rps_primes_tab[md] > n)
        hi = md;
      else
        lo = md;
    };
  if (hi < (int) numprimes - 1)
    hi++;
  if (hi < (int) numprimes - 1)
    hi++;
  for (int ix = hi; ix >= 0; ix--)
    if (rps_primes_tab[ix] < n)
      return rps_primes_tab[ix];
  return 0;
} // end rps_prime_below


int64_t
rps_prime_lessequal_ranked (int64_t n, int*prank)
{
  constexpr unsigned numprimes = sizeof (rps_primes_tab) / sizeof (rps_primes_tab[0]);
  if (prank) *prank = -1;
  int lo = 0, hi = numprimes;
  if (n >= rps_primes_tab[numprimes - 1])
    return 0;
  if (n < 2)
    return 2;
  while (lo + 8 < hi)
    {
      int md = (lo + hi) / 2;
      if (rps_primes_tab[md] > n)
        hi = md;
      else
        lo = md;
    };
  if (hi < (int) numprimes - 1)
    hi++;
  if (hi < (int) numprimes - 1)
    hi++;
  for (int ix = hi; ix >= 0; ix--)
    if (rps_primes_tab[ix] <= n)
      {
        if (prank)
          *prank = ix;
        return rps_primes_tab[ix];
      }
  return 0;
} // end rps_prime_lessequal_ranked

// eof primes_rps.cc
