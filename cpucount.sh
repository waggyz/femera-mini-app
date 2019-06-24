#!/bin/bash
NCORE=`grep ^cpu\\\scores /proc/cpuinfo | uniq |  awk '{print($4)}'`
NSOCK=`grep "physical id" /proc/cpuinfo | sort -u | wc -l`
#echo $NCORE
#echo $NSOCK
NCPU=`expr $NCORE \* $NSOCK`
echo $NCPU