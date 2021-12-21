#!/bin/bash
#
grep -n --color "$1" src/femera/*.?pp src/femera/*.h src/femera/*.?pp \
  src/fmr/*.?pp src/fmr/*.h src/fmr/*.?pp
#
