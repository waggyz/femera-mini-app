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
