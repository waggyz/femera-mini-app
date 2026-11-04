# Refactor geometry class

- `Geom`: abstract base class (replaces `Grid`)
    - *Derived from `Work`, with `????` interface?*
    - `Mesh`: unstructured discretization (FE, BE)
        - 1 `Elem`: abstract single finite element base class
    - `Grid`: structured discretization (FD, FV, FFT, FE, BE)
        - 1 `Cell`: abstract single structured discrete volume base class
    - `CADs`: abstract geometry

- `Sims`: collection of `Part` classes to describe a model, with one or more `Runs`. `Runs` may change `Part` data, e.g., remeshing during progressive fracture.

- `Part`: computational geometric domain partition
    - *Derived from `Task`, with `Jobs` interface*
    - *Where do boundary conditions (BCs) go?*
    - 1 `Geom`: `Mesh`, `Grid`, `CADs` instance
    - 1 `Phys`: `Mtrl` instance
    - 1 `Algo`: solver, etc. instance
