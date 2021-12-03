#!/bin/bash
# Updates or creates file $1 if content is different from file $2
#
if [ -n "$1" ]; then
  FILE1="$1";
else
  echo " ERROR  $0 needs a filename." >&2
  exit 1
fi
if [ -f "$2" ]; then
  FILE2="$2";
else
  FILE2="$1.new";
fi
if [ ! -f "$FILE2" ]; then
  echo " ERROR  $0 could not find file: $FILE2" >&2
  exit 1
fi
if test -r $FILE1; then
  cmp $FILE2 $FILE1 >/dev/null || ( mv -f $FILE2 $FILE1; )
else
  mv $FILE2 $FILE1;
fi
