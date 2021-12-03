#!/usr/bin/env bats
#
@test "fmrmodel, fmrcores, fmrnumas, fmrexec are installed" {
  which fmrmodel
  which fmrcores
  which fmrnumas
  which fmrexec
}
