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
 *      RPS_SERIAL63_MIN - minimum object ID serial value
 *
 *      The RPS_SERIAL63_MIN symbolic constant defines the minimum value of an
 *      object ID serial.
 *
 *      TODO: explain why it is 62 * 62
 */
#define RPS_SERIAL63_MIN ((uint64_t) 3884)




/*
 *      RPS_SERIAL63_MAX - maximum object ID serial value
 *
 *      The RPS_SERIAL63_MAX symbolic constant defines the maximum value of an
 *      object ID serial.
 *
 *      TODO: explain why it is 10 * 62 * (62* 62*62) * (62*62*62) * (62*62*62)
 */
#define RPS_SERIAL63_MAX ((uint64_t) 8392993658683402240)




/*
 *      rps_serial63_new() - generates a new random object ID serial
 *
 *      TODO: elaborate detailed description
 *
 *      Return:
 *        - new random object ID serial
 *
 *      See:
 *        - randomserial63_BM() in Bismon's id_BM.h
 */
extern rps_serial63 rps_serial63_new(void);




/*
 *      rps_serial63_valid() - checks validity of object ID serial
 *        - s63: contextual object ID serial
 *
 *      TODO: elaborate detailed description
 *
 *      Return:
 *        - true if @s63 is valid
 *        - false if @s63 is invalid
 *
 *      See:
 *        - validserial63_BM() in Bismon's id_BM.h
 */
static inline bool rps_serial63_valid(rps_serial63 s63)
{
                /* @s63 must fall within the permissible maximum and minimum
                 * values for object ID serials */
        return (s63 > RPS_SERIAL63_MIN && s63 < RPS_SERIAL63_MAX);
}




/*
 *      rps_serial63_str() - stringifies an object ID serial
 *        - s63: contextual object ID serial
 *        - str: 16-byte string representation
 *
 *      TODO: elaborate detailed description
 *
 *      Return:
 *        - TODO
 *
 *      See:
 *        - serial63tocbuf16_BM() in Bismon's id_BM.h
 */
extern int rps_serial63_str(rps_serial63 s63, char str[static 16]);



        /* close support for C++ */
#if (defined __cplusplus)
        }
#endif




#endif /* (!defined __REFPERSYS_OBJECT_DEFINED) */

