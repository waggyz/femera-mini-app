#!/bin/bash
# module unload anaconda_3 #NOTE Needed for K
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/mkl/latest/lib/intel64/libmkl_core.a" ]; then
  echo "looks like the Intel Math Kernel Library (MKL) is already installed"
  exit 0
fi
FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
DOWNLOAD_DIR="$FMRDIR/build/external/mkl"
KITFILE=`ls -d "$DOWNLOAD_DIR/"*"Kit"*".sh" | tail -n1`
if [ ! -f "$KITFILE" ]; then
  KITNAME="l_BaseKit_p_2021.4.0.3422_offline.sh"
  RUNURL="https://registrationcenter-download.intel.com/akdlm/irc_nas/18236/$KITNAME"
  DLFILE="$DOWNLOAD_DIR/$KITNAME"
  wget --no-check-certificate "$RUNURL" -O "$DLFILE"
  KITFILE=`ls -d "$DOWNLOAD_DIR/"*"Kit"*".sh" | tail -n1`
fi
if [ -f "$KITFILE" ]; then
  BUILD_DIR="$FMRDIR/build/external/mkl"
  SRC_DIR="$FMRDIR/external/mkl"
  if [ -d "$SRC_DIR" ]; then
    KITDIR=`ls -d "$SRC_DIR/"*"Kit"* | tail -n1`
    if [ -f "$KITDIR/install.sh" ]; then # Remove MKL
       echo "Remove MKL?"
#      $KITDIR/install.sh --silent --action remove
    fi
  else
    mkdir -p "$SRC_DIR"
  fi
  echo $KITFILE $FLAGS
  eval $KITFILE $FLAGS
  # https://www.intel.com/content/www/us/en/develop/documentation/installation-guide-for-intel-oneapi-toolkits-linux/top/installation/install-with-command-line.html#install-with-command-line
  #CKFILE="$FMRDIR/external/mkl/l_BaseKit_p_2021.4.0.3422_offline/README.md"
  exit
else
  echo " ERROR  Could not find MKL installer in:" >&2
  echo "        $DOWNLOAD_DIR/" >&2
  exit 1
fi

