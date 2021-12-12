#!/usr/bin/env bats
#
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
@test "fmrmodel, fmrcores, fmrnumas, fmrexec are absent from $DIR/" {
  ! assert_file_exist "$DIR/fmrmodel"
  ! assert_file_exist "$DIR/fmrcores"
  ! assert_file_exist "$DIR/fmrnumas"
  ! assert_file_exist "$DIR/fmrexec"
}
