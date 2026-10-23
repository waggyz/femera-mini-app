# Single-element and finite element patch test stub plan

Date: 2026-10-23

Status: stub milestone implemented through section 5. Numerical verification
cases and the later roadmap in sections 6-7 remain unimplemented.

## Recommendation: keep Ptch derived from Test<Ptch>

[`Ptch`](../../src/femera/test/Ptch.hpp) is a test task that uses meshes, not a
discretization task. Providing coordinates and connectivity does not require
inheriting from `Grid`.

This matches the current architecture:

- [`Test`](../../src/femera/Test.hpp) supplies test-task lifecycle and construction.
- [`Grid`](../../src/femera/Grid.hpp) supplies discretization-task lifecycle, not
  mesh storage or coordinate/connectivity interfaces.
- [`Beds`](../../src/femera/test/Beds.cpp) already constructs `Ptch` through
  `Test<Ptch>`.
- [`Tets`](../../src/femera/grid/elem/Tets.hpp) and
  [`Cube`](../../src/femera/grid/elem/Cube.hpp) provide reusable geometry data
  independently of `Grid`.

Use composition: test tasks consume small mesh fixtures and, eventually,
production grid and physics functionality. Avoid inheriting from both bases:
both derive from `Work` and provide final lifecycle overrides.

## 1. Separate responsibilities

Introduce a dedicated single-element test task, provisionally named `FE1e`,
alongside `Ptch`.

| Component | Responsibility |
|-----------|----------------|
| `FE1e : Test<FE1e>` | Single-element verification and diagnostics |
| Existing `Ptch : Test<Ptch>` | Multi-element patch verification |
| Shared test mesh fixtures | Coordinates, connectivity, topology metadata |
| Existing element classes | Shape functions, quadrature, reference geometry |

Do not repurpose the already-declared `Unit` task identity: single-element tests
are a particular numerical verification suite, not the entire unit-testing
interface.

## 2. Stub the single-element task first

Proposed new files beneath [`src/femera/test/`](../../src/femera/test/):

| Proposed file | Purpose |
|---------------|---------|
| `FE1e.hpp` | CRTP task declaration, constructor, lifecycle hooks |
| `FE1e.ipp` | Short constructor/lifecycle definitions following neighboring tasks |
| `FE1e.cpp` | Eventual case orchestration and diagnostics |
| `FE1e.gtst.cpp` | Lifecycle and fixture-contract tests using existing GoogleTest integration |

Initially scope the numerical roadmap to a linear, four-node tetrahedron.
Later extend to the existing ten- and twenty-node tetrahedra.

Retain the existing low-level tests in
[`Tets.gtst.cpp`](../../src/femera/grid/elem/Tets.gtst.cpp); do not duplicate or
relocate them during stubbing.

## 3. Stub a small shared mesh-fixture interface

Proposed new files in the same test directory:

| Proposed file | Purpose |
|---------------|---------|
| `Mesh.hpp` | Lightweight test-only mesh description and named fixture declarations |
| `Mesh.cpp` | Eventual fixture construction |
| `Mesh.gtst.cpp` | Fixture integrity tests |

Keep this a plain data/helper interface, not another `Work` or `Grid` task.
Record:

- Spatial dimension, node count, element count, and nodes per element.
- Node-major coordinates and element-major, zero-based connectivity.
- Boundary/interior node identification for later patch tests.
- Explicit conversion where existing kernels require component-major coordinates.

Initial fixture roadmap:

1. One reference Tet4 using existing tetrahedron coordinates/connectivity.
2. Affinely transformed Tet4 variants.
3. Five- and six-tetrahedron cube fills reusing the existing tables.
4. A genuine patch fixture with unconstrained interior degrees of freedom.

Do not copy reference tables unnecessarily. Do not treat a cube fill whose
vertices are all prescribed boundary nodes as sufficient evidence of a solved
patch test.

## 4. Prepare, but do not implement, the patch suite

During the later stub implementation, update the existing
[`Ptch.hpp`](../../src/femera/test/Ptch.hpp),
[`Ptch.ipp`](../../src/femera/test/Ptch.ipp), and
[`Ptch.cpp`](../../src/femera/test/Ptch.cpp) to document the chosen
responsibilities and shared fixture dependency.

Add a proposed `Ptch.gtst.cpp` for lifecycle and fixture contracts.

Before using the cube fills numerically, verify signed Jacobians and face
conformity. The `cube_conn_5tet` table in
[`Tets.hpp`](../../src/femera/grid/elem/Tets.hpp) explicitly requires rotation
for tiling; repeated cells must have compatible face triangulations.

## 5. Wire task registration without changing existing behavior

During the later stub implementation:

- Add the `FE1e` forward declaration beside the test tasks in
  [`Work.hpp`](../../src/femera/Work.hpp).
- Add its built-in identity in [`type.hpp`](../../src/femera/type.hpp), avoiding
  renumbering existing identities.
- Register it before `Ptch` under the existing `-T` path in
  [`Beds.cpp`](../../src/femera/test/Beds.cpp).
- Follow source discovery and test-target conventions in the
  [`Makefile`](../../Makefile); add no new build system or dependencies.

`FE1e` has a distinct explicit enum value so existing built-in and user task
identities do not shift. The existing Makefile discovers the new source and
GoogleTest files automatically.

Stub initialization must not claim numerical tests passed. Leave unimplemented
cases unregistered or explicitly disabled.

## 6. Subsequent numerical implementation order

These steps follow the stub milestone; they are not part of stubbing.

1. **Geometry/interpolation:** coordinate layout, nodal interpolation, partition
   of unity, gradient sum, full Jacobian, positive volume, and quadrature.
2. **Single-element fields:** rigid translation/rotation and affine displacement
   reproduction.
3. **Physics verification:** constant strain/stress, force balance, energy, and
   stiffness properties where production APIs exist.
4. **Patch verification:** assembled interior equilibrium, solved interior
   displacement, strain/stress reproduction, and reactions.
5. **Extensions:** distorted meshes, higher-order elements, and additional
   physics.

The inspected [`Elas`](../../src/femera/phys/Elas.hpp) and
[`FEms`](../../src/femera/grid/FEms.hpp) task interfaces are skeletal. Identify
the actual numerical kernels before promising solver-level tests; do not invent
substitute production solvers inside test fixtures.

## 7. Stub acceptance criteria

- New task and fixture tests build using existing tooling.
- Lifecycle and registration tests pass.
- Existing targeted element tests remain unchanged and pass.
- `-T` includes single-element tests before patch tests; normal execution and
  `-P` retain their behavior.
- GoogleTest-disabled builds do not gain a GoogleTest dependency.
- No placeholder reports an unimplemented numerical test as successful.
- Documentation distinguishes the stub milestone from later numerical
  implementation.

[Edited by David Wagner: replaced Selt with FE1e.]
