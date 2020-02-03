#!/bin/bash
CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($7)}'`
if [ "$CPUMODEL" == "CPU" ];then CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($6)}'`;fi
if [ "$CPUMODEL" == "Phi(TM)" ];then CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($8)}'`;fi
if [ "$CPUMODEL" == "Processor" ];then CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($8)}'`;fi
#
MEM_GB=`free -g  | grep Mem | awk '{print $7}'`
if [ $MEM_GB -gt 100 ]; then
  CPUMODEL=$CPUMODEL"b"
fi
#
echo $CPUMODEL
