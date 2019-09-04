#!/bin/bash
#
ALLFLAGS=`grep -i -m1 flags /proc/cpuinfo | tr '[a-z]' '[A-Z]'`
for FLAG in SSE SSE2 AVX AVX2 AVX-256 AVX-512 AVX512; do
  if [[ $ALLFLAGS =~ $FLAG ]]; then
    MYFLAGS=$MYFLAGS" "$FLAG
  fi
done;
echo $MYFLAGS