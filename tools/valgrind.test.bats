#!/usr/bin/env bats

setup() {
  load '../external/bats-support/load.bash'
  load '../external/bats-assert/load.bash'
  load '../external/bats-file/load.bash'
  cd "$BATS_TEST_DIRNAME/.."
  fixture="$PWD/tools/test-fixtures/valgrind-tool.sh"
  work="$BATS_TEST_TMPDIR/build"
  bin="$BATS_TEST_TMPDIR/bin"
  mkdir -p "$work" "$bin"
  for tool in mpic++ mpiexec valgrind fmrmodel fmrcores fmrnumas; do
    install -m 755 "$fixture" "$bin/$tool"
  done
  install -m 755 "$fixture" "$work/mini"
  export PATH="$bin:$PATH"
  make_check=(make --no-print-directory ENABLE_VALGRIND=ON ENABLE_MPI=ON
    FMR_MPI_TYPE=MPICH "BUILD_CPU=$work" "CXX=$bin/mpic++"
    ADD_TO_PATH= ADD_TO_LDPATH= ADD_TO_LDRUN= VGMPISUPP= VALGRIND_SUPP_FILE=
    LDFLAGS= INCFLAGS= "LIBFEMERA_A=$work/libfemera.a" -o "$work/libfemera.a")
  target="$work/mini.valgrind.log"
}

@test "MPICH checks each native rank and merges generated suppressions" {
  run "${make_check[@]}" "$target"
  assert_success
  logs=( "$work"/mini.valgrind.*.log )
  [[ ${#logs[@]} == 2 ]]
  [[ $(grep -c '^Command:' "$target") == 2 ]]
  grep -q "Command: $work/mini -fmr:T -fmr:o2 -v0" "$target"
  grep -q 'Memcheck:Leak' "$work/valgrind.supp"
  [[ $(grep -c '^fixture$' "$work/valgrind.supp") == 1 ]]
}

@test "OpenMPI places binding options before the per-rank checker" {
  run "${make_check[@]}" FMR_MPI_TYPE=OPENMPI -n "$target"
  assert_success
  [[ "$output" == *"mpiexec -n 2 --bind-to core -map-by node:pe=2 valgrind "* ]]
  run "${make_check[@]}" FMR_MPI_TYPE=OPENMPI "$target"
  assert_success
  assert_file_exist "$target"
}

@test "non-MPI checks the application directly with one process log" {
  run "${make_check[@]}" ENABLE_MPI=OFF -n "$target"
  assert_success
  [[ "$output" != *"mpiexec -n"* ]]
  run "${make_check[@]}" ENABLE_MPI=OFF "$target"
  assert_success
  logs=( "$work"/mini.valgrind.*.log )
  [[ ${#logs[@]} == 1 ]]
}

@test "unchanged inputs reuse logs but a changed thread count reruns checking" {
  run "${make_check[@]}" "$target"
  assert_success
  original=$(stat -c %y "$target")
  run "${make_check[@]}" "$target"
  assert_success
  [[ $(stat -c %y "$target") == "$original" ]]
  run "${make_check[@]}" TDD_OMP_NP=3 "$target"
  assert_success
  grep -q -- '-fmr:o3' "$target"
}

@test "rebuilt mini and deleted suppression data invalidate checking" {
  run "${make_check[@]}" "$target"
  assert_success
  original=$(stat -c %y "$target")
  touch "$work/mini"
  run "${make_check[@]}" "$target"
  assert_success
  [[ $(stat -c %y "$target") != "$original" ]]
  rm "$work/valgrind.supp"
  run "${make_check[@]}" "$target"
  assert_success
  [[ "$output" == *"suppression file:"* ]]
  assert_file_exist "$work/valgrind.supp"
}

@test "checker failure retains rank logs but no success target and can be retried" {
  run "${make_check[@]}" "$work/valgrind.supp"
  assert_success
  export MOCK_CHECKER_STATUS=37
  run "${make_check[@]}" "$target"
  assert_failure
  assert_file_not_exist "$target"
  logs=( "$work"/mini.valgrind.*.log )
  assert_file_exist "${logs[0]}"
  unset MOCK_CHECKER_STATUS
  run "${make_check[@]}" "$target"
  assert_success
  assert_file_exist "$target"
}

@test "suppression helper failure cannot publish suppression data" {
  export MOCK_CHECKER_STATUS=37
  run "${make_check[@]}" "$target"
  assert_failure
  assert_file_not_exist "$work/valgrind.supp"
  assert_file_not_exist "$target"
  unset MOCK_CHECKER_STATUS
  run "${make_check[@]}" "$target"
  assert_success
}

@test "successful checker without logs cannot publish success" {
  run "${make_check[@]}" "$work/valgrind.supp"
  assert_success
  export MOCK_SKIP_LOG=1
  run "${make_check[@]}" "$target"
  assert_failure
  assert_file_not_exist "$target"
}

@test "explicit MPI and user seeds merge without appending duplicates" {
  run "${make_check[@]}" "$work/valgrind.supp"
  assert_success
  cp "$work/valgrind.supp" "$BATS_TEST_TMPDIR/mpi.supp"
  cp "$work/valgrind.supp" "$BATS_TEST_TMPDIR/user.supp"
  run "${make_check[@]}" "VGMPISUPP=$BATS_TEST_TMPDIR/mpi.supp" \
    "VALGRIND_SUPP_FILE=$BATS_TEST_TMPDIR/user.supp" "$target"
  assert_success
  [[ $(grep -c '^fixture$' "$work/valgrind.supp") == 1 ]]
}

@test "missing explicit seed fails instead of silently dropping it" {
  run "${make_check[@]}" "VALGRIND_SUPP_FILE=$BATS_TEST_TMPDIR/missing.supp" "$target"
  assert_failure
  assert_file_not_exist "$target"
}

@test "seed read failure propagates through the merge pipeline" {
  mkdir "$BATS_TEST_TMPDIR/directory.supp"
  run "${make_check[@]}" "VALGRIND_SUPP_FILE=$BATS_TEST_TMPDIR/directory.supp" "$target"
  assert_failure
  assert_file_not_exist "$work/valgrind.supp"
  assert_file_not_exist "$work/valgrind.supp.tmp"
}

@test "disabled checking creates no fake log and enabling it performs a check" {
  run "${make_check[@]}" ENABLE_VALGRIND=OFF "$target"
  assert_success
  assert_file_not_exist "$target"
  run "${make_check[@]}" "$target"
  assert_success
  assert_file_exist "$target"
  run "${make_check[@]}" ENABLE_VALGRIND=OFF "$target"
  assert_success
  [[ "$output" == *"Set ENABLE_VALGRIND=ON"* ]]
}

@test "MPI seed discovery handles missing compilers and installation share paths" {
  run env MPICC="$BATS_TEST_TMPDIR/missing-mpicc" tools/valgrind-mpi-supp
  assert_success
  assert_output ''
  install -m 755 "$fixture" "$bin/mpicc"
  run env MPICC="$bin/mpicc" tools/valgrind-mpi-supp
  assert_success
  assert_output ''
  mkdir -p "$BATS_TEST_TMPDIR/share/mpich"
  cp "$fixture" "$BATS_TEST_TMPDIR/share/mpich/mpi.supp"
  run env MPICC="$bin/mpicc" tools/valgrind-mpi-supp
  assert_success
  assert_output "$BATS_TEST_TMPDIR/share/mpich/mpi.supp"
}