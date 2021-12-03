#!/bin/bash
#
if which lscpu >/dev/null 2>/dev/null; then
  line=`lscpu | grep -i -m1 "^NUMA node[(s]"`
  echo ${line##* }
else
  echo 1
  echo "  NOTE  fmrnumas did NOT check memory layout and assumed 1 NUMA domain." \
    1>&2
  exit 11
fi
