#!/bin/bash

CPUMODEL=`./cpumodel.sh`
CPUCOUNT=`./cpucount.sh`
ITERS=1000
CSTR=icc
YSTR=iso

advixe-cl --collect survey --no-auto-finalize\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-10mdof-tet10-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  /u/dwagner5/femera-mini-develop/"femerq-"$CPUMODEL"-"$CSTR -v1 -c$CPUCOUNT -r0 -i$ITERS -p /hpnobackup1/dwagner5/femera-test/cube/uhxt123p2/uhxt123p2n1920

advixe-cl --collect tripcounts --flop --stacks --no-auto-finalize\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-10mdof-tet10-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  /u/dwagner5/femera-mini-develop/"femerq-"$CPUMODEL"-"$CSTR -v1 -c$CPUCOUNT -r0 -i$ITERS -p /hpnobackup1/dwagner5/femera-test/cube/uhxt123p2/uhxt123p2n1920

# 
  -resume-after=30 -stop-after=40