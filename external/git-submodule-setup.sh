#!/bin/bash
#
#NOTE Remove modules that need to be changed, e.g., to a different version
#     before running this.
#
touch .gitmodules
#
# git submodule add -b $REPO_BRANCH $REPO_URL $DEST_FOLDER
git submodule add https://github.com/bats-core/bats-core.git external/bats-core
git submodule add https://github.com/bats-core/bats-support.git external/bats-support
git submodule add https://github.com/bats-core/bats-assert.git external/bats-assert
git submodule add https://github.com/bats-core/bats-file.git external/bats-file
git submodule add https://github.com/pybind/pybind11.git external/pybind11
#TODO Check if pybind11 should be v2.5.0
git submodule add -b master https://github.com/CGNS/CGNS.git external/cgns
git submodule add -b master https://gitlab.freedesktop.org/freetype/freetype.git external/freetype
git submodule add -b branch-1.3 https://github.com/fltk/fltk.git external/fltk
git submodule add https://gitlab.onelab.info/gmsh/gmsh.git external/gmsh
git submodule add -b release https://gitlab.com/petsc/petsc.git external/petsc
#
git submodule update --init --recursive
#
