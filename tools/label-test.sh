#!/bin/bash
#
touch "$4.out";
$3 >"$4.out" 2>"$4.err";
if [[ "$?" -eq 0 ]]; then # truncate command if ok.
  if [ ${3%% *} != ${3##* } ]; then DD=" .."; fi
  if [ -s "$4.err" ]; then
    printf "$1 %-.70s\n" "${3%% *}$DD, but see:";
    printf "        $4.err\n";
  else
    printf "$1 %-.70s\n" "${3%% *}$DD";
  fi;
else # include arguments if returned nonzero
  printf "$2 $3\n";
  if [ -f "$4.out" ]; then tail "$4.out" >&2; mv -f "$4.out" "$4.bad"; fi;
  if [ -f "$4.err" ]; then tail "$4.err" >&2; fi;
fi;
#
