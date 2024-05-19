#!/bin/sh

#
# Ensure that we're running a supported Linux distribution.
# As of now, we support only Debian/Ubuntu, but other Linux
# distros are in the pipeline.
#
check_os()
{
	echo 'TODO'
}

#
# Parse the command line options passed to the configure script.
#
parse_flags()
{
	echo 'TODO'
}

#
# Parse the command line arguments passed to the configure script.
#
parse_args()
{
	echo 'TODO'
}

#
# Main entry point for the configuration script.  We parse the command
# line options and arguments passed to this script after checking that
# a supported host OS environment is present.
#
main()
{
	check_os
	parse_flags "$@"
	parse_args "$@"
}

main "$@"

