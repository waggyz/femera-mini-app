#!/bin/bash
# module unload anaconda_3 #NOTE Needed for K
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
DOWNLOAD_DIR="$FMRDIR/build/external/mkl"
KITFILE=`ls -d "$DOWNLOAD_DIR/"*"Kit"*".sh" | tail -n1`

if [ -f "$KITFILE" ]; then
  SRC_DIR="$FMRDIR/external/mkl"
  BUILD_DIR="$FMRDIR/build/external/mkl"
  mkdir -p "$SRC_DIR" # "$INSTALL_DIR"
  #
  echo $KITFILE $FLAGS
  $KITFILE $FLAGS
  #
  exit
else
  echo " ERROR  Could not find MKL installer in:" >&2
  echo "        $DOWNLOAD_DIR/" >&2
  exit 1
fi

