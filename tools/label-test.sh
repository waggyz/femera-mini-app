#!/bin/bash
#
touch "$4.out";
$3 >"$4.out" 2>"$4.err";
if [[ "$?" -eq 0 ]]; then # truncate command if ok.
  if [ ${3%% *} != ${3##* } ]; then DD=" .."; fi
  printf "$1 %-.70s\n" "${3%% *}$DD";
  if [ -s "$4.err" ]; then printf "        but see: $4.err\n"; fi;
else # include arguments if returned nonzero
  printf "$2 $3\n";
  if [ -f "$4.out" ]; then tail "$4.out" >&2; fi;
  if [ -f "$4.err" ]; then tail "$4.err" >&2; fi;
fi;
#
