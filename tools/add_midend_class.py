#!/usr/bin/python3

#* SPDX-License-Identifier: GPL-3.0-or-later
#*
#* Description:
#*

#*  This file is part of the Reflective Persistent System.
#*  It is a temporary Python script to add a middle-end code
#*  representation class (for objects or instances).
#*
#*
#* Author(s):
#*      Basile Starynkevitch <basile@starynkevitch.net>
#*      Abhishek Chakravarti <abhishek@taranjali.org>
#*      Nimesh Neema <nimeshneema@gmail.com>
#*
#*      © Copyright 2019 - 2023 The Reflective Persistent System Team
#*      team@refpersys.org & http://refpersys.org/
#*
#* You can consider RefPerSys as either GPLv3+ or LGPLv3+ licensed (at
#* your choice)
#*
#* License: GPLv3+ (file COPYING-GPLv3)
#*    This software is free software: you can redistribute it and/or modify
#*    it under the terms of the GNU General Public License as published by
#*    the Free Software Foundation, either version 3 of the License, or
#*    (at your option) any later version.
#*
#* Alternative license: LGPLv3+ (file COPYING-LGPLv3)
#*    This software is is free software: you can
#*    redistribute it and/or modify it under the terms of the GNU
#*    Lesser General Public License as published by the Free Software
#*    Foundation, either version 3 of the License, or (at your option)
#*    any later version.


parser = argparse.ArgumentParser(
                    prog='add_midend_class.py',
                    description='Add a middle end code representation class',
                    epilog='Add to RefPerSys a middle end code representation class')

parser.add_argument('-V', '--version',
                    action='show version information');

#@@@@ TODO ADD OTHER ARGUMENTS, INCLUDING --help

args = parser.parse_args();

raise RuntimeError("add_midend_class.py unimplemented")
