#!/bin/bash
#

CODE_FILES=`find src -name '*.?pp' -printf x | wc -c`
TEST_FILES=`find src \( -name '*.gtst.?pp' -o -name '*.bats' \) -printf x | wc -c`
CODE_LINES=`(find src \( -name '*.?pp' \) -print0 | xargs -0 cat) | wc -l`
TEST_LINES=`(find src \( -name '*.gtst.?pp' -o -name '*.bats' \) \
  -print0 | xargs -0 cat) | wc -l`
#
LOC="lines of source code in"

printf "        counted %5i %s %4i %s\n" $CODE_LINES "$LOC" $CODE_FILES "files (*.?pp), including"
printf "                %5i %s %4i %s\n" $TEST_LINES "$LOC" $TEST_FILES "files (*.gtst.?pp, *.bats)"

TD="TO""DO"
FM="FIX""ME"

check_dirs='src data docs examples extras tests tools'
check_files='Makefile README* config*'
grep -rn "$TD" $check_dirs $check_files > "build/"$TD".txt"
grep -rn "$FM" $check_dirs $check_files > "build/"$FM".txt"

source_dirs='src/femera src/fmr'
NTD=`grep -rn "$TD" $source_dirs | tee "build/src-"$TD".txt" | wc -l`
NFM=`grep -rn "$FM" $source_dirs | tee "build/src-"$FM".txt" | wc -l`
printf "        with     %4i $FM (build/src-$FM.txt)\n" "$NFM"
printf "        and      %4i $TD  (build/src-$TD.txt)\n" "$NTD"