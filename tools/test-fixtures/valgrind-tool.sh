#!/bin/bash
set -eu

case "$(basename "$0")" in
  mpic++|g++)
    if [[ "${1:-}" == -dumpversion ]]; then
      printf 'test-compiler\n'
      exit 0
    fi
    while [[ $# -gt 0 ]]; do
      if [[ "$1" == -o ]]; then
        cp "$0" "$2"
        chmod +x "$2"
        exit 0
      fi
      shift
    done
    exit 90
    ;;
  mpiexec)
    ranks=1
    while [[ $# -gt 0 ]]; do
      case "$1" in
        -n) ranks=$2; shift 2 ;;
        --bind-to|-map-by) shift 2 ;;
        *) break ;;
      esac
    done
    [[ "${1:-}" == valgrind ]] || exit 91
    for ((rank=0; rank<ranks; rank++)); do
      "$@" || exit $?
    done
    ;;
  valgrind)
    log=
    generate=
    while [[ "${1:-}" == --* ]]; do
      case "$1" in
        --log-file=*) log=${1#--log-file=} ;;
        --gen-suppressions=all) generate=yes ;;
      esac
      shift
    done
    if [[ -n "$log" ]]; then
      log=${log//%p/$$}
      printf 'Command: %s\n' "$*" > "$log"
      if [[ -n "$generate" ]]; then
        printf '{\nfixture\nMemcheck:Leak\nfun:fixture\n}\n' >> "$log"
        cat >> "$log" <<'EOF'
{
fixture-mpi-init
Memcheck:Leak
match-leak-kinds: definite
fun:malloc
fun:ompi_mpi_init
fun:PMPI_Init_thread
fun:main
}
{
fixture-mpi-finalize
Memcheck:Leak
match-leak-kinds: definite
fun:calloc
fun:ompi_mpi_finalize
fun:main
}
{
fixture-non-mpi
Memcheck:Leak
fun:malloc
fun:baseline_helper
fun:main
}
EOF
      fi
    fi
    if [[ "${MOCK_CHECKER_STATUS:-0}" != 0 ]]; then
      exit "$MOCK_CHECKER_STATUS"
    fi
    [[ "${MOCK_SKIP_LOG:-0}" != 1 || -z "$log" ]] || rm -f "$log"
    printf 'checked rank\n'
    ;;
  fmrcores) printf '4\n' ;;
  fmrnumas) printf '2\n' ;;
  fmrmodel) printf 'fixture-cpu\n' ;;
  *) exit 0 ;;
esac