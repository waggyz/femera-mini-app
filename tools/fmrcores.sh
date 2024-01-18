#!/bin/bash
if which lscpu >/dev/null 2>/dev/null; then
  # physical cores per socket
  ncore=`lscpu | grep -i -m1 "^Core" | awk 'NF>1{print $NF}'`
  [ "$ncore" -eq 0 ] && ncore="1"
  
  # Number of sockets
  nsock=`lscpu | grep -i -m1 "^Sock" | awk 'NF>1{print $NF}'`
  [ "$nsock" -eq 0 ] && nsock="1"
else
  # physical cores per socket
  ncore=$(grep -w "core id" /proc/cpuinfo | sort -u | wc -l)
  [ "$ncore" -eq 0 ] && ncore="1"
  
  # Number of sockets
  nsock=$(grep -w "physical id" /proc/cpuinfo | sort -u | wc -l)
  [ "$nsock" -eq 0 ] && nsock="1"
fi

echo $(expr $ncore \* $nsock)

exit

# logical cores
# grep -w "processor" /proc/cpuinfo | sort -u | wc -l

#NCORE=`grep ^cpu\\\scores /proc/cpuinfo | uniq |  awk '{print($4)}'`
#NCORE=`grep ^siblings /proc/cpuinfo | uniq |  awk '{print($3)}'`

#NSOCK=`grep "physical id" /proc/cpuinfo | sort -u | wc -l`
#echo $NCORE
#echo $NSOCK
#NCPU=`expr $NCORE \* $NSOCK`
#echo $NCPU

  # b4 RHEL8: line=`lscpu | grep -i -m1 "^Core"`
  # line=`lscpu | grep -i -m1 "^CPU(s)"`
  # echo ${line##* }
