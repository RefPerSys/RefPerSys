/*
 * File: refpersys/src/refpersys.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It implements the
 *      refpersys executable
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


        /* include required header files */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <argp.h>
#include "../inc/.version.gen.h"


/** COMMAND LINE ARGUMENT PARSING *********************************************/


        /* define long names for argument options */
#define NAME_PRAND "print-random-uint64"


        /* define keys for argument options */
#define KEY_PRAND 'r'


        /* define documentation strings for arguments options */
#define DOC_PRAND "Print random 64-bit unsigned integer"


        /* define length of buffer to hold version metadata */
#define VERSION_BFRLEN 1024


        /* define error message to display if no argument is provided */
#define MSG_NOARGS "refpersys: no options specified\n" \
                   "\trun refpersys --help to display options\n"


        /* define buffer to hold version metadata generated at compile time;
         * TODO: confirm from Dr. Basile if this needs to be thread_local */
static char version_bfr[VERSION_BFRLEN];


        /* set the address to e-mail bug reports; this global is provided by the
         * ARGP library */
const char *argp_program_bug_address = "basile@starynkevitch.net";


        /* set the version information; this global is provided by the ARGP
         * library */
const char *argp_program_version = version_bfr;


        /* define ARGP-specific vector to hold argument options;
         * TODO: confirm from Dr. Basile if this needs to be thread_local */
static struct argp_option argopt_vec[] = {
        {
                .name = NAME_PRAND,
                .key = KEY_PRAND,
                .arg = NULL,
                .flags = 0,
                .doc = DOC_PRAND
        },
        { NULL }
};


        /* undefine the symbolic constants defined in this section so that they
         * may safely be reused in the following sections if required */
#undef NAME_PRAND
#undef KEY_PRAND
#undef DOC_PRAND
#undef VERSION_BFRLEN
#undef MSG_NOARGS


/** MAIN ENTRY POINT **********************************************************/


int main(int argc, char **argv)
{
        //return rps_cmdline_parse(argc, argv);
        return EXIT_SUCCESS;
}

