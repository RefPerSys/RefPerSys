# This script defines the SU helper variable. Requires tools/msg.sh to be 
# sourced.

# su command for client cod
export SU

# set SU to either sudo or doas, depending on which is available. sudo takes
# preference.
if sudo -V >/dev/null 2>&1; then
	SU=sudo
elif doas -L >/dev/null 2>&1; then
	SU=doas
else
	msg_failt 'neither sudo nor doas found; install one of the two'
fi
