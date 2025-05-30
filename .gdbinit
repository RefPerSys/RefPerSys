# file refpersys/.gdbinit
# SPDX-License-Identifier: GPL-3.0-or-later
# GPLv3+ licensed - see https://www.gnu.org/licenses/gpl-3.0.en.html
### see https://stackoverflow.com/a/56407988/841108
### see http://refpersys.org/ and https://github.com/RefPerSys/RefPerSys
### © Copyright 2019 - 2025 The Reflective Persistent System Team
add-auto-load-safe-path ./.gdbinit
set debuginfod enabled on
break abort
break rps_fatal_stop_at
break std::terminate
break exit
## carbrepl_rps_issue is a global label defined by asm code
## in file carbrepl_rps.cbrt function rps_carbrepl_next_token
## commit b12db4cdd434227 in mid-may 2025
break carbrepl_rps_issue
#break rps_small_quick_tests_after_load
set max-value-size 67108864
