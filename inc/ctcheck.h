/*
 * File: refpersys/inc/ctcheck.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It defines the
 *      compile-time check macros.
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
 *      Released under the GNU General Public License version 3 (GPLv3)
 *      <http://opensource.org/licenses/GPL-3.0>. See the accompanying LICENSE
 *      file for complete licensing details.
 *
 *      BY CONTINUING TO USE AND/OR DISTRIBUTE THIS FILE, YOU ACKNOWLEDGE THAT
 *      YOU HAVE UNDERSTOOD THESE LICENSE TERMS AND ACCEPT THEM.
 */




        /* create header guard */
#if (!defined __REFPERSYS_ASSERTMACROS_DEFINED)
#       define __REFPERSYS_ASSERTMACROS_DEFINED




        /* ensure C99 dialect is being used for compilation */
#if (!defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#       error "building refpersys requires a C99 compiler"
#endif




        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
#endif




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_ASSERTMACROS_DEFINED) */

