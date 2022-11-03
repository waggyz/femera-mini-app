#!/bin/bash
#
DIR="$1"
URL="$2"
BRANCH="$3"
TAG="$4"
#
if [ -z "$DIR" ]; then
  echo "git.submodule.add: the first argument, submodule directory, is empty" >2
return 1
fi
if [ -z "$URL" ]; then
  echo "git.submodule.add: the second argument, repository URL, is empty" >2
return 1
fi
if [ ! -f ".gitmodules" ]; then
  touch .gitmodules
fi
if [ -z "$BRANCH" ]; then
  git submodule add --force -b $URL $DIR
else
  git submodule add --force -b $BRANCH $URL $DIR
fi
if [ -n "$TAG" ]; then
  if [ -d "$DIR" ]; then
    cd $DIR
    git fetch
    git checkout $TAG
    cd ../
  fi
fi
#
