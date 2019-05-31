# file refpersys/.gdbinit
# GPLv3+ licensed
add-auto-load-safe-path ./.gdbinit
break abort
break rps_fatal_stop_at
set max-value-size 67108864
