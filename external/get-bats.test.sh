#!/usr/bin/bash
#NOTE Cannot use Bats before it's cloned, so this test suite is a Bash script.
function readme_exists () {
  FILE=$REPO_DIR"/"$1"/README.md"
  if [ ! -f $FILE ]; then
    echo " WARN  Cannot find "$FILE"."
    echo " HINT  Update Bats submodules."
    echo "       make get-bats # Get Bats source code."
    return 1;
  else
    echo "  OK   Found "$FILE"."
  fi
}
REPO_DIR="$1"
if [ -z "$REPO_DIR" ]; then
  REPO_DIR=external
fi
RET=0
readme_exists "bats-core"   ;R=$?; if [ $R != 0 ]; then RET=$R; fi
readme_exists "bats-assert" ;R=$?; if [ $R != 0 ]; then RET=$R; fi
readme_exists "bats-support";R=$?; if [ $R != 0 ]; then RET=$R; fi
readme_exists "bats-file"   ;R=$?; if [ $R != 0 ]; then RET=$R; fi
exit $RET
#
# The rest is currently disabled.
if [ $RET == 0 ]; then
  echo "  OK   Found Bats source code in "$REPO_DIR"/bats-*/."
else
  echo " WARN  Did not find Bats source code in "$REPO_DIR"/bats-*/."
  echo " HINT  Configure Femera to use Bats then download dependencies."
  echo "       Set ENABLE_BATS:=ON in config.local, then:"
  echo "       make external # Get source code for Femera dependencies."
fi
exit $RET
