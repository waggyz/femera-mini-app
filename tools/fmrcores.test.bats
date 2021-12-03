#!/usr/bin/env bats
#
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
@test "tools/fmrcores.sh does exist" {
  assert_file_exist tools/fmrcores.sh
}
@test "tools/fmrcores.sh output is an integer > 0" {
  run tools/fmrcores.sh
  assert_output --regexp '^[0-9]+$'
  assert [ "$output" -gt "0" ]
}
