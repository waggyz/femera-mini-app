#!/bin/bash
# Hash of the contents of files, starting in the directory specified
cd $1

find . \( -name \*.h -o -name \*.c -o -name \*.hpp -o -name \*.cpp \
  -o -name \*.py -o -name \*.sh -o -name \*.txt \) -type f \
  -exec md5sum {} \;| sort -k 2 | md5sum | cut -d ' ' -f 1
