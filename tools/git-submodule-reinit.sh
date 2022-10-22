#!/bin/bash
#

# https://stackoverflow.com/questions/34890313/deleting-all-git-cached-submodules-from-repository
if [ 1 -eq 1 ]; then
  echo 'deinit all submodules from .gitmodules...'
  git submodule deinit .
  echo 'remove all submodules (`git rm`) from .gitmodules...'
  #git submodule | cut -c43- | while read -r line; do (git rm "$line"); done
  git submodule | awk '{print $2}' | while read -r line; do (git rm "$line"); done
fi
echo 'add all submodules...'
for I in {1..10}; do
  DIR=""
  URL=""
  BRANCH="master"
  case $I in
  1)
  DIR=external/bats-core
  URL=https://github.com/bats-core/bats-core.git
  BRANCH=master
  ;;
  2)
  DIR=external/bats-support
  URL=https://github.com/bats-core/bats-support.git
  BRANCH=master
  ;;
  3)
  DIR=external/bats-assert
  URL=https://github.com/bats-core/bats-assert.git
  BRANCH=master
  ;;
  4)
  DIR=external/bats-file
  URL=https://github.com/bats-core/bats-file.git
  BRANCH=master
  ;;
  5)
  DIR=external/pybind11
  URL=https://github.com/pybind/pybind11.git
  BRANCH=master
  ;;
  6)
  DIR=external/cgns
  URL=https://github.com/CGNS/CGNS.git
  BRANCH=master
  ;;
  7)
  DIR=external/freetype
  URL=https://gitlab.freedesktop.org/freetype/freetype.git
  BRANCH=master
  ;;
  8)
  DIR=external/fltk
  URL=https://github.com/fltk/fltk.git
  BRANCH=branch-1.3
  ;;
  9)
  DIR=external/gmsh
  URL=https://gitlab.onelab.info/gmsh/gmsh.git
  BRANCH=master
  ;;
  10)
  DIR=external/petsc
  URL=https://gitlab.com/petsc/petsc.git
  BRANCH=release
  ;;
  esac
  if [ "$DIR" != "" ]; then
    git submodule add --force -b "$BRANCH" "$URL" "$DIR"
  fi
done
echo 'update and init all submodules and their submodules recursively...'
git submodule update --init --recursive 
#
