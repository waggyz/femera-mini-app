# Valgrind Target Repair Plan

Date: 2026-10-23

Status: implemented and validated on the local MPICH stack. The illustrative
fragments below preserve the original proposal; the Makefile is authoritative
for the implemented targets and configuration tracking.

## Implementation Results

- Corrected MPI checker ordering in the Makefile and `tools/fmrexec.sh`.
- Split helper compilation, suppression generation and application checking;
  added content-compared configuration tracking and success-only aggregate logs.
- Enabled and repaired the helper's MPI branch, including MPI error handling.
- Repaired optional MPI suppression discovery and added executable test fixtures.
- All 23 focused Bats tests pass: 10 wrapper tests and 13 target/discovery tests.
- Shell syntax checks and editor diagnostics pass for the touched implementation.
- The helper compiled and ran in both non-MPI and two-rank MPI modes.
- The real MPICH application check completed with two ranks and two OpenMP
  threads per rank. Both rank logs reported zero errors and zero definitely
  lost bytes, and identified the native `mini` executable rather than Bash.
- `make mini ENABLE_VALGRIND=ON` reached `build-done`, including a successful
  Valgrind check after rebuilding the application. Its other test stages
  reported an elastic-material AVX2 speed assertion failure
  (`PerfMtrlElasticLinearIso.AVX2IsCorrectAndFaster`) and a pymera test failure.
  Those failures are outside this change and were not repaired.
- OpenMPI and non-MPI Make target branches were exercised with isolated fake
  tools. Full application runtime validation used MPICH only; no clean was run.

The implementation does not install a new global `fmrexec` copy. Makefile
Valgrind targets use explicit launch commands and therefore do not depend on
the version installed in the user's PATH. Separate simultaneous checks in the
same CPU build directory remain unsupported, as noted in the original plan.

## Diagnosis

The existing `build/Ultra-7-255H/mini.valgrind.log` identifies the command as
the installed `fmrexec` script and reports SIGTRAP in Bash's `bind_variable`,
before the application starts. This establishes that the wrong process is
being instrumented; it does not establish the underlying Bash/Valgrind bug.

The required MPI command structure is:

```text
mpiexec [launcher options] valgrind [checker options] native-executable [application options]
```

Neither `valgrind fmrexec ...` nor `valgrind mpiexec ...` reliably checks each
application rank. Both the Makefile's suppression commands and the existing
checker mode in `tools/fmrexec.sh` need the same ordering correction.

Additional defects in the current targets:

- `mini.valgrind.log` depends on the suppression executable, not on `mini` or
  the generated suppression data. A rebuild of `mini` may not rerun checking.
- Compilation and suppression generation share the helper executable's recipe,
  while suppression data is a prerequisite of that executable. Deleting the
  data alone does not reliably regenerate it.
- Multiple ranks must not write the same Valgrind log or interleave raw
  suppression blocks on a shared stream.
- OpenMPI's trailing `sed` commands can hide a failed launch's exit status.
- A failed check can leave a log that Make subsequently considers up to date.
- Disabled checking currently creates a fake log, which can survive enabling
  checking later.
- The explicit suppression-data rule uses `$(*)` instead of `$@` and appends
  seeds rather than rebuilding the output.
- The helper compile omits `FMR_HAS_MPI` and MPI include flags, disabling its
  intended MPI work. Enabling that branch exposes invalid MPI calls.
- `tools/valgrind-mpi-supp` has malformed path construction and quoted wildcard
  tests. No installed MPI suppression file must be a supported outcome.

## Implementation Sequence

1. Repair and compile the suppression helper, including its actual MPI branch.
2. Define the launcher once, with separate MPICH, OpenMPI and non-MPI cases.
3. Split helper compilation, suppression generation and application checking
   into targets with accurate prerequisites.
4. Generate per-process raw logs, merge seeds and generated suppressions, and
   publish outputs only after successful commands.
5. Correct `fmrexec`'s existing checker mode without redesigning its parser.
6. Add focused regression tests, then run the real MPICH check available on
   this machine. Do not clean the entire build just to test recipe ordering.

Preserve existing warning-only treatment of reported memory errors. A crash,
failed launcher, failed checker startup or failed merge must still fail Make.
Do not add `--error-exitcode` during intentional suppression discovery.

## Illustrative Makefile Corrections

These fragments show the proposed replacement structure, not an applied patch.
Remove the old `VGMPI`, `VGEXEC`, `VGSUPP`, helper and suppression-data recipes
when integrating them; do not append duplicate recipes. Keep all enabled-only
definitions and targets inside `ifeq ($(ENABLE_VALGRIND),ON)`.

### Launcher and Commands

```makefile
ifeq ($(ENABLE_VALGRIND),ON)
VALGRIND_SUPP := valgrind.supp
VALGRIND_SUPP_EXE := $(BUILD_CPU)/$(VALGRIND_SUPP).exe
VALGRIND_SUPP_PATH := $(BUILD_CPU)/$(VALGRIND_SUPP)
VGFLAGS := $(CXXFLAGS) $(filter-out -Winline,$(CXXWARNS))

VG_LAUNCH :=
ifeq ($(ENABLE_MPI),ON)
VG_LAUNCH := mpiexec -n $(TDD_MPI_N)
ifeq ($(FMR_MPI_TYPE),OPENMPI)
VG_LAUNCH += --bind-to core -map-by node:pe=$(TDD_OMP_NP)
endif
endif

VGSUPP_FLAGS += --suppressions=$(VALGRIND_SUPP_PATH)
VG_ENV := env OMP_NUM_THREADS=$(TDD_OMP_NP)
VG_SUPP_COMMAND = $(VG_ENV) $(VG_LAUNCH) valgrind \
  --leak-check=full --show-reachable=yes --error-limit=no \
  --show-leak-kinds=all --gen-suppressions=all \
  --log-file=$(BUILD_CPU)/valgrind.supp.raw.%p.log \
  $(VALGRIND_SUPP_EXE)

VG_CHECK_COMMAND = $(VG_ENV) $(VG_LAUNCH) valgrind \
  --leak-check=full --track-origins=yes $(VGSUPP_FLAGS) \
  --log-file=$(BUILD_CPU)/mini.valgrind.%p.log \
  $(BUILD_CPU)/mini -fmr:T -fmr:o$(TDD_OMP_NP) -v0 $(TDD_FMRFILE)
endif
```

An empty `VG_LAUNCH` directly checks the native executable in non-MPI builds.
Keep Make's existing library-path and affinity exports. Here the OpenMPI
processing-element count matches the actual application thread count;
`TDD_MPI_N` controls ranks, while `TDD_OMP_NP` controls threads per rank.
The existing `fmrexec tdd` adds `-fmr:T`; preserve that behavior explicitly.

Use a matching MPI compiler and launcher from the same installation. Never
pass OpenMPI-specific binding options to MPICH.

### Compile the Helper Independently

Place these targets after `TOPDEPS` has been defined, inside the enabled branch:

```makefile
$(VALGRIND_SUPP_EXE): export TMPDIR := $(abspath $(BUILD_CPU)/tmp)
$(VALGRIND_SUPP_EXE): src/valgrind.supp.cpp $(TOPDEPS)
$(VALGRIND_SUPP_EXE): | $(BUILD_CPU)/ $(BUILD_CPU)/tmp/
	$(CXX) $(VGFLAGS) $(filter -DFMR_HAS_MPI,$(FMRFLAGS)) \
	  $(INCFLAGS) $< $(LDFLAGS) -o $@ $(LDLIBS)
```

Compilation should only create the executable. It must not depend on or
generate the suppression file. The filtered define avoids introducing
unrelated application macros into this standalone helper.

### Generate Suppression Data Independently

Treat `VGMPISUPP` and `VALGRIND_SUPP_FILE` as optional seed file paths. A missing
explicitly configured seed must fail visibly; no automatically discovered seed
is normal. The following assumes the repository's existing no-spaces-in-paths
convention for these Make prerequisite lists.

```makefile
$(VALGRIND_SUPP_PATH): $(VALGRIND_SUPP_EXE) $(TOPDEPS)
$(VALGRIND_SUPP_PATH): tools/grindmerge tools/valgrind-mpi-supp
$(VALGRIND_SUPP_PATH): $(VGMPISUPP) $(VALGRIND_SUPP_FILE)
  set -euo pipefail; \
	  rm -f "$@" "$@.tmp" $(BUILD_CPU)/valgrind.supp.raw.*.log; \
	  trap 'rm -f "$@.tmp"' EXIT; \
	  $(VG_SUPP_COMMAND) > "$(BUILD_CPU)/valgrind.supp.out"; \
	  logs=( $(BUILD_CPU)/valgrind.supp.raw.*.log ); \
	  test -f "$${logs[0]}"; \
	  { \
	    for seed in $(VGMPISUPP) $(VALGRIND_SUPP_FILE); do \
	      cat "$$seed" || exit; \
	    done; \
	    cat "$${logs[@]}"; \
	  } | tools/grindmerge > "$@.tmp"; \
	  mv "$@.tmp" "$@"
```

Bash `pipefail` ensures a failed seed/log read cannot be hidden by a successful merger.
`tools/grindmerge` already extracts suppression blocks from raw Valgrind logs
and deduplicates them, so streaming seeds alongside raw logs avoids its
single-`-f` limitation. Do not call `grindmerge -f` with an empty filename.
An empty merged suppression file is valid when no suppressions are needed.

For a durable implementation, also track checker configuration in a small
content-compared flags file: enabled/MPI mode, launcher, rank/thread counts,
compiler/helper flags, seed paths, checker flags and application arguments.
Make the generated suppression data and check log depend on it. This catches
command-line overrides without forcing every unchanged check to rerun. Use
the existing `tools/update-file-if-diff` pattern rather than touching a flags
file on every invocation. Refresh it even for the directly requested target,
not only through `intro`.

### Check the Application and Publish a Success Log

```makefile
$(BUILD_CPU)/mini.valgrind.log: $(BUILD_CPU)/mini $(VALGRIND_SUPP_PATH)
$(BUILD_CPU)/mini.valgrind.log: $(TOPDEPS)
	$(info $(GRND) checking $(BUILD_CPU)/mini under Valgrind)
	set -eu; \
	  rm -f "$@" "$@.tmp" $(BUILD_CPU)/mini.valgrind.*.log; \
	  trap 'rm -f "$@.tmp"' EXIT; \
	  $(VG_CHECK_COMMAND) > "$(BUILD_CPU)/mini.valgrind.out"; \
	  logs=( $(BUILD_CPU)/mini.valgrind.*.log ); \
	  test -f "$${logs[0]}"; \
	  cat "$${logs[@]}" > "$@.tmp"; \
	  mv "$@.tmp" "$@"
	$(info $(MORE) $(BUILD_CPU)/mini.valgrind.log)
```

Keep any warning-summary extraction in subsequent commands, after the checked
launch. Do not delete diagnostic lines using `sed` or overwrite the launch
status with post-processing. Raw per-process logs and application output stay
available if the application fails; the aggregate target does not.

In the disabled branch, make this specific target phony and print the enable
instruction without writing a log. That also ensures an old log cannot hide
the disabled status. The enabled branch should keep it a normal file target.

```makefile
else # ENABLE_VALGRIND
.PHONY: $(BUILD_CPU)/mini.valgrind.log
$(BUILD_CPU)/mini.valgrind.log:
	@echo "Set ENABLE_VALGRIND=ON to check for memory leaks."
endif
```

The application's existing serial recursive Make call can remain. These
fragments assume one check invocation per CPU build directory at a time;
separate simultaneous Make invocations would need a lock or unique run
directories before deleting or aggregating shared log patterns.

## Suppression Helper Corrections

In `src/valgrind.supp.cpp`, the MPI branch currently contains invalid API calls.
The minimal corrections would look like:

```diff
-    if(tot<0.0){ MPI_Abort (); }
-    MPI_Free (& comm );
+    if(tot<0.0){ MPI_Abort (comm, 1); }
+    MPI_Comm_free (& comm );
     MPI_Finalize ();
-    if( provided != MPI_THREAD_SERIALIZED ){ return 1; }
+    if( provided < MPI_THREAD_SERIALIZED ){ return 1; }
```

More capable thread support is acceptable. Check MPI initialization and
communication return codes during implementation rather than assuming a
successful compile establishes a valid suppression run.

## fmrexec Correction

The existing script already recognizes `grind`, `valgrind`, `memcheck` and
`vg:` arguments. Its current `RUN` construction puts Valgrind before the MPI
launcher. The narrow command-order correction would look like:

```diff
-RUN=$VGEXE"$VGARGS"mpiexec$MPIARGS" "$FMREXE$FMRARGS
+RUN="mpiexec$MPIARGS $VGEXE$VGARGS$FMREXE$FMRARGS"
```

This relies on the script's existing block that appends a separator to
`VGARGS` when `VGEXE` is nonempty. With no checker selected it still launches
the application directly through MPI. Preserve the existing child status,
echo mode, library paths and thread environment.

Example expected echo output, with environment details omitted:

```text
mpiexec --bind-to core -n 2 valgrind --leak-check=full /path/to/mini -fmr:o2 -fmr:T
```

That example reflects the wrapper's current default launcher options, not a
portable MPICH binding prescription. The Makefile should use the explicit
MPI-specific launcher above rather than depend on the installed wrapper.
The existing string-based wrapper still does not preserve arbitrary quoted
argument boundaries; do not claim this one-line correction fixes that separate
parser limitation. An array-based parser would be a separate scoped change.

## Optional MPI Seed Discovery

Repair `tools/valgrind-mpi-supp` to resolve the selected `mpicc` location and
search plausible installation share directories with real pathname expansion,
not a quoted `*`. Return one readable suppression path or no output. Resolve
symlinks and account for distribution layouts during implementation; do not
assume every MPI installation ships a seed. An explicit `VGMPISUPP=...`
override should remain usable. The existing discovery helper must not silently
select suppressions from a different MPI installation.

## Validation Plan

The focused checks and real MPICH workflow below have now been exercised as
summarized above. Alternate-stack runtime checks remain conditional on suitable
installed compilers, launchers and libraries.

1. Run shell syntax checks and the existing Bats suite after implementation:

   ```bash
   bash -n tools/fmrexec.sh
   bash -n tools/valgrind-mpi-supp
   external/bats-core/bin/bats tools/fmrexec.test.bats
   ```

2. Extend `tools/fmrexec.test.bats` to assert ordering for checker aliases and
   `vg:` options, unchanged plain `tdd` behavior, executable selection and
   propagation of a failed child status. Use fake tools to test actual argv,
   not only echo output.
3. Add focused Makefile tests using disposable outputs and fake tools. Cover
   MPICH/OpenMPI ordering, direct non-MPI checking, absent and missing seeds,
   failed launch/merge, independent rank logs, deletion of suppression data,
   rebuilt application inputs, changed flags, unchanged-input reuse and
   failed-run retries. Reuse existing Bats helpers.
4. Compile the MPI and non-MPI helper branches; run the MPI helper with two
   ranks using the locally selected MPI stack before generating suppressions.
5. Run the focused real target:

   ```bash
   make ENABLE_VALGRIND=ON build/Ultra-7-255H/mini.valgrind.log
   ```

   Each process log's `Command:` must name the native `mini`, not Bash,
   `fmrexec` or `mpiexec`. With defaults, expect two rank logs and one aggregate
   after success. Check suppression-helper logs similarly.
6. Run `make mini ENABLE_VALGRIND=ON` once focused checks pass. Only then
   reproduce the original clean-build workflow if removing existing build
   outputs is acceptable. Do not permanently edit `config.local` for testing.
7. Test alternate MPI/non-MPI command branches with stubs. Claim real OpenMPI
   or non-MPI runtime validation only when matching builds and libraries have
   actually been exercised. Never mix MPI distributions.

## Out of Scope

Diagnosing Bash's internal SIGTRAP, hiding exit 133, using `--trace-children`
as a substitute for correct process placement, broadly rewriting the wrapper,
changing dependency installations, or cleaning existing core dumps.