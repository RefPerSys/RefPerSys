/****************************************************************
 * File: rps_hints.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Internal file for GNU C compiler hint wrappers.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.org>
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


/* IMPORTANT TODO: merge this file later into refpersys.h */


#if !defined RPS_HINTS_INCLUDED

/* hints that a predicate is likely to be true */
#if (defined __GNUC__)
#	define RPS_LIKELY(p) (__builtin_expect(!!(p), 1))
#else
#	define RPS_LIKELY(p) (p)
#	warning RPS_LIKELY() macro has no effect
#endif

/* hints that a predicate is likely to be false */
#if (defined __GNUC__)
#	define RPS_UNLIKELY(p) (__builtin_expect(!!(p), 0))
#else
#	define RPS_UNLIKELY(p) (p)
#	warning RPS_UNLIKELY() macro has no effect
#endif

/* hints that a variable or function is unused */
#if (defined __GNUC__)
#	define RPS_UNUSED __attribute__((unused))
#else
#	define RPS_UNUSED
#	warning RPS_UNUSED attribute has no effect
#endif

/* hints that a function is pure */
#if (defined __GNUC__)
#	define RPS_PURE __attribute__((pure))
#else
#	define RPS_PURE
#	warning RPS_PURE attribute has no effect
#endif

/* hints that a function is hot */
#if (defined __GNUC__)
#	define RPS_HOT __attribute__((hot))
#else
#	define RPS_HOT
#	warning RPS_HOT attribute has no effect
#endif

#endif /* !defined RPS_HINTS_INCLUDED */

