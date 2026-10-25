# Small-matrix compiled-code review

Date: 2026-10-25

## Scope and build

Inspected the current `build/Ultra-7-255H/fmr/smat.perf` binary and its saved
run log, alongside `src/fmr/smat.perf.cpp` and `src/fmr/smat.ipp`. The binary
and object timestamps are newer than the relevant source files. Build metadata
identifies GCC 11.5.0 on the Ultra-7-255H with `-march=native`, `-Ofast`,
`-flto=auto`, loop unrolling, and tree vectorization. `-DFMR_PERF_PCT=50` is
active.

`objdump -d -C` shows the benchmark helpers were inlined into their GoogleTest
test bodies, so the variant implementations do not appear as independent
symbols. The assembly observations below refer to those inlined paths.

## Findings

- **2x2 arithmetic is scalar, even though the compiler uses YMM registers to
  assemble and store the four results.** In the inlined 2x2 loop, the
  determinant reciprocal is a scalar `vdivsd`; result calculations use scalar
  `vmulsd`; then values are packed with `vunpcklpd`/`vinsertf128` and stored
  with `vmovupd`. This is not SIMD arithmetic across independent matrices.
- **The 3x3 `_a` inverse path has compiler-generated AVX2/FMA arithmetic.**
  The loop loads the 3x3 values, rearranges them using `vperm2f128`,
  `vpermpd`, `vblendpd`, and shuffle instructions, computes cofactors using
  packed `vmulpd` and FMA instructions, performs a scalar `vdivsd` for the
  determinant reciprocal, broadcasts that reciprocal, and stores packed
  output. This demonstrates SIMD within one matrix, though with considerable
  data rearrangement.
- **The benchmark checks the return status but does not read inverse-result
  values; the current binary nevertheless emits the result stores.** The 2x2
  loop packs and stores the inverse output; the 3x3 `_a` path stores its output
  with packed stores. Keep an explicit output check or checksum in mind so a
  future compiler/build change cannot silently remove the work.
- **The saved run supports investigating 2x2 `_b`, not promoting it.** It
  measured 814 Minv/s for reference, 837 Minv/s for `_a`, and 532 Minv/s for
  `_b`. `_b` was about 35% slower than the reference in that run and narrowly
  failed the 50% relative tolerance. `smat_invt_det<2>` measured 840 Minv/s;
  the 2x2 determinant-returning variants measured 796 and 717 Minv/s.
- **3x3 results vary by operation and should not be generalized from this one
  run.** Inverse-only `_a` measured 379 Minv/s versus 337 Minv/s for reference
  (about 12% faster). For inverse-transpose plus determinant, reference
  measured 368 Minv/s, `_a` 304 Minv/s, and the generic entry point 362
  Minv/s; `_a` was about 17% slower than reference in that run.
- **The saved timing test is noisy near its threshold.** The saved log fails
  only 2x2 case 2: 0.01577 s versus a 0.01031 s baseline, just beyond the
  50% tolerance. A subsequent run under `perf stat` passed all tests, but had
  noticeably different timings (for example, 644 Minv/s baseline versus 507
  Minv/s for case 2). Treat a single pass/fail or one-run ranking as
  insufficient evidence for selecting a default.
- `perf` is installed and ran, but `cycles`, `instructions`, `branches`, and
  `branch-misses` all reported `<not supported>` in this execution context.
  No usable PMU counts were collected.

## Optimization opportunities to investigate

1. **Reassess 2x2 `_b` or remove it from the candidate set.** Its source
   materializes cofactors into the output and then scales them by the inverse
   determinant. The run is slower than both `_a` and the generic transposed
   inverse path. Inspect its exact instruction block and register/memory
   traffic before keeping this alternative.
2. **Prototype batching independent matrices for SIMD.** The current
   array-of-matrices layout makes 2x2 calculations scalar per matrix. An
   internal structure-of-arrays or small AoSoA batch could place matching
   elements from multiple matrices in lanes, potentially replacing scalar
   divides with packed reciprocal/divide work. This changes data handling and
   should be benchmarked as a separate candidate; do not assume it wins for
   this small-kernel workload.
3. **Measure the 3x3 shuffle cost against a simpler scalar/cofactor schedule.**
   `_a` already gets packed arithmetic and FMA, but its lane construction
   requires many permutations and blends. Compare retired instructions and
   cycles when usable counters are available, not just the source-level FLOP
   count.
4. **Repeat timing over multiple working-set sizes and randomized order.**
   These tests use a single deterministic diagonal matrix and a single
   default problem size. Use varied non-singular matrices and run each
   implementation in alternating/repeated order. Keep correctness checking
   separate from timing if its branch/status accumulation affects results,
   while ensuring output values remain observable.
5. **Keep floating-point semantics in view.** `-Ofast` enables aggressive
   floating-point transformations and this build emits FMA. Recheck
   determinant classification and inverse accuracy on near-singular and
   non-diagonal inputs before adopting any schedule that changes operation
   order.

## Reproduction notes

Useful commands:

```sh
objdump -d -C build/Ultra-7-255H/fmr/smat.perf
nm -S -C build/Ultra-7-255H/fmr/smat.perf
perf stat -e cycles,instructions,branches,branch-misses -- \
  build/Ultra-7-255H/fmr/smat.perf -fmr:o2 -fmr:T
```

For meaningful per-variant PMU comparisons, isolate a single variant per run
or add a benchmark mode that selects one case. The current `perf stat` run
could not read hardware events here.
