#!/usr/bin/env bats
#
setup() {
  load '../external/bats-support/load.bash'
  load '../external/bats-assert/load.bash'
  load '../external/bats-file/load.bash'
  mkdir "$BATS_TEST_TMPDIR/bin"
  for tool in env basename; do
    ln -s "$(command -v "$tool")" "$BATS_TEST_TMPDIR/bin/$tool"
  done
  for tool in fmrcores fmrnumas fmrmodel valgrind; do
    install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/bin/$tool"
  done
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

@test "checker aliases place valgrind after mpiexec and before the application" {
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/mpiexec"
  export PATH="$BATS_TEST_TMPDIR:$PATH"
  for mode in grind valgrind memcheck; do
    run tools/fmrexec.sh echo "$mode" --leak-check=full tdd /usr/bin/true
    assert_success
    [[ "$output" == *"mpiexec "*" valgrind --leak-check=full /usr/bin/true "* ]]
  done
}

@test "vg options enable the checker without a mode alias" {
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/mpiexec"
  export PATH="$BATS_TEST_TMPDIR:$PATH"
  run tools/fmrexec.sh echo vg:--track-origins=yes tdd /usr/bin/true
  assert_success
  [[ "$output" == *"mpiexec "*" valgrind --track-origins=yes /usr/bin/true "* ]]
}

@test "plain tdd does not enable valgrind" {
  run tools/fmrexec.sh echo tdd /usr/bin/true
  assert_success
  [[ "$output" != *"valgrind"* ]]
}

@test "launcher executes the checker once per rank and preserves application arguments" {
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/mpiexec"
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/valgrind"
  export PATH="$BATS_TEST_TMPDIR:$PATH"
  run tools/fmrexec.sh grind --leak-check=full tdd /usr/bin/true
  assert_success
  [[ "$output" == *" valgrind --leak-check=full /usr/bin/true -fmr:o2 -fmr:T"* ]]
  [[ "$output" == *$'checked rank\nchecked rank' ]]
}

@test "checker failure propagates through the launcher and wrapper" {
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/mpiexec"
  install -m 755 tools/test-fixtures/valgrind-tool.sh "$BATS_TEST_TMPDIR/valgrind"
  export PATH="$BATS_TEST_TMPDIR:$PATH"
  export MOCK_CHECKER_STATUS=37
  run tools/fmrexec.sh grind tdd /usr/bin/true
  assert_failure 37
}

@test "echo without MPI omits the launcher and MPI-only arguments" {
  run env PATH="$BATS_TEST_TMPDIR/bin" /bin/bash tools/fmrexec.sh echo mpi:--bind-to core tdd /usr/bin/true
  assert_success
  [[ "$output" == *" /usr/bin/true -fmr:o2 -fmr:T" ]]
  [[ "$output" != *"mpiexec"* ]]
  [[ "$output" != *"--bind-to"* ]]
  [[ "$output" != *" -n "* ]]
}

@test "without MPI the application runs directly with its arguments" {
  run env PATH="$BATS_TEST_TMPDIR/bin" /bin/bash tools/fmrexec.sh tdd /bin/echo application-argument
  assert_success
  [[ "${lines[1]}" == "-fmr:o2 -fmr:T application-argument" ]]
}

@test "without MPI application failure propagates through the wrapper" {
  run env PATH="$BATS_TEST_TMPDIR/bin" /bin/bash tools/fmrexec.sh tdd /bin/false
  assert_failure 1
}

@test "without MPI the checker runs once with application arguments" {
  run env PATH="$BATS_TEST_TMPDIR/bin" /bin/bash tools/fmrexec.sh grind --leak-check=full tdd /usr/bin/true
  assert_success
  [[ "$output" == *" valgrind --leak-check=full /usr/bin/true -fmr:o2 -fmr:T"* ]]
  [[ "${#lines[@]}" -eq 2 ]]
  [[ "${lines[1]}" == "checked rank" ]]
}

@test "without MPI checker failure propagates through the wrapper" {
  export MOCK_CHECKER_STATUS=37
  run env PATH="$BATS_TEST_TMPDIR/bin" /bin/bash tools/fmrexec.sh grind tdd /usr/bin/true
  assert_failure 37
}
