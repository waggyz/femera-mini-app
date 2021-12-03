#!/bin/bash
# e.g., external/get-external bats-core
# or    bats-support, bats-assert, bats-file
#
if [ -z "$1" ]; then
  # REPO="bats-core"
  echo "ERR""OR external/get-external.sh requires an argument." >&2
  exit 1
else
  REPO=$1
fi
BUILDDIR="build/external"
DESTDIR="external"
mkdir -p $BUILDDIR $DESTDIR

WGET="wget --no-check-certificate"

unset CKFILE GITMOD GITURL ZIPURL ZIPDIR
if [[ $REPO == bats-* ]]; then
  GITMOD="tools"
  BRANCH="master"
  GITSSH="git@github.com:bats-core/"$REPO".git"
  GITURL="https://github.com/bats-core/"$REPO".git"
  ZIPURL="https://github.com/bats-core/"$REPO"/archive/refs/heads/master.zip"
  ZIPDIR=$DESTDIR"/"$REPO"-master"
  CKFILE=$DESTDIR"/"$REPO"/README.md"
else
  DATFILE="external/get-$REPO.dat"
  if [ -f $DATFILE ]; then
    while read LINE; do
        LABEL=${LINE%% *}
        VAL=${LINE##* }
        case $LABEL in
        GITMOD) GITMOD=$VAL ;; #REPO=$GITMOD ;;
        BRANCH) BRANCH=$VAL ;;
        GITSSH) GITSSH=$VAL ;; #FIXME Not used yet
        GITURL) GITURL=$VAL ;;
        ZIPURL) ZIPURL=$VAL ;;
        ZIPDIR) ZIPDIR=$VAL ;;
        TGZURL) TGZURL=$VAL ;;
        TGZDIR) TGZDIR=$VAL ;;
        EXEURL) EXEURL=$VAL ;;
        CKFILE) CKFILE=$VAL ;;
        esac
    done < "$DATFILE"
  else
    echo " ERROR  $DATFILE not found." >&2
    exit 1
  fi
fi
if [ -f $CKFILE ]; then
  # echo "looks like $REPO source code is already available."
  DEST_IS_REPO=`cd $DESTDIR/$GITMOD &&git rev-parse --is-inside-work-tree 2>/dev/null`
  if [ "$DEST_IS_REPO" == "true" ]; then
    cd $DESTDIR/$GITMOD
    DEST_BRANCH=`git rev-parse --abbrev-ref HEAD`
    if [ -n "$BRANCH" ]; then
      if [ "$BRANCH" != "$DEST_BRANCH" ]; then # switch branch
        git checkout $BRANCH
        git submodule update --init
        wait
        echo "checked out $BRANCH"
      fi
    fi
    cd ../
  fi
  exit 0
fi
if [ -f $CKFILE ]; then
  if [ -n "$BRANCH" ]; then
    echo "updated git repo $GITMOD branch $BRANCH"
  else
    echo "updated git repo $GITMOD"
  fi
  exit 0
fi
if [ -n "$GITMOD" ]; then # Try updating git submodule.
  IS_IN_REPO=`git rev-parse --is-inside-work-tree 2>/dev/null`
  if [ "$IS_IN_REPO" == "true" ]; then
    git submodule update --init --recursive $GITMOD
    #  --OR--
    #  git submodule update --remote $GITMOD
    #if [ -n "$BRANCH" ]; then
    #  if [ -d "$DESTDIR/$GITMOD" ]; then
    #    cd $DESTDIR/$GITMOD
    #    git checkout $BRANCH
    #    wait
    #    cd ../
    #    echo "checked out $BRANCH"
    #  fi
    #fi
  fi
fi
if [ -f $CKFILE ]; then
  echo "updated git sumbmodule $GITMOD"
  exit 0
fi
if [ -n "$GITURL" ]; then # Try cloning it.
  cd $DESTDIR
  if [ -n "$BRANCH" ]; then
    git clone -b $BRANCH --recurse-submodules $GITURL
  else
    git clone --recurse-submodules $GITURL
  fi
  wait
  #if [ -n "$BRANCH" ]; then
  # if [ -d "$DESTDIR/$GITMOD" ]; then
  #   cd $DESTDIR/$GITMOD
  #   git checkout $BRANCH
  #   cd ../
  #     echo "checked out $BRANCH"
  # fi
  #fi
  cd ../
fi
if [ -f $CKFILE ]; then
  if [ -n "$BRANCH" ]; then
    echo "cloned $GITURL branch $BRANCH"
  else
    echo "cloned $GITURL"
  fi
  exit 0
fi
if [ -n "$ZIPURL" ]; then # Try downloading and extracting a zip file.
  if [ ! -f $BUILDDIR"/"$REPO".zip" ]; then
    $WGET $ZIPURL -O $BUILDDIR"/"$REPO".zip"
    wait
  fi
  if [ -f $BUILDDIR"/"$REPO".zip" ]; then
    unzip $BUILDDIR"/"$REPO".zip" -o -d $DESTDIR;
  fi
  if [ -n "$ZIPDIR" ]; then
    if [ "$ZIPDIR" != "$DESTDIR/$GITMOD" ]; then
      if [ -d "$ZIPDIR" ]; then
        rm -rf  "$DESTDIR/$GITMOD"
        mv $ZIPDIR  "$DESTDIR/$GITMOD"
      fi
    fi
  fi
fi
if [ -f $CKFILE ]; then
  echo "downloaded and extracted $ZIPURL"
  exit 0
fi
if [ -n "$TGZURL" ]; then # Try downloading and extracting a tgz file.
  if [ ! -f $BUILDDIR"/"$REPO".tgz" ]; then
    # echo $WGET $TGZURL -O $BUILDDIR"/"$REPO".tgz"
    $WGET $TGZURL -O $BUILDDIR"/"$REPO".tgz"
    wait
  fi
  if [ -f $BUILDDIR"/"$REPO".tgz" ]; then
    tar zxf $BUILDDIR"/"$REPO".tgz" --directory=$DESTDIR;
  fi
  if [ -n "$TGZDIR" ]; then
    if [ "$TGZDIR" != "$DESTDIR/$GITMOD" ]; then
      if [ -d "$TGZDIR" ]; then
        rm -rf  "$DESTDIR/$GITMOD"
        mv $TGZDIR  "$DESTDIR/$GITMOD"
      fi
    fi
  fi
fi
if [ -f $CKFILE ]; then
  echo "downloaded and extracted $TGZURL"
  exit 0
fi
if [ -n "$EXEURL" ]; then # Try downloading an executable file.
  if [ ! -f "$BUILDDIR/$GITMOD/$REPO" ]; then
    mkdir -p "$BUILDDIR/$GITMOD"
    $WGET "$EXEURL" -O "$BUILDDIR/$GITMOD/$REPO"
    wait
  fi
  if [ ! -f "$DESTDIR/$GITMOD/$REPO" ]; then
    mkdir -p "$DESTDIR/$GITMOD"
    cp "$BUILDDIR/$GITMOD/$REPO" "$DESTDIR/$GITMOD/$REPO"
  fi
fi
if [ -f $CKFILE ]; then
  chmod +x "$CKFILE"
  echo "downloaded executable $EXEURL"
  exit 0
fi

echo "WARN""ING unable to get $REPO" >&2
exit 1
