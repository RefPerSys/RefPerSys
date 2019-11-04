/****************************************************************
 * file rps_main.c
 *
 * Description:
 *      The main function of RefPerSys.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
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

#include "refpersys.h"


/* open /dev/urandom file descriptor */
static int
randfd_open(void)
{
	static int randfd = 0;

	if (RPS_UNLIKELY (randfd < 2)) {
		/* see https://unix.stackexchange.com/questions/324209/ */
		randfd = open ("/dev/urandom", O_RDONLY);
		if (RPS_UNLIKELY (!randfd)) {
#warning FIXME: use RPS_FATAL
			printf ("failed to open /dev/urandom\n");
			abort ();
		}
	}

	return randfd;
}


/* close /dev/urandom file descriptor */
static void
randfd_close(void)
{
	if (RPS_LIKELY (randfd > 2)) {
		close (randfd);
		randfd = 0;
	}
}


int main(int argc, char **argv)
{
	/* TODO: parse argc and argv for command line options */
	(void) argc;
	(void) argv;

	randfd_close ();
} /* end of main */

///// end of file main_rps.c
