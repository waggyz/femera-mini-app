#!/bin/bash
NSOCK=$(grep -w "physical id" /proc/cpuinfo | sort -u | wc -l)
[ "$NSOCK" -eq 0 ] && NSOCK="1"

# physical cores
NCORE=$(grep -w "core id" /proc/cpuinfo | sort -u | wc -l)
[ "$NCORE" -eq 0 ] && NCORE="1"

echo $(expr $NCORE \* $NSOCK)

# logical cores
# grep -w "processor" /proc/cpuinfo | sort -u | wc -l

#NCORE=`grep ^cpu\\\scores /proc/cpuinfo | uniq |  awk '{print($4)}'`
#NCORE=`grep ^siblings /proc/cpuinfo | uniq |  awk '{print($3)}'`

#NSOCK=`grep "physical id" /proc/cpuinfo | sort -u | wc -l`
#echo $NCORE
#echo $NSOCK
#NCPU=`expr $NCORE \* $NSOCK`
#echo $NCPU
