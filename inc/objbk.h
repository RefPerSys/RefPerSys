/*
 * File: refpersys/inc/objbk.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      object bucket type and its interface.
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
#if (!defined __REFPERSYS_OBJBK_DEFINED)
#       define __REFPERSYS_OBJBK_DEFINED




        /* include required header files */
#include <stddef.h>
#include "./ctcheck.h"




        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
#endif




/*
 *      RPS_OBJBK_MAX - maximum object buckets
 *
 *      The RPS_OBJBK_MAX symbolic constant defines the maximum object buckets.
 *      TODO: explain why it is 10 * 62, and its significance
 */
#define RPS_OBJBK_MAX ((size_t) 620)




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_OBJBK_DEFINED) */

