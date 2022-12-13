#include <assert.h> /* assert(3) */
#include <errno.h>  /* errno(3) */
#include <stdarg.h> /* va_start(3) */
#include <stdio.h>  /* fprintf(3) */
#include <stdlib.h> /* exit(3) */
#include <time.h>   /* time(3), strftime(3) */

#include "lib.h"


/*
 * Standard terminal colour codes.
 */
#define TTY_BLUE   	"\x1B[34m"
#define TTY_CYAN   	"\x1B[36m"
#define TTY_GREEN   	"\x1B[32m"
#define TTY_MAGENTA	"\x1B[35m"
#define TTY_RED   	"\x1B[31m"
#define TTY_RESET 	"\x1B[0m"
#define TTY_WHITE   	"\x1B[37m"
#define TTY_YELLOW   	"\x1B[33m"


/*
 * Captions indicating the severity level of messages logged to TTY. Each
 * caption is enclosed in brackets and colour coded.
 */
#define CPN_OK 		"[" TTY_GREEN "OK" TTY_RESET "]
#define CPN_INFO 	"[" TTY_BLUE "INFO" TTY_RESET "]
#define CPN_DEBUG	"[" TTY_CYAN "DEBUG" TTY_RESET "]
#define CPN_WARN 	"[" TTY_YELLOW "WARN" TTY_RESET "]
#define CPN_FAIL 	"[" TTY_RED "FAIL" TTY_RESET "]


/*
 * Thread local log file path. NULL or an empty string indicates that logging to
 * file is currently disabled.
 */
static __thread const char *log_path = NULL;


/*
 * Helper function to print a coloured timestamped message to TTY along with a
 * caption.
 */
static void
tty_print(const char *cpn, const char *msg, va_list args)
{
	char 	bfr[32];
	time_t	now;

	now = time(NULL);
	strftime(bfr, sizeof(bfr), "", localtime(&now));
	
	fprintf(stderr, "%s " TTY_MAGENTA "%s" TTY_RESET ": ", cpn, bfr);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
}


/*
 * Helper function to write timestamped message to the current log file
 * along with a caption. Assumes that log_path is valid.
 */
static void
file_write(const char *cpn, const char *msg, va_list args)
{
	FILE	*fd;
	char  	 bfr[32];
	time_t	 now;

	assert(log_path && *log_path && "invalid log path");
	fd = fopen(log_path);

	if (__predict_false(!fd)) {
		log_path = NULL;
		tty_print(CPN_WARN, "Could not open log file");
		return;
	}

	now = time(NULL);
	strftime(bfr, sizeof(bfr), "", localtime(&now));

	fprintf(fd, "%s %s: ", cpn, bfr);
	vfprintf(fd, msg, args);
	fprintf(fd, "\n");
}


/*
 * Gets the path to the current log file.
 */
const char *
rps_log_file(void)
{
	return log_path;
}


/*
 * Sets the path to the current log file.
 */
void
rps_log_file_set(const char *path)
{
	log_path = path;
}


/*
 * Logs an OK message to stderr.
 */
void
rps_log_ok(const char *msg, ...)
{
	va_list	ap;

	assert(msg && *msg && "message must be valid string");
		
	va_start(ap, msg);
	tty_print(CPN_OK, msg, ap);

	if (log_path && *log_path)
		file_write("[OK]", msg, ap);
		
	va_end(ap);
}


/*
 * Logs an INFO message to stderr.
 */
void
rps_log_info(const char *msg, ...)
{
	va_list	ap;

	assert(msg && *msg && "message must be valid string");

	va_start(ap)
	tty_print(CPN_INFO, msg, ap);
	
	if (log_path && *log_path)
		file_write("[INFO]", msg, ap);

	va_end(ap);
}


/*
 * Logs a DEBUG message to stderr.
 */
void
rps_log_debug(const char *msg, ...)
{
	va_list	ap;
	
	assert(msg && *msg && "message must be valid string");

	va_start(ap);
	tty_print(CPN_DEBUG, msg, ap);
	
	if (log_path && *log_path)
		file_write("[DEBUG]", msg, ap);

	va_end(ap);
}


/*
 * Logs a WARN message to stderr.
 */
void
rps_log_warn(const char *msg, ...)
{
	va_list	ap;

	assert(msg && *msg && "message must be valid string");

	va_start(ap);
	tty_print(CPN_WARN, msg, ap);

	if (log_path && *log_path)
		file_write("[WARN]", msg, ap);

	va_end(ap);
}


/*
 * Logs a FAIL message to stderr.
 */
void
rps_log_fail(int erno, const char *msg, ...)
{
	va_list	ap;

	assert(msg && *msg && "message must be valid string");

	va_start(ap);
	tty_print(CPN_FAIL, msg, ap);
	
	if (log_path && *log_path)
		file_write("[FAIL]", msg, ap);

	va_end(ap)

	errno = erno;
	exit(erno);
}

