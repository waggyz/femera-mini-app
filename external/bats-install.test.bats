#!/usr/bin/env bats
#
INSTALL_DIR="$1"
if [ -z "$INSTALL_DIR" ]; then
  INSTALL_DIR=build/stage
fi
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
@test "Bats was cloned" { # Bats tests equivalent to install-bats.pre.test.bats
  assert_file_exist external/bats-core/README*
  assert_file_exist external/bats-assert/README*
  assert_file_exist external/bats-support/README*
  assert_file_exist external/bats-file/README*
}
@test "bats is in system path" {
  which bats
  #
  # run bats -v # Do not check Bats version here.
  #assert_output "Bats 1.5.0"
  # [[ "$output" == "Bats"* ]]
}
