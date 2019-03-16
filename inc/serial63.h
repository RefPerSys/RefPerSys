/*
 * File: refpersys/inc/serial63.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      object ID serial type and its interface.
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
#if (!defined __REFPERSYS_SERIAL63_DEFINED)
#       define __REFPERSYS_SERIAL63_DEFINED




        /* include required header files */
#include <stdbool.h>
#include <stdint.h>
#include "./ctcheck.h"




        /*  open support for C++ */
#if (defined __cplusplus)
        extern "C" {
#endif




        /* declare the rps_serial63 type; this is analogous to the Bismon
         * serial64_tyBM type in the header file id_BM.h */
typedef uint64_t rps_serial63;




/*
 *      rps_serial63_new() - generates a new random object ID serial
 *
 *      Return:
 *        - new random object ID serial
 *
 *      See:
 *        - randomserial63_BM() in Bismon's id_BM.h
 */
extern rps_serial63 rps_serial63_new(void);




        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

