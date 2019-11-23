/****************************************************************
 * file inline_rps.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its internal C++ header file for inlined functions.
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



#ifndef INLINE_RPS_INCLUDED
#define INLINE_RPS_INCLUDED


//// this file should only be included at end of refpersys.hh
#ifndef REFPERSYS_INCLUDED
#error wrong direct inclusion of "inline_rps.hh"
#endif /*REFPERSYS_INCLUDED*/

// see http://man7.org/linux/man-pages/man2/clock_gettime.2.html
static inline double
rps_monotonic_real_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_monotonic_real_time


static inline double
rps_process_cpu_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_process_cpu_time


static inline double
rps_thread_cpu_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_thread_cpu_time


static inline
std::ostream& operator << (std::ostream& out, const Rps_BackTrace_Helper& rph)
{
  auto o = rph.swap_output(&out);
  out << std::endl;
  rph.do_out();
  rph.swap_output(o);
  out << std::endl;
  return out;
} // end of << for Rps_Backtrace_Helper

#endif /*INLINE_RPS_INCLUDED*/
// end of internal header file inline_rps.hh
