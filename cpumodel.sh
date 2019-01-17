#!/bin/bash
CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($7)}'`
if [ "$CPUMODEL" == "CPU" ];then CPUMODEL=`grep -m 1 "model name" /proc/cpuinfo | awk '{print($6)}'`;fi
echo $CPUMODEL
