#!/bin/bash
#
external/git-submodule-add.sh external/bats-core \
  https://github.com/bats-core/bats-core.git master v1.8.2
external/git-submodule-add.sh external/bats-assert \
  https://github.com/bats-core/bats-assert.git master v2.1.0
external/git-submodule-add.sh external/bats-support \
  https://github.com/bats-core/bats-support.git master v0.3.0
external/git-submodule-add.sh external/bats-file \
  https://github.com/bats-core/bats-file.git master v0.3.0
#
external/git-submodule-add.sh external/pybind11 \
  https://github.com/pybind/pybind11.git master v2.10.1
#TODO Check if pybind11 should be tag v2.5.0
#
external/git-submodule-add.sh external/cgns \
  https://github.com/CGNS/CGNS.git master v4.3.0
#
external/git-submodule-add.sh external/freetype \
  https://gitlab.freedesktop.org/freetype/freetype.git master VER-2-12-1
#
external/git-submodule-add.sh external/fltk \
  https://github.com/fltk/fltk.git master release-1.3.8
#
external/git-submodule-add.sh external/gmsh \
  https://gitlab.onelab.info/gmsh/gmsh.git master gmsh_4_10_5
#
external/git-submodule-add.sh external/petsc \
  https://gitlab.com/petsc/petsc.git release v3.18.1
#
git submodule update --init --recursive
#
git commit -m "external/git-submodule-setup.sh: reset submodules"
#

if [ 0 -eq 1 ]; then
  #
  #NOTE Remove submodules that need to be changed, e.g., to a different version
  #     before running this.
  #
  # git submodule add -b $REPO_BRANCH $REPO_URL $DEST_FOLDER
  touch .gitmodules
  git submodule add https://github.com/bats-core/bats-core.git external/bats-core
  git submodule add https://github.com/bats-core/bats-support.git external/bats-support
  git submodule add https://github.com/bats-core/bats-assert.git external/bats-assert
  git submodule add https://github.com/bats-core/bats-file.git external/bats-file
  #
  git submodule add https://github.com/pybind/pybind11.git external/pybind11
  #TODO Check if pybind11 should be tag v2.5.0
  git submodule add -b master https://github.com/CGNS/CGNS.git external/cgns
  #
  git submodule add -b master https://gitlab.freedesktop.org/freetype/freetype.git external/freetype
  #
  git submodule add -b branch-1.3 https://github.com/fltk/fltk.git external/fltk
  #
  DIR=external/gmsh
  git submodule add -b master https://gitlab.onelab.info/gmsh/gmsh.git $DIR
  #cd $DIR && git checkout gmsh_4_10_5; cd ..; git add $DIR
  #
  git submodule add -b release https://gitlab.com/petsc/petsc.git external/petsc
  #
  git submodule update --init --recursive
  #
  git commit -m ""
fi
