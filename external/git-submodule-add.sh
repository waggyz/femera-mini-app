#!/bin/bash
#
DIR="$1"
URL="$2"
BRANCH="$3"
TAG="$4"
#
echo "**** $1 $2 $3 $4 ****"
if [ -z "$DIR" ]; then
  echo "git.submodule.add: first argument, submodule directory, is empty" >2
return 1
fi
if [ -z "$URL" ]; then
  echo "git.submodule.add: second argument, repository URL, is empty" >2
return 1
fi
#if [ -d "$DIR" ]; then
#  git rm $DIR
  if [ ! -f ".gitmodules" ]; then
    touch .gitmodules
  fi
#fi
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
    #git add $DIR
  fi
fi
#
