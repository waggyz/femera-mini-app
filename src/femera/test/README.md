# Femera integration tests

This folder contains partial and full end-to-end integration tests.

## Reproducible random sequences

`mini->test->get_rand()` returns the next `std::mt19937` integer in the inclusive
range `[0, mini->test->get_rand_max()]`. Each Beds instance owns a separate
generator for every thread in the OpenMP team created during `Beds::init`.
Without OpenMP there is one generator. The initialized processing hierarchy
provides each thread's seed:

```cpp
(fmr::rand_seed + proc->get_proc_id()) % std::numeric_limits<unsigned int>::max()
```

`Beds::get_rand_seed(proc_id, base_seed)` implements this mathematical formula
without overflow; `base_seed` defaults to `fmr::rand_seed`. The modulus is
`UINT_MAX`, not `UINT_MAX + 1`, so seeds can wrap and collide. Set the existing
`FMR_RAND_SEED` build definition to override the default base seed of 309.

For a normalized value, divide the draw by `get_rand_max()` in the desired
floating-point type. This includes both 0 and 1; subtracting 0.5 produces the
inclusive centered interval used for Bars mesh perturbations. This explicit
scaling avoids implementation-dependent standard-library distributions.

Initialize Beds outside OpenMP regions, after the processing hierarchy.
Generators are ready before child tests initialize and remain available while
child tests exit. Exit clears their state; initialization after exit reseeds
the streams. Do not initialize, exit, or destroy Beds concurrently with draws.
Calling init on an already initialized Beds is an error.

Serial calls and first-level OpenMP calls continue each thread's sequence.
Smaller teams are supported when their thread-to-slot mapping is unchanged.
Nested regions (including serialized nested regions), larger-than-initialized
teams, replacement workers, foreign `std::thread` callers, and changed process
IDs throw `femera::Errs`, as do draws before init or after exit. Configuration
of multiple OpenMP levels alone does not prevent first-level use. Initialization
rejects a hierarchy that gives multiple workers the same process ID.

Sequences are reproducible for the same base seed, process/thread IDs, engine
and per-thread draw order. Interleaving draws on different threads does not
advance another thread's stream. Changing task scheduling or MPI/OpenMP layout
can change which sequence supplies a mesh coordinate. These generators are for
testing, not cryptographic use.

- `femera::test::fems::Self`: orchestrates Femera integration self-tests
- `femera::test::fems::FE1e`: provides and tests single element geometric data and methods. These are also used for Phys and Algo tests.
- `femera::test::fems::Ptch`: provides and tests patch geometric data and methods. These are also used for Phys and Algo tests. Results can be compared to element single tiles provided by the Elem class.
- `femera::test::fems::Phys`: provides and tests material data and methods.
- `femera::test::fems::Algo`: provides and tests solver data and methods.

