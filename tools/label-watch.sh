#!/bin/bash
SPC70="                                                                      "
CLRLINE=`printf "\r        $SPC70\r"`
function watchlastline {
  # call: watchlastline <interval> <filename>
  WATCH="  tail  "
  while [ "$LINE" != "fmr::EOF" ]; do
    printf "\r$WATCH$SPC70\r$WATCH%-.70s" "$LINE";
    sleep $1;
    LINE=`tail -n1 $2`;
  done
}
touch "$4.out";
watchlastline 2 "$4.out" &
$3 >"$4.out" 2>"$4.err";
echo "fmr::EOF" >>"$4.out";
if [[ "$?" -eq 0 ]]; then # truncate command if ok,
  if [ ${3%% *} != ${3##* } ]; then DD=" .."; fi
  printf "$CLRLINE$1 %-.70s\n" "${3%% *}$DD";
  if [ -s "$4.err" ]; then printf "        but see: $4.err\n"; fi;
else # include arguments if returned nonzero
  printf "$CLRLINE$2 $3\n";
  if [ -f "$4.out" ]; then tail "$4.out" >&2; fi;
  if [ -f "$4.err" ]; then tail "$4.err" >&2; fi;
fi;
#
