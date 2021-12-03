#!/bin/bash
# Wrap text file content into a C-style string to #include in a C source file.
case "$#" in
  0) ;; # Do nothing. TODO accept stdin as input.
  1)    # Wrap contents of text in file $1 to stdout.
  printf "R\"+++++("
  cat $1
  printf ")+++++\""
  ;;
  *)    # Wrap contents of file $1 to file $2.
  printf "R\"+++++(" > $2
  cat $1 >> $2
  printf ")+++++\"" >> $2
  ;;
esac
