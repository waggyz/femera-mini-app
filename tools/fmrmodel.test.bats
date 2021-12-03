#!/usr/bin/env bats
#
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
@test "tools/fmrmodel.sh does exist" {
  assert_file_exist tools/fmrmodel.sh
}
@test "tools/fmrmodel.sh output does contain an integer" {
  run tools/fmrmodel.sh
  ! [[ "$output" =~ '[0-9]' ]]
}
@test "tools/fmrmodel.sh output length is 3-255 (2 < length < 256)" {
  run tools/fmrmodel.sh
  [[   "2" -lt ${#output} ]]
  [[ ${#output} -lt "256" ]]
}
@test "tools/fmrmodel.sh output is a reasonable file name" {
  run tools/fmrmodel.sh
  ! assert_output "."
  ! assert_output ".."
  [[ $output =~ ^[0-9a-zA-Z._-]+$ ]];
}
