#!/usr/bin/env bats
#
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
@test "tools/fmrexec.sh does exist" {
  assert_file_exist tools/fmrexec.sh
}
@test "'tools/fmrexec.sh echo' output does contain 'femera'" {
  run tools/fmrexec.sh echo
  [[ "$output" == *"femera"* ]]
}
@test "'tools/fmrexec.sh echo tdd' contains '-fmr:o2'" {
  run tools/fmrexec.sh echo tdd
  [[ "$output" == *" -fmr:o2"* ]]
}
@test "'tools/fmrexec.sh echo true' contains the executable 'true'" {
  run tools/fmrexec.sh echo `which true`
  [[ "$output" == *"true"* ]]
}
@test "'tools/fmrexec.sh echo true' does not contain 'femera'" {
  run tools/fmrexec.sh echo `which true`
  [[ "$output" != *"femera"* ]]
}
