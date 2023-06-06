#!/usr/bin/python3
# file RefPerSys/tools/oidpython.py
# SPDX-License-Identifier: GPL-3.0-or-later
# Author(s):
#      Basile Starynkevitch <basile@starynkevitch.net>
#      © Copyright 2019 - 2023 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/
#


## the hope is to write Rps_Oid(1389064578974621886,517442547)
## or Rps_Oid("_1EBVGSfW2m200z18rx") to get the oid of our name root
class Rps_Oid:
    """Python's variant of RefPerSys object id

    Should be in sync with RefPerSys/oid_rps.hh C++ file
    """
    oid_hi: int
    oid_lo: int

    def __init__(self, hi : int, lo : int):
        # is this the right syntax? How to check that hi & lo are integers
        self.oid_hi = hi
        self.oid_lo = lo

    def __init__(self, s : string)
        # missing code
