#!/bin/bash
#
find src -maxdepth 3 -xdev -type f -name '*.?pp' -exec grep "$1" -l {} \; \
  -exec sed -i "s/$1/$2/g" {} \;
find src -maxdepth 3 -xdev -type f -name '*.h' -exec grep "$1" -l {} \; \
  -exec sed -i "s/$1/$2/g" {} \;
#
#sed -i "s/$1/$2/g" src/*.?pp src/*/*.?pp
#
