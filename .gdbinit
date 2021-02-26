# file refpersys/.gdbinit
# GPLv3+ licensed
### see https://stackoverflow.com/a/56407988/841108
add-auto-load-safe-path ./.gdbinit
break abort
break rps_fatal_stop_at
break std::terminate
#break rps_small_quick_tests_after_load
set max-value-size 67108864
