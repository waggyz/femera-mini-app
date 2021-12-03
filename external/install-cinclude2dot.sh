#!/bin/bash
# module unload anaconda_3 #NOTE Needed for K
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if which cinclude2dot 2>/dev/null ; then
  echo "looks like cinclude2dot is already installed"
  exit 0
else
  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/tools"
  BUILD_DIR="$FMRDIR/build/external/tools"

  mkdir -p "$SRC_DIR" $INSTALL_DIR"/bin"
  if [ -f "$BUILD_DIR/cinclude2dot" ]; then
    cp $BUILD_DIR"/cinclude2dot" $SRC_DIR"/cinclude2dot"
    cp $BUILD_DIR"/cinclude2dot" $INSTALL_DIR"/bin/cinclude2dot"
    exit 0
  else
    echo " ERROR  Could not find $BUILD_DIR/cinclude2dot" >&2
    exit 1
  fi
fi
