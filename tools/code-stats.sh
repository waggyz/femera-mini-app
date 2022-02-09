#!/bin/bash
#
CODE_FILES=`find src \( -name '*.?pp' -o -name '*.bats' \) -printf x | wc -c`
TEST_FILES=`find src \( -name '*.gtst.?pp' -o -name '*.bats' \) -printf x | wc -c`
CODE_LINES=`(find src \( -name '*.?pp' -o -name '*.bats' \) -print0 | xargs -0 cat) | wc -l`
TEST_LINES=`(find src \( -name '*.gtst.?pp' -o -name '*.bats' \) \
  -print0 | xargs -0 cat) | wc -l`
#
LOC1="lines of source code in"
LOC2="lines of  test  code in"

printf "  src/  counted   %5i %s %4i %s\n" $CODE_LINES "$LOC1" $CODE_FILES "files (*.?pp) total,"
printf "        including %5i %s %4i %s\n" $TEST_LINES "$LOC2" $TEST_FILES "files (*.gtst.?pp, *.bats)"

TD="TO""DO"
FM="FIX""ME"

check_dirs='src data docs examples extras tests tools'
check_files='Makefile README* config*'
grep -rn "$TD" $check_dirs $check_files > "build/"$TD".txt"
grep -rn "$FM" $check_dirs $check_files > "build/"$FM".txt"

TD+=" "
source_dirs='src/femera src/fmr'
NTD=`grep -rn "$TD" $source_dirs | tee "build/src-"$TD".txt" | wc -l`
NFM=`grep -rn "$FM" $source_dirs | tee "build/src-"$FM".txt" | wc -l`
printf "        with       %4i $FM see: build/src-$FM.txt\n" "$NFM"
printf "        and        %4i $TD see: build/src-$TD.txt\n" "$NTD"

printf "%5i,%5i,%5i,%5i,%5i,%5i" \
  $CODE_LINES $CODE_FILES $TEST_LINES $TEST_FILES $NFM $NTD \
  > build/src-code-stats.csv
