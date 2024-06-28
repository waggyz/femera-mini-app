#!/bin/bash
BASE=$(dirname "${0}")
cd $BASE
cd ../
#NOTE This works when this script is within a top-level directory.

RELEASE=`cat data/VERSION`
if [ -f data/PRE_RELEASE ]; then
  PRE_RELEASE=`cat data/PRE_RELEASE`
  if [ -n "$PRE_RELEASE" ]; then
    PRE_RELEASE=-$PRE_RELEASE
  fi
fi
if [ 0 -eq 1 ]; then # FIXME DISABLED
  # Check if user has modified source code, data, or tests.
  LOCAL_MD5=`tools/md5-all.sh -`
  REPO_MD5=`cat .md5`
  MOD=`diff <(echo "$LOCAL_MD5") <(echo "$REPO_MD5")`
    if [ -n "$MOD" ]; then
      #if [ -e "$LOCAL_BUILD_ID" ]; then
      #  MOD="mod"
      #else
        MOD="-mod"
      #fi
    fi
fi
# Check the git repository (if present) to determine version from release tag.
IS_IN_REPO=`git rev-parse --is-inside-work-tree 2>/dev/null`
if [ "$IS_IN_REPO"=="true" ]; then
  DESC=`git describe --long --dirty 2>/dev/null`
  # Use --tags to find lightweight tags, instead of only annotated tags.
  if [ -n "$DESC" ]; then
    #BR=`git rev-parse --abbrev-ref HEAD`
    #if [ "$BRANCH"!="master" ]; then
    #  BRANCH="$BR"
    #fi
    DESC_ARRAY=(${DESC//\-/ }) # Parse on hyphens into array.
    BUILD_TAG=${DESC_ARRAY[0]}
    COMMITS_PAST=${DESC_ARRAY[1]}
    COMMIT=${DESC_ARRAY[2]}
    DIRTY=${DESC_ARRAY[3]}
    if [ -n "$DIRTY" ]; then
      #if [ "$LOCAL_BUILD_ID"=="" ]; then
        # MOD="-mod"
        MOD="$MOD"
      #else
      #  MOD="-mod-"$LOCAL_BUILD_ID
      #fi
    fi
    BUILD_INFO="+"$COMMIT""$MOD""
    # v 0.3 demo branch started at 204 commits ahead of master.
    COMMITS_PAST=`echo "$COMMITS_PAST-204" | bc`
  fi
else # Not in git repository
  #COMMITS_PAST="$LOCAL_BUILD_ID"
  COMMITS_PAST="x"
  BUILD_INFO="-"$MOD
fi
#
echo $RELEASE"."$COMMITS_PAST""$PRE_RELEASE""$BUILD_INFO
#
