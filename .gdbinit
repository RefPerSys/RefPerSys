# file refpersys/.gdbinit
# SPDX-License-Identifier: GPL-3.0-or-later
# GPLv3+ licensed - see https://www.gnu.org/licenses/gpl-3.0.en.html
### see https://stackoverflow.com/a/56407988/841108
### see http://refpersys.org/ and https://github.com/RefPerSys/RefPerSys
### © Copyright 2019 - 2023 The Reflective Persistent System Team
add-auto-load-safe-path ./.gdbinit
break abort
break rps_fatal_stop_at
break std::terminate
#break rps_small_quick_tests_after_load
set max-value-size 67108864
