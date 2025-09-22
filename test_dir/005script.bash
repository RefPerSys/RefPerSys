#!/bin/bash
if [ -x refpersys ]; then
    echo 'no refpersys executable in ' $(/bin/pwd) > /dev/stderr
    exit 1
fi
./refpersys -AREPL --script=$0 --batch --run-name=$0

## magic string REFPERSYS_SCRIPT foo
## etc
