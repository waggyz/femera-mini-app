#!/bin/bash
# Usage: md5-all.sh $BUILD_DIR # writes all current sources md5 to $BUILD_DIR
# md5-all.sh .                 # updates all sources md5.
# md5-all.sh -                 # echos current value of .md5.
BASE=$(dirname "${0}")
cd $BASE
cd ../

if [ "$1" != "-" ]; then
  if [ "$1" == "." ]; then
    printf "" > .md5
    > .md5
  else
    printf "" > $1/.md5
    > $1/.md5
  fi
fi
for DIR in src data extras tools; do
  MD5=`tools/md5-dir-content.sh $DIR`
  echo $MD5
  if [ "$1" == "." ]; then
    echo $MD5 >  $DIR/.md5
    echo $MD5 >> .md5
  else
    if [ -z "$1" ]; then
      if [ "$1" != "-" ]; then
        echo $MD5 >  $1/$DIR.md5
        echo $MD5 >> $1/.md5
      fi
    fi
  fi
done
