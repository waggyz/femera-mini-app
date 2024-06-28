#!/bin/bash
#
grep --no-filename '^[[:space:]]*tdd:' $@ | sed 's/^[[:space:]]*tdd://' \
| sed 's/\\//g'
#
#TODO Should be able to do this with a shorter pipe chain.
#sed 's/^tdd:(.*)$/\1/' "$@" | sed 's/\\//g'
exit 0
