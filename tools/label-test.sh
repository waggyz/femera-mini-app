#!/bin/bash
#
touch "$4.out";
$3 >"$4.out" 2>"$4.err";
if [[ "$?" -eq 0 ]]; then # truncate command if ok.
#  if [ ${3%% *} != ${3##* } ]; then DD=" .."; fi
  if [ "${3%% *}" == "${3##* }" ]; then
    FIRSTLAST="${3%% *}"
  else
    FIRSTLAST="${3%% *} ${3##* }"
    if [ "$FIRSTLAST" != "$3" ]; then FIRSTLAST="${3%% *} .. ${3##* }"; fi
  fi
#  MEM=`ls -sh $4 | cut -d " " -f1`"B"
#  ARR=($3); W2=`echo ${ARR[@]:0:2}`
  if [ -s "$4.err" ]; then
#    printf "$1 %-.70s\n" "$3, but see:";
#    printf "$1 %-.70s\n" "${3%% *}$DD, but see:";
#    printf "$1 %-.70s\n" "$W2, but see:";
    printf "$1 %-59s%8s\n" "$FIRSTLAST, but" "$MEM";
    printf "  see:  $4.err\n";
  else
#    printf "$1 %-.70s\n" "$3";
#    printf "$1 %-.70s\n" "${3%% *}$DD";
#    printf "$1 %-.70s\n" "$W2";
#    printf "$1 %-.70s\n" "${3%% *}$DD";
    printf "$1 %-64s%8s\n" "$FIRSTLAST" "$MEM";
  fi;
else # include arguments if returned nonzero
  printf "$2 $3\n";
  if [ -f "$4.out" ]; then tail "$4.out" >&2; mv -f "$4.out" "$4.bad"; fi;
  if [ -f "$4.err" ]; then tail "$4.err" >&2; fi;
fi;
#
