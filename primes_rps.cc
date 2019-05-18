/****************************************************************
 * file primes_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has some array of primes and functions to get a prime
 *      above, or less, than some given number.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
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

#include <cstdint>

extern "C" int64_t rps_prime_above (int64_t n);
extern "C" int64_t rps_prime_below (int64_t n);

// an array of primes, gotten with something similar to
//   /usr/games/primes 3  | awk '($1>p+p/9){print $1, ","; p=$1}'
static const int64_t rps_primes_tab[] =
{
  3, 5, 7, 11, 13, 17, 19, 23, 29, 37, 43, 53, 59, 67, 79, 89, 101, 113,
  127, 149, 167, 191, 223, 251, 281, 313, 349, 389, 433, 487, 547, 613,
  683, 761, 853, 953, 1061, 1181, 1319, 1471, 1637, 1823, 2027, 2267,
  2521, 2803, 3119, 3467, 3853, 4283, 4759, 5297, 5897, 6553, 7283, 8093,
  8999, 10007, 11119, 12373, 13751, 15287, 16987, 18899, 21001, 23339,
  25933, 28817, 32027, 35591, 39551, 43951, 48847, 54277, 60317, 67021,
  74471, 82757, 91957, 102181, 113537, 126173, 140197, 155777, 173087,
  192319, 213713, 237467, 263863, 293201, 325781, 361979, 402221, 446921,
  496579, 551767, 613097, 681221, 756919, 841063, 934517, 1038383, 1153759,
  1281961, 1424407, 1582697, 1758553, 1953949, 2171077, 2412323, 2680367,
  2978189, 3309107, 3676789, 4085339, 4539277, 5043653, 5604073, 6226757,
  6918619, 7687387, 8541551, 9490631, 10545167, 11716879, 13018757,
  14465291, 16072547, 17858389, 19842659, 22047401, 24497113, 27219019,
  30243359, 33603743, 37337497, 41486111, 46095719, 51217477, 56908337,
  63231499, 70257241, 78063641, 86737379, 96374881, 107083213, 118981367,
  132201521, 146890631, 163211821, 181346479, 201496157, 223884629,
  248760703, 276400823, 307112027, 341235667, 379150777, 421278643,
  468087391, 520097111, 577885681, 642095213, 713439127, 792710159,
  880789067, 978654533, 1087393949, 1208215531, 1342461719, 1491624137,
  1657360153, 1841511311, 2046123679, 2273470799, 2526078691, 2806754123,
  3118615693, 3465128567, 3850142869, 4277936557, 4753262879, 5281403287,
  5868225889, 6520251019, 7244723357, 8049692639, 8944102939, 9937892189,
  11042102507, 12269002787, 13632225331, 15146917109, 16829907907,
  18699897683, 20777664097, 23086293457, 25651437191, 28501596883,
  31668440987, 35187156661, 39096840751, 43440934181, 48267704657,
  53630782993, 59589758903, 66210843271, 73567603643, 81741781829,
  90824202037, 100915780061, 112128644521, 124587382807, 138430425373,
  153811583771, 170901759761, 189890844179, 210989826869, 234433140979,
  260481267763, 289423630891, 321581812103, 357313124579, 397014582917,
  441127314403, 490141460453, 544601622727, 605112914149, 672347682449,
  747052980503, 830058867229, 922287630259, 1024764033637, 1138626704071,
};


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
}

// eof primes_rps.cc
