#!/bin/bash
#
if which lscpu >/dev/null 2>/dev/null; then
  allflags=`lscpu | grep -i -m1 'flags' | tr '[a-z]' '[A-Z]'`
else
  allflags=`grep -i -m1 flags /proc/cpuinfo | tr '[a-z]' '[A-Z]'`
fi
for flag in SSE SSE2 AVX AVX2 FMA AVX-256 AVX-512 AVX512; do
  if [[ $allflags =~ $flag ]]; then
    myflags=$myflags" "$flag
  fi
done;
echo $myflags
