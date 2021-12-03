#!/usr/bin/env bats
#
setup() {
  load 'external/bats-support/load.bash'
  load 'external/bats-assert/load.bash'
  load 'external/bats-file/load.bash'
}
FMRNUMAS="build/"`tools/fmrmodel.sh`"/tools/fmrnumas"
if [ ! -f $FMRNUMAS ]; then
  FMRNUMAS="tools/fmrnumas.sh"
fi
  @test "$FMRNUMAS does exist" {
    assert_file_exist $FMRNUMAS
  }
  get_fmrnumas_exe_stdout() {
    "$FMRNUMAS" 2>>build/`tools/fmrmodel.sh`/tools/fmrnumas.test.err
    return
  }
  @test "$FMRNUMAS stdout is an integer > 0" {
    run get_fmrnumas_exe_stdout
    assert_output --regexp '^[0-9]+$'
    assert [ "$output" -gt "0" ]
  }
