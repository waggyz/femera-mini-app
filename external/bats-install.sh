#!/bin/bash
# NOTE Run from the root of the Femera repository [femera-mini-app/].
# usage: external/bats-install.sh "<INSTALL_DIR>"
#
cd external/bats-core
./install.sh "$1"
#
