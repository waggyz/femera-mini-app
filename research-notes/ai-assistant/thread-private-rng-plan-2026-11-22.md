# Thread-private test-bed random generators: implementation plan

Date: 2026-11-22

Status: implemented and validated on the local MPICH/OpenMP build.

## Implementation results

- `Beds::get_rand()` supplies per-instance, per-thread `std::mt19937` sequences,
  with `get_rand_max()` defining the inclusive upper bound and `get_rand_seed()`
  implementing the specified modulo arithmetic without overflow.
- Initialization seeds the actual OpenMP team before child tests initialize.
  Each slot records its operating-system thread owner and processing ID.
  Nested regions, larger teams, foreign/replacement workers and uninitialized
  access fail explicitly. RNG readiness remains true during child-test exit,
  because `Work::exit_list` clears `did_init()` before those tests run.
- The Bars call site uses the Beds draw and its range. Defaults and unrelated
  benchmark parameters remain unchanged. The test README documents the API,
  supported topology and lifecycle.
- All nine focused Beds tests pass with one MPI rank and one or four OpenMP
  threads, and with two MPI ranks and two threads per rank. XML results for the
  single-rank runs report nine completed tests and zero failures/errors.
  Coverage includes exact reference sequences, overflow boundaries, separate
  instances, reset, serial/smaller-team/parallel continuity, independent draw
  counts, lifetime through child exit, invalid access and mesh perturbations.
  The complete Beds executable also passes all 10 tests on both MPI ranks.
- The changed RNG source compiles without OpenMP. Compile-time checks also
  pass with 64-bit `Local_int` and `FMR_RAND_SEED=4294967295u`, including an
  addition that would overflow without modular reduction. A full non-OpenMP
  application build was not validated; the existing Jobs constructors use
  OpenMP functions unconditionally.
- The default Bars benchmark builds and passes its displacement check
  (10/10), with no bad Jacobians. A scratch unstructured variant exercises the
  actual replacement call: no bad Jacobians and solver convergence. Its final
  displacement check reports 2/10 because the existing checker uses uniform
  node-index coordinates. A scratch variant using the old `rand()` expression
  has the same 2/10 failure. This pre-existing checker limitation is unchanged.
- Affected archive members were rebuilt to remove stale-layout ODR warnings
  before final validation. Existing Bars inline warnings remain. No race
  detector run or full application test suite was performed.

The original implementation and validation plan follows.

## Goal and current behavior

Expose reproducible, thread-private pseudorandom sequences through
`femera::test::Beds`, with generators initialized during test-bed initialization.
Seed each thread from the existing global base seed and the processing
hierarchy's global thread ID:

```text
seed = (fmr::rand_seed + proc->get_proc_id()) % max_unsigned_int
max_unsigned_int = std::numeric_limits<unsigned int>::max()
```

This is modulo `max()`, not modulo `max() + 1`; the maximum unsigned-int value
itself is not a seed under this formula. Perform the mathematical addition and
modulo without unsigned overflow changing the result. The seed can wrap and
therefore distinct IDs are not necessarily guaranteed distinct seeds.

Currently `src/femera/test/Beds.hpp` exposes no RNG, and
`src/femera/test/Beds.cpp` initializes its child test tasks in `task_init`.
`src/femera/task/Jobs.cpp` registers `proc::Main` before `test::Beds`;
`Work::init_list` initializes children in order, and `Proc::init` calculates
the processing ID bases after initializing its process tree. Thus Beds can
initialize its generators after the process hierarchy is ready. `Beds` is
shared through `Work::test`, rather than one Beds per thread.

The only `rand()` call found in `src/femera/grid/elem/Bars.perf.cpp` perturbs
interior node coordinates. It is currently behind `is_unstructured = false`,
and scales `rand() / RAND_MAX` to `[0, 1]`. The benchmark already contains
unrelated local modifications: preserve them.

## Design and implementation

1. Add a public, clearly named random-draw API to `test::Beds`, with generator
   state privately owned by that Beds instance. Use a standard, specified
   engine such as `std::mt19937`, seeded with the formula above. Define the
   raw result type and range and, if a normalized draw is provided, implement
   its scaling explicitly from the engine's range rather than `RAND_MAX`.
   Document whether the normalized interval includes its endpoints. Avoid
   relying on standard-library distribution algorithms for a cross-library
   reproducibility promise.
2. Initialize all supported thread streams in `Beds::task_init`, before
   `set_init(true)` and before child tests are initialized. Ensure each seed
   uses `this->proc->get_proc_id()` as seen by the corresponding thread,
   including MPI rank contributions. Do not initialize only the calling
   thread's `thread_local` state: `mini->init` normally runs outside an OpenMP
   parallel region and worker threads must also be ready on first use.
3. Use a fixed, per-Beds collection of engine states and a stable mapping from
   each supported execution thread to its own state; no concurrent calls may
   mutate the same engine. The global process ID is a seed input, not
   necessarily a safe dense local array index. Confirm the configured process
   hierarchy, team sizes, and the behavior of `get_proc_id()` on every worker;
   in `Proc.ipp` the recursive implementation follows only child path 0.
   Never assume IDs are unique without testing them. Fail initialization or
   report an explicit error if the requested mapping cannot be established.
   Cover serial and ordinary OpenMP teams, including a serial draw before or
   after a parallel region without resetting that thread's stream.
4. Define and document the supported lifecycle and topology: deterministic
   reseeding on each successful Beds initialization, no calls before init or
   after exit, and no use by threads outside the initialized mapping. Check
   dynamic or changed team sizes, nested OpenMP levels and `FMR_OMP_LOCAL`
   against the actual hierarchy. Support them only if stable, unique IDs and
   initialized state can be guaranteed; otherwise make unsupported use fail
   explicitly rather than sharing a stream or silently reseeding. Account for
   allocation/initialization errors through the existing task-init error path.
   Reinitializing or destroying a Beds must not affect another Beds instance.
5. In `Bars.perf.cpp`, replace the global `rand()` call with the Beds API and
   scale using the new API's actual range. Preserve the existing perturbation
   amplitude, centered interval, endpoint exclusion of the bar, and other
   current benchmark behavior; the exact random geometry is expected to change.
   Do not modify the unrelated local changes or enable unstructured mode by
   default.
6. Document API, seed arithmetic, engine and range, reinitialization behavior,
   topology limits and reproducibility scope in `src/femera/test/README.md`.
   Reproducibility requires the same seed, process/thread-ID mapping and
   per-thread draw order; it is not independent of work scheduling.

## Regression and validation plan

- Extend `src/femera/test/Beds.gtst.cpp` with a known-answer check against a
  separately constructed `std::mt19937` seeded with the exact formula. Exercise
  arithmetic at zero, near `UINT_MAX`, at `UINT_MAX`, and across the modulo
  boundary without relying on overflow. Compare several consecutive draws.
- Record multi-thread IDs and sequences independently in an OpenMP region,
  assert IDs and state ownership are distinct where expected, and compare
  each sequence with its independently seeded reference. Repeat with the same
  configuration, altered draw counts and scheduling to show that one thread's
  draws do not advance another's stream. Verify the main thread's sequence
  across serial/parallel transitions and separate Beds instances.
- Exercise reset/reinitialization and explicit failures for unsupported access
  and topology changes. When supported by the build, test serial, multiple
  OpenMP team sizes, configured nested/local modes, and multiple MPI ranks;
  assert the actual IDs and seeds, not merely that generated values differ.
  Include builds with the default and an overridden `FMR_RAND_SEED`.
- Test normalized range and perturbation bounds, unchanged endpoint
  coordinates, repeatable perturbed coordinates and valid element Jacobians.
  Exercise the unstructured branch in a focused test without changing the
  benchmark's default configuration; also run the default benchmark.
- Build and run the focused Beds GoogleTest and Bars performance targets using
  the repository Makefile and available configurations; ensure GoogleTest is
  enabled and the tests actually run. In this repository GoogleTest runs during
  test-task exit, so RNG state must remain alive through child-task exit.
  Use a thread race detector if available. Report unavailable configurations
  and any pre-existing build/test failures rather than masking them.
