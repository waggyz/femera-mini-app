# Free mesh-access proposal documents

Date: 2026-11-26

- [Design summary](tile-free-design-summary-2026-11-26.md): short overview of
  the preferred element-based `Free<Bars>` policy, compared with `Free<Bar1>`
  and raw `Tile<Free>` copies.
- [Implementation plan](tile-free-unstructured-mesh-plan-2026-11-26.md):
  proposed signatures, packed-array layouts, caller obligations, alternatives,
  element-based versus tile-based metadata, homogeneous partition contracts,
  single-record simplex geometry, bar-kernel integration, and
  correctness/optimization/performance checks.

The revision prefers `Free<T, P, F>` using `Elem<T, P, F>`, with caller-owned
arrays, homogeneous partition layouts, and one stored geometric record per
natural-node simplex element regardless of quadrature count or solution order.
It supersedes tile-based metadata recommendations and retains the earlier
alternatives for comparison.

Status: Bars-only prototype implemented in `femera::grid::free`. Both focused
tests and C++14 syntax checks of the cleaned translation units passed.
Caller integration, further element support, and measurements remain future work.
