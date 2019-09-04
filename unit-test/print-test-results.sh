#!/bin/bash
PASS_COLOR="\e[92m"
INFO_COLOR="\e[94m"
NOTE_COLOR="\e[96m"
WARN_COLOR="\e[93m\e[1m"
FAIL_COLOR="\e[91m\e[1m\e[7m"
NORM_COLOR="\e[0m"
#
PASS="$PASS_COLOR Pass $NORM_COLOR"
INFO="$INFO_COLOR Info $NORM_COLOR"
NOTE="$NOTE_COLOR Note $NORM_COLOR"
WARN="$WARN_COLOR WARN $NORM_COLOR"
FAIL="$FAIL_COLOR FAIL $NORM_COLOR"
#
COLS=`tput cols`;
if [ -z $COLS ]; then COLS=80; fi
#
while IFS= read -r LINE; do
  if [ `echo $LINE | cut -c1` -eq 0 ];
  then printf "$PASS_COLOR Pass $NORM_COLOR ";
  else printf "$FAIL_COLOR FAIL $NORM_COLOR ";
  fi;
  STRLEN=`echo $LINE | wc -c`
  NOANSILEN=`echo $LINE | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | wc -c`
  FIT=$(( $COLS + $STRLEN - $NOANSILEN - 7 ))
  echo $1`echo $LINE | cut -c2-` | cut -c1-$FIT
done < "$2"
#
