# Femera runtime interfaces and CRTP migration plan

Date: 2026-10-26

Status: architectural proposal; no implementation changes made.

## Objective

Preserve Femera's public facades and optional-backend independence while making
heterogeneous task access type-safe. Keep CRTP for concrete implementation hooks,
and introduce actual common base types for runtime-selected family members.

Applications must continue to compile without headers or libraries for disabled
backends such as CGNS. They should continue to access data operations through
`File`, processing operations through `Main`, and the corresponding public
interfaces for tasks and tests.

The previously discussed initialization-order issue is already fixed and is not
part of this proposal.

## Current design and the distinction to preserve

`Work` provides the ownership tree, metadata, and virtual lifecycle contract.
`Data<T>`, `Proc<T>`, `Task<T>`, and `Test<T>` provide CRTP implementation
machinery. Concrete classes include the public facades (`File`, `Main`, `Jobs`,
and `Beds`) and specialized or optional backends.

The facade design is appropriate. The problem is using the facade's concrete
pointer type to represent sibling backend objects:

- Within `Data<Cgns>`, casting `this` to `Cgns*` is valid when the object really
  is a `Cgns`. This is normal CRTP dispatch.
- Looking up a `Bank` or `Cgns` through `Data<File>` does not make that child a
  `File`. Casting its `Work*` to `File*` is an invalid downcast.
- The same distinction applies to different specializations of `Proc<T>`,
  `Task<T>`, and `Test<T>`. Different template specializations are different
  types, not a shared runtime family interface.

CRTP dispatches to a concrete type known at compile time. A heterogeneous task
tree needs a valid common type for runtime-selected objects.

Relevant source:

- [Work](../../src/femera/Work.hpp)
- [Data declarations](../../src/femera/Data.hpp) and
  [implementation](../../src/femera/Data.ipp)
- [Proc declarations](../../src/femera/Proc.hpp) and
  [implementation](../../src/femera/Proc.ipp)
- [Task declarations](../../src/femera/Task.hpp) and
  [implementation](../../src/femera/Task.ipp)
- [Test declarations](../../src/femera/Test.hpp) and
  [implementation](../../src/femera/Test.ipp)

## Proposed hierarchy

Introduce a non-template base for each family. The names below are illustrative,
not a naming requirement.

```text
Work
|-- DataNode
|   |-- Data<File> -- File
|   |-- Data<Bank> -- Bank
|   `-- Data<Cgns> -- Cgns       [optional]
|-- ProcNode
|   |-- Proc<Main> -- Main
|   |-- Proc<Fmpi> -- Fmpi       [optional]
|   `-- Proc<Fomp> -- Fomp       [optional]
|-- TaskNode
|   |-- Task<Jobs> -- Jobs
|   `-- Task<Sims> -- Sims
`-- TestNode
    |-- Test<Beds> -- Beds
    `-- ...
```

| Layer | Responsibility |
|-------|----------------|
| `Work` | Ownership tree, metadata, common lifecycle contract |
| Family base | Actual common family interface and shared state |
| CRTP adapter | Dispatch from the family/lifecycle interface to concrete hooks |
| Public facade | Application entry point, routing, orchestration |
| Concrete backend | Specialized implementation details |

Keep ordinary single inheritance. There is no need for a diamond hierarchy,
virtual inheritance, or making every backend inherit from the concrete facade.

The existing core pointers may continue to refer to the actual `Main`, `File`,
and `Beds` facade objects. A backend should not need to masquerade as one of
those objects.

## Retain valid CRTP dispatch

The lifecycle dispatch in `Data<T>::init()` is an appropriate use of CRTP:

```cpp
static_cast<T*>(this)->task_init(argc, argv);
```

A conceptual adapter structure is:

```cpp
class DataNode : public Work {
protected:
  explicit DataNode(Work::Core_ptrs_t core) noexcept
      : Work(core) {}
};

template <typename T>
class Data : public DataNode {
protected:
  explicit Data(Work::Core_ptrs_t core) noexcept
      : DataNode(core) {}

  T& implementation() noexcept {
    return static_cast<T&>(*this);
  }

  // Existing lifecycle overrides dispatch to implementation().
};
```

This sketch is not a complete replacement for the current classes. Preserve
existing lifecycle behavior, exception translation, initialization state,
construction requirements, and ownership during implementation.

Keep CRTP casts local to adapters operating on their own concrete object.
Do not use the adapter's `T` to identify arbitrary children.

## Separate generic lookup from family lookup

Use distinct return contracts, for example:

```cpp
Work* find_work(Task_type type, fmr::Local_int index = 0) noexcept;
DataNode* find_data(Task_type type, fmr::Local_int index = 0) noexcept;
ProcNode* find_proc(Task_type type, fmr::Local_int index = 0) noexcept;
```

Equivalent overloads can preserve existing index/path lookup capabilities.

- Generic traversal returns `Work*`.
- Family traversal returns the corresponding actual family base.
- Lookup does not return the concrete type of the object performing the lookup.
- A lookup for an absent backend can return `nullptr`, with that behavior
  documented and checked by callers.
- A requested operation on an unavailable backend must report unavailability
  through Femera's error mechanism, rather than silently pretending to succeed.

This distinction matters for `Jobs`, whose children include processing, data,
and test objects, not just members of its own task family.

If RTTI is enabled, a checked `dynamic_cast<DataNode*>` from `Work*` is the
simplest initial family check. It requires the common family definition, not
optional concrete backend headers.

If avoiding RTTI is a firm requirement, use an explicit family-query mechanism
or enforce family membership through typed containers. Do not substitute
another unchecked cast without an established type invariant.

Do not change traversal semantics incidentally. The current
[Work lookup implementation](../../src/femera/Work.cpp) deliberately descends
only the first-child branch. Decide and document whether that remains the
intended contract; broader traversal is a separate behavior change.

## Keep routing in public facades

### Initial approach: preserve explicit backend dispatch

[File::send](../../src/femera/data/File.cpp) already performs internal routing
using task identifiers. Retain this approach during the first migration:

1. Retrieve a valid `DataNode*` or `Work*`.
2. Identify the concrete backend internally.
3. Perform a checked concrete cast.
4. Call its implementation.

Keep optional includes and dispatch branches behind feature guards in
implementation files. Application-facing headers must remain independent of
CGNS, MPI, and other optional implementation headers.

This is the smallest architectural migration. It does not require introducing
a virtual method for every operation.

### Selective improvement: virtual common operations

Where multiple backends implement a genuine shared operation, put that contract
on the family base and implement it through the CRTP adapter.

For example:

```cpp
class ProcNode : public Work {
public:
  virtual fmr::Local_int get_proc_ix() noexcept = 0;

protected:
  explicit ProcNode(Work::Core_ptrs_t core) noexcept
      : Work(core) {}
};

template <typename T>
class Proc : public ProcNode {
public:
  fmr::Local_int get_proc_ix() noexcept final override {
    return static_cast<T*>(this)->task_proc_ix();
  }

  // Other existing CRTP machinery.
};
```

Through a `ProcNode*` referring to `Fomp`, this invokes the `Proc<Fomp>` adapter,
then the actual `Fomp` hook. It removes the need for
[Main::task_proc_ix](../../src/femera/proc/Main.cpp) to impersonate another
processing type and manually route to its implementation.

Move shared processing state, such as `base_id`, `base_n`, and `proc_n`, into
`ProcNode`. Rewrite processing traversal to use the actual common base rather
than treating different `Proc<T>` specializations as interchangeable.

Do not force unrelated operations onto every backend. An in-memory bank and a
mesh-file reader need not implement the same text-output contract. Retain
facade routing for those differences, or introduce a narrower capability
interface only when it has a clear use.

## Performance boundary

`Work` is already polymorphic through its lifecycle methods and destructor.
Adding a family base does not inherently require another allocation or another
separately owned object.

Place runtime dispatch at backend-selection boundaries:

```text
Application
    -> facade / backend selection
        -> concrete operation
            -> tight numerical or data-processing loop
```

Keep templates, CRTP, and concrete data access inside hot loops. Avoid virtual
dispatch per value or per finite element unless measurements justify it.

Measure any performance-sensitive changes rather than assuming that all virtual
dispatch is expensive or that all existing casts are necessary for speed.

## Incremental migration

1. **Correct generic lookup return types.**
   Return `Work*` rather than the caller's concrete `T*`. Update dependent
   call sites together so each migration step builds.

2. **Introduce the four non-template family bases.**
   Move common family state and genuinely common operations into them.
   Preserve concrete factories, lifecycle ordering, and ownership.

3. **Add checked family lookup.**
   Define absent-backend and wrong-family results explicitly. Keep optional
   concrete types out of public lookup signatures.

4. **Preserve the existing public facades.**
   Keep `File`, `Main`, `Jobs`, and `Beds` as application entry points.
   Retain guarded concrete backend dispatch initially.

5. **Replace manual switches selectively.**
   Introduce virtual family operations where they express a real shared
   contract and simplify dispatch. Do not expand the abstraction unnecessarily.

6. **Validate behavior and build configurations.**
   Confirm optional-backend independence and lifecycle correctness before
   considering the migration complete.

## Validation checklist

- Generic lookup returns the actual `Work` subobject of each retrieved object.
- Family lookup succeeds for the correct family and rejects the wrong family.
- Metadata and lifecycle methods work through common base pointers.
- Processing operations dispatch to the actual backend implementation.
- Existing initialization and shutdown ordering is preserved.
- Lookup behavior for missing backends and indexes is documented and tested.
- Requests for unavailable functionality produce explicit errors.
- A minimal supported build compiles without optional backend dependencies.
- Enabled optional-backend builds retain their expected behavior.
- Application translation units do not need optional concrete backend headers.
- Existing tests are updated to use valid common interfaces.
- Run targeted tests and applicable undefined-behavior/vptr sanitizer checks
  where supported by the toolchain.
- Measure representative hot paths if runtime dispatch changes affect them.

## Decisions to resolve before implementation

- Final names for non-template family bases and CRTP adapters.
- Whether RTTI is available and acceptable for checked lookup.
- Exact shared operations belonging on each family base.
- Whether first-child-branch lookup remains the intended traversal contract.
- Whether "loadable" means conditionally compiled handlers or actual runtime
  shared-library plugins.

Actual shared-library plugins require separate decisions about registration,
object lifetime across module boundaries, and ABI stability. Do not introduce
a plugin ABI unless runtime loading is required.

## Recommendation

Keep the facade architecture. Add real common bases for backend objects, and
use CRTP to adapt each concrete implementation to those bases. Do not use CRTP
to represent sibling backend objects as instances of the public facade.
