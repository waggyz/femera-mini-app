#!/bin/bash
if which lscpu >/dev/null 2>/dev/null; then
  line=`lscpu | grep -i -m1 "^Core"`
  echo ${line##* }
else
  nsock=$(grep -w "physical id" /proc/cpuinfo | sort -u | wc -l)
  [ "$nsock" -eq 0 ] && nsock="1"

  # physical cores
  ncore=$(grep -w "core id" /proc/cpuinfo | sort -u | wc -l)
  [ "$ncore" -eq 0 ] && ncore="1"

  echo $(expr $ncore \* $nsock)
fi
# logical cores
# grep -w "processor" /proc/cpuinfo | sort -u | wc -l

#NCORE=`grep ^cpu\\\scores /proc/cpuinfo | uniq |  awk '{print($4)}'`
#NCORE=`grep ^siblings /proc/cpuinfo | uniq |  awk '{print($3)}'`

#NSOCK=`grep "physical id" /proc/cpuinfo | sort -u | wc -l`
#echo $NCORE
#echo $NSOCK
#NCPU=`expr $NCORE \* $NSOCK`
#echo $NCPU
