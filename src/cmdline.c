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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "../inc/.version.gen.h"


static char version_bfr[1024];
const char *argp_program_version = version_bfr;


static void version_set(void)
{
        const char *format = "refpersys: version information\n"
                             "\t last git commit: %s\n";
        /*const char *lastcommit = "git log --format=oneline --abbrev=12"
                                 " --abbrev-commit -q  | head -1";*/

        snprintf(version_bfr, 1024, format, RPS_VERSION_LASTCOMMIT);
}


int rps_cmdline_parse(int argc, char **argv)
{
        version_set();
        return 0;
}

