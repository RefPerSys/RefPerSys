/*
 * File: refpersys/src/cmdline.c
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It implements the
 *      commandline parsing interface.
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


        /* include required header files */
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "../inc/.version.gen.h"
//#include "../inc/mt64.hpp"


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


        /* parse version metadata from the .version.gen.h header file generated
         * at compile time */
static void version_parse(void)
{
        const char *format = "refpersys: version information\n"
                             "\t last git commit: %s\n";
        /*const char *lastcommit = "git log --format=oneline --abbrev=12"
                                 " --abbrev-commit -q  | head -1";*/

        snprintf(version_bfr, VERSION_BFRLEN, format, RPS_VERSION_LASTCOMMIT);
}


        /* define helper function to execute the NAME_PRAND argument option */
static void exec_prand(void)
{
        //printf("rps_random_uint64(): %" PRIu64 "\n", rps_random_uint64());
        printf("rps_random_uint64(): %" PRIu64 "\n", 5UL);
}


        /* define the argument option parsing callback */
static error_t argopt_parse(int key, char *arg, struct argp_state *state)
{
                /* switch through argument option keys, handling each case in
                 * turn as required */
        switch (key) {
        case KEY_PRAND:
                exec_prand();
                break;

        default:
                return ARGP_ERR_UNKNOWN;
        }

                /* signal that no error has occurred */
        return 0;
}


        /* define the rps_cmdline_parse() interface function; this function was
         * declared in refpersys/inc/cmdline.h */
int rps_cmdline_parse(int argc, char **argv)
{
        auto struct argp argp = {
                .options = argopt_vec,
                .parser = argopt_parse,
                .args_doc = NULL,
                .doc = NULL
        };

                /* parse refpersys version metadata generated at compile time */
        version_parse();

                /* ensure we have at least one command line argument in addition
                 * to the refpersys command */
        if (argc < 2) {
                printf(MSG_NOARGS);
                exit(EXIT_FAILURE);
        }

                /* parse argument options */
        return argp_parse(&argp, argc, argv, 0, NULL, NULL);
}

