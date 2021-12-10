#!/bin/bash
# NOTE Only use this in serial parts of a makefile.
SPC70="                                                                      "
CLRLINE=`printf "\r        $SPC70\r"`
function watchlastline {
  # call: watchlastline <interval> <filename>
  set -e
  #WATCH="  tail  "
  START=$SECONDS
  while [ "$LINE" != "fmr::EOF" ]; do
    ELAPSED=$(( SECONDS - START ))
    H=$(( ELAPSED / 3600 ))
    M=$(( ELAPSED / 60 % 60 ))
    S=$(( ELAPSED % 60 ))
    printf "\r%1i:%02i:%02i $SPC70\r%1i:%02i:%02i %-.70s" \
      $H $M $S $H $M $S "$LINE";
    sleep $1;
    # Get the last non-blank line from the last few lines.
    LINE=`tail -n5 "$2" | tac | grep -m1 .`
  done
}
touch "$4.out";
watchlastline 2 "$4.out" &
$3 >"$4.out" 2>"$4.err";
echo "fmr::EOF" >>"$4.out";
if [[ "$?" -eq 0 ]]; then # truncate command if ok,
  if [ ${3%% *} != ${3##* } ]; then DD=" .."; fi
  if [ -s "$4.err" ]; then
    printf "$CLRLINE$1 %-.70s\n" "${3%% *}$DD, but see:";
    printf "        $4.err\n";
  else
    printf "$CLRLINE$1 %-.70s\n" "${3%% *}$DD";
  fi;
else # include arguments if returned nonzero
  printf "$CLRLINE$2 $3\n";
  if [ -f "$4.out" ]; then tail "$4.out" >&2; fi;
  if [ -f "$4.err" ]; then tail "$4.err" >&2; fi;
fi;
#
