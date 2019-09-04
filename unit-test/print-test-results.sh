#!/bin/bash
PASS_COLOR="\e[92m"
FAIL_COLOR="\e[91m\e[1m\e[7m"
NORM_COLOR="\e[0m"
#
C=`tput cols`;
if [ -z $C ]; then C=80; fi
#
while IFS= read -r LINE; do
  if [ `echo $LINE | cut -c1` -eq 0 ];
  then printf "$PASS_COLOR Pass $NORM_COLOR ";
  else printf "$FAIL_COLOR FAIL $NORM_COLOR ";
  fi;
  echo $1`echo $LINE | cut -c2-$C` | cut -c1-$(( $C - 7 ))
done < "$2"
#
