#!/bin/bash
# Prints the first word containing digits from the model name in /proc/cpuinfo
#
if which lscpu >/dev/null 2>/dev/null; then
  cpumodel=`lscpu | grep -i  -m1 '^model name' \
  | sed -e 's/\ /\n/g' \
  | grep -m 1 '[0-9]'`
  if [ "$cpumodel" == "12th" ]; then
    cpumodel=`lscpu | grep -i  -m1 '^model name' \
    | sed -e 's/\ /\n/g' \
    | grep -m 2 '[0-9]' | tail -n1`
  fi
else
  cpumodel=`grep -m 1 "model name" /proc/cpuinfo \
  | sed -e 's/\ /\n/g' \
  | grep -m 1 '[0-9]'`
fi
if [ -z "$cpumodel" ]; then
  echo "unknown-cpu"
  echo "fmrmodel.sh: unknown processor model" 1>&2
  exit 1
fi
echo $cpumodel
