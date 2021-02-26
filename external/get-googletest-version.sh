#!/bin/bash
#
cd external/googletest;
# echo `git describe --tags | sed 's/^.*[^0-9]\([0-9]*\.[0-9]*\.[0-9]*\).*$/\1/'`
echo `git describe --tags | sed 's/^.*[^0-9]\([0-9]*\.[0-9]*\)\..*$/\1/'`
#
