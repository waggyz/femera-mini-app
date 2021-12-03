#!/bin/bash
# Roll back to Gmsh 4.7.1 and apply a patch for g++ 4.8.
#
# First, get the directory of this script.
DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
#
#TODO Check if gcc < 4.9.
#
#TODO Check if the patch has been applied already.
#
if [ -d "$DIR/gmsh" ]; then
  cd "$DIR/gmsh"
  git checkout tags/gmsh_4_7_1     # Revert to Gmsh 4.7.1.
  git apply ../gmsh471-gcc48.patch # Apply the patch for gcc 4.8.
else
  echo "Could not find the $DIR/gmsh directory. Have you cloned Gmsh yet?"
fi
#TODO Check that the diff matches the patch.
# Use the following to regenerate the Gmsh 4.7.1 patch for g++ 4.8.
# git diff -b > ../gmsh471-gcc48.patch
#
