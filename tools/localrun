#!/bin/bash
#
# localrun gmsh
#
LOCAL_INSTALL="/home/dwagner5/.local"

if [ -f "$LOCAL_INSTALL/bin/fmrmodel" ]; then
  CPUMODEL=`$LOCAL_INSTALL/bin/fmrmodel`
else
  CPUMODEL="unknown-cpu"
fi

if ! [[ "$LD_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/lib64:" ]]
TMP_LIBRARY_PATH=$LD_LIBRARY_PATH
then
  TMP_LIBRARY_PATH="$LOCAL_INSTALL/lib64:$TMP_LIBRARY_PATH"
fi
if ! [[ "$TMP_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/lib:" ]]
then
  TMP_LIBRARY_PATH="$LOCAL_INSTALL/lib:$TMP_LIBRARY_PATH"
fi
if ! [ -z "$CPUMODEL" ]; then
  if ! [[ "$TMP_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/$CPUMODEL/lib64:" ]]
  then
    TMP_LIBRARY_PATH="$LOCAL_INSTALL/$CPUMODEL/lib64:$TMP_LIBRARY_PATH"
  fi
  if ! [[ "$LD_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/$CPUMODEL/lib:" ]]
  then
    TMP_LIBRARY_PATH="$LOCAL_INSTALL/$CPUMODEL/lib:$TMP_LIBRARY_PATH"
  fi
fi

echo LD_LIBRARY_PATH=$TMP_LIBRARY_PATH $@

if [ ! -z "$@" ]; then
  LD_LIBRARY_PATH=$TMP_LIBRARY_PATH $@
fi

exit $?
#
