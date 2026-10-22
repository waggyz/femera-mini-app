# Plan: Replace Femera Dependency Installation With Spack

Date: 2026-10-21  
Status: Proposed migration; implementation and build validation have not begun.

## Goal And Scope

Replace Femera's broken bespoke external package installation system with Spack while retaining the existing GNU Make application build, feature switches, sources, and tests. Establish a dependency-only environment first. Add a Femera Spack recipe as a separately gated later stage.

Do not repair the legacy installers or require them to produce a working baseline. This migration does not require converting the application to CMake, implementing unsupported features, broadly refactoring numerical code, or upgrading every dependency.

Use existing Spack recipes for upstream packages. The DOT graph mixes installation responsibility with actual dependencies, so it is an inventory rather than a directly importable dependency graph. Leave unsupported dotted features disabled. Do not declare dependencies solely because PETSc previously downloaded them.

## Current Architecture

The following observations are grounded in the inspected repository:

- The root Makefile loads recommended and personal configuration and derives CPU-specific build, staging, and installation directories. Compiler setup is conditional on literal compiler names: explicitly setting an MPI wrapper can bypass the options selected for `g++`.
- The Makefile injects `PREFIX` and CPU-specific prefix paths into include, link, and runtime searches. These can accidentally supply old dependencies during a Spack migration.
- It includes generated `$(BUILD_CPU)/external-config.mk`. The `intro` target concatenates `external/*-config.mk` into that file, coupling feature/link configuration to installer configuration and generated state.
- `INSTALL_EXTERNAL` is derived from `LIST_EXTERNAL`. The default `all` target invokes `external`, which invokes `install-external`; pattern rules execute the legacy installation scripts.
- `external/petsc-config.mk` combines feature macros, linker flags, PETSc configuration, installation bookkeeping, and many `--download-*` options.
- `mini` is not a compile-only smoke test: it runs GoogleTest output targets, builds the executable, invokes a Valgrind-log target, optionally runs pymera, and invokes other completion work.
- Some link recipes ignore failures, and the pymera wrapper can report a failure without returning a failing status. A successful Make exit alone does not certify acceptance.
- The README's manual PETSc configuration uses shared libraries and 64-bit indices, while the installer configuration requests static PETSc. Historical compiler and index-compatibility notes require verification against the selected versions.
- No top-level CMake application build was found in the inspected repository. Third-party projects may use CMake internally.

## Design Decisions

1. **Separate ownership.** Spack owns third-party fetching, patches, configuration, compilation, and installation. Femera owns its compilation, feature macros, tests, staging, and installation.
2. **Use discovery-only integration.** Introduce `tools/dependencies.mk`, independent of installer configuration. Prefer `pkg-config` for include and complete link requirements. Use explicit dependency prefixes or installed metadata where a usable `.pc` file is unavailable.
3. **Fail clearly.** An enabled but undiscoverable dependency is an actionable error. Never silently disable the feature or fall back to an installer.
4. **Keep normal builds offline.** Provide an explicit dependency installation command or target. Ordinary compilation must not install dependencies. Resolve Spack information outside per-translation-unit recipes.
5. **Use a coherent stack.** Start with unified concretization and one intended MPI provider. OpenMPI and OpenBLAS are proposed initial defaults unless host/site requirements dictate otherwise. Keep site overrides separate from the common manifest.
6. **Choose ABI settings explicitly.** Verify PETSc scalar type, precision, index width, shared/static configuration, and compiler compatibility against application interfaces. Femera's integer typedefs alone do not establish `PetscInt` compatibility. Check CGNS size configuration independently.
7. **Prefer shared dependencies initially.** This reduces static-link complexity, subject to compile/link validation. Femera may still build its own static archive. If static dependencies are required, explicitly configure them and obtain complete static link metadata rather than guessing transitive `-l` flags.
8. **Preserve user state.** Do not overwrite personal configuration, delete installed libraries, or remove submodules during the initial migration. Retain legacy installers temporarily as retired references, unreachable from supported builds, without an automatic fallback.

## Phase 0: Record The Build Contract

1. Inspect working-tree changes, applicable repository instructions, active feature settings, compiler/MPI selection, relevant source API uses, and Spack availability. Record the chosen Spack release and package-repository revision.
2. Create a feature matrix distinguishing requirements for `mini`, optional linked libraries, Python bindings, testing/build tools, and standalone meshing/rendering applications. Verify actual consumers rather than deriving requirements from installation arrows.
3. Record the first supported profile: compiler, CPU target, PETSc scalar/precision/index settings, CGNS width, shared/static policy, and necessary Gmsh capabilities.
4. Evaluate LTO and profile-generation separately from initial correctness. Do not propagate legacy `-march=native` or `-Ofast` options indiscriminately into dependencies.

**Gate:** An explicit first supported profile and ABI contract, with each enabled feature mapped to a package or compiler facility. A working legacy installation is not required.

## Phase 1: Establish The Spack Environment

Depends on Phase 0. Recipe/variant inspection can proceed alongside the local consumer inventory, but profile selection must precede final concretization.

1. Add `spack/spack.yaml` using existing recipes, unified concretization, an optional default view, and explicit MPI/BLAS/LAPACK provider requirements. Verify variant names and syntax with `spack info` against the pinned package repository before encoding them.
2. Start with only the libraries and test tools required by the selected profile. Candidates include PETSc, HDF5, CGNS, Gmsh, GoogleTest, and numactl. Add Python, NumPy, mpi4py, petsc4py, pybind11, and Python test tools only when their corresponding features need them.
3. Select tested versions through concretization and smoke tests. Do not blindly pin every package to historical submodule versions or install every DOT node. Let PETSc's Spack recipe manage its dependencies instead of reproducing its old download orchestration.
4. Register site dependencies as accurately specified externals when necessary. Use `buildable: false` and provider requirements where substitution or rebuilding must be forbidden. Keep machine-specific absolute paths out of the shared manifest.
5. From the repository root, concretize and inspect the environment, then install it:

   ```bash
   spack -e ./spack concretize
   spack -e ./spack spec
   spack -e ./spack find -c -dl
   spack -e ./spack install
   ```

6. Confirm the intended MPI provider and compatible shared dependencies. Concretization alone does not prove application ABI compatibility or successful linking.
7. Preserve `spack/spack.lock` for the supported platform profile and record Spack and package-repository revisions. Other platforms or CPU targets may require separate concretization. Ignore generated views, logs, and local site configuration following repository conventions.

**Gate:** The environment installs, required metadata is available, and compiler/MPI/PETSc/CGNS compile/link probes pass for the enabled profile.

## Phase 2: Replace Installer Coupling With Discovery

Depends on the validated package interfaces from Phase 1. Make changes can be prepared earlier, but acceptance requires an installed environment.

1. Add `tools/dependencies.mk` with feature-to-dependency mapping, feature macros, metadata queries, include/link flags, and missing-package diagnostics. Reuse `FMRFLAGS`, `INCFLAGS`, `LDFLAGS`, and `LDLIBS`; do not build another dependency solver.
2. Include that file directly from the Makefile. Stop concatenating installer configuration in `intro` and stop depending on generated `external-config.mk`. Move necessary feature/link behavior from the old configuration files without retaining their download logic.
3. Remove legacy installer calls and installer-flag generation from default build and install paths, including recursive Make calls. Recommended interface: an explicit `deps` target runs `spack -e ./spack install`; the old `external` target reports the replacement command without attempting legacy installation.
4. Remove old dependency-prefix search-path injection from the supported Spack workflow while preserving Femera staging and destination paths. Discover package-specific locations rather than assuming one combined prefix. Configure appropriate RUNPATH/RPATH for dependencies and staged Femera libraries.
5. Separate compiler-family detection from literal `CXX` strings. Honor explicit compiler and MPI wrappers while preserving the language standard and required OpenMP options. Check which compiler the MPI wrapper actually invokes.
6. Make the environment/configuration identity participate in build invalidation, or use isolated build directories, so changing stacks cannot reuse ABI-incompatible objects. Document a safe rebuild procedure without invoking destructive `purge` or broad cleanup targets.
7. Add focused regression checks using existing shell/Bats conventions where practical: missing enabled dependency fails; disabled dependency is not required; a build works without generated external configuration; stale external configuration is ignored; representative targets cannot invoke retired installers; compiler wrappers retain required flags. Prefer hermetic fake metadata commands for integration tests.

**Gate:** Focused checks pass, a representative source compiles and links to Spack libraries, and logs contain no legacy installer calls or unintended old-prefix dependencies.

## Phase 3: Application Acceptance And Documentation

Depends on Phase 2.

1. Initialize Spack shell support and activate the environment with `spack env activate ./spack`. Perform an isolated clean build. Compile a representative target first, then run `make mini`, recognizing that it executes tests and additional hooks.
2. Run selected existing GoogleTest executables directly to obtain reliable exit status. Derive actual executable paths from the source tests and `BUILD_CPU`/`FMROUTS`; do not invent test names or assume `make check` exists. Inspect logs as well as Make status.
3. Execute a small MPI case with the same provider used to compile Femera and a representative existing mesh/solver input. Check process completion and numerical results. Ensure the MPI communicator-size probe uses the same stack.
4. Inspect dynamic dependencies with `ldd`/`readelf` on the executable and shared library when built. Confirm resolution to the selected Spack packages or approved system/site externals. Test runtime from a clean shell using the documented activation procedure, without relying on old CPU-prefix libraries.
5. When Python is enabled, run `make pymera` plus direct Python tests/imports using the environment's interpreter. Verify native and Python MPI/PETSc components agree. Do not rely solely on the existing wrapper target's status.
6. Install Femera into a temporary user-writable application prefix and verify expected artifacts and runtime behavior. The existing install target emphasizes tools/docs, so verify executable/library installation semantics before claiming application installation is complete. Never write application files into dependency prefixes.
7. Update the root README, external README, and example configuration guidance with prerequisites, pinned Spack/package revisions, environment installation/activation, verified build/test commands, optional profiles, externals, and stale-artifact recovery. Mark the legacy installers retired. Do not delete submodules as part of this phase.
8. Treat `build/external/external.dot` as historical generated installation output. Retain the conceptual graph separately or generate dependency views from Spack; do not make generated build output the new installation authority.

**Gate:** The selected application profile and tests run with the Spack stack; missing dependencies fail clearly; retired installers never execute; the documented workflow works without preexisting legacy installations. Report unrelated application failures separately rather than expanding this migration to repair them.

## Phase 4: Optional Femera Spack Recipe

Defer until Phase 3 passes. This is not a prerequisite for replacing the broken external installers.

1. Add a custom repository and a Femera `MakefilePackage` recipe using the layout/API supported by the pinned Spack release. Determine exact repository paths at this stage because Spack repository layouts differ by release.
2. Expose supported features as variants and declare conditional build/link/run/test dependencies. Translate them into the same discovery-only build contract, with automatic third-party installation permanently disabled.
3. Implement installation of the application into Spack's prefix. Address the current `PREFIX`/CPU-subdirectory split explicitly. Verify executable, libraries, tools, and enabled Python artifacts, not just documentation.
4. Test a local-source workflow with `spack develop` after the recipe exists. Confirm it agrees with the dependency-only developer workflow on enabled capabilities and ABI settings.
5. Only then consider removal of retired scripts/submodules, broader CI coverage, and binary caches. Review each removal and preserve unrelated user changes.

## File-Level Work

| File Or Area | Planned Role |
| --- | --- |
| `spack/spack.yaml` (new) | Shared dependency manifest and provider requirements |
| `spack/spack.lock` (generated) | Validated concrete platform profile |
| `tools/dependencies.mk` (new) | Discovery-only feature and link integration |
| Root Makefile | Installer decoupling, path isolation, compiler handling, rebuild identity |
| `examples/config.recommended` | Align documented feature switches with supported profiles |
| Root and external READMEs | Supported workflow and legacy installer retirement |
| Existing shell/Bats test structure | Focused dependency integration regression checks |
| `external/*-config.mk` | Reference for necessary existing feature/link behavior, not an installer repair project |
| `config.local` | Inspect and preserve personal settings |
| `tools/set-undefined.mk`, `tools/functions.mk` | Reuse existing configuration and test conventions |

## Completion Criteria

- Spack owns third-party installation; normal builds never download packages.
- Enabled dependencies are discovered explicitly and missing packages produce useful errors.
- One supported compiler/MPI/ABI profile builds and runs the selected application tests.
- Clean builds and environment changes cannot silently reuse old dependencies or incompatible objects.
- The documented workflow does not depend on legacy installer success or old prefix contents.
- Optional features and remaining unverified profiles are clearly identified.

## Validation Status

This plan is based on repository inspection and Spack documentation. Spack installation, concretization, application compilation, and runtime tests have not been performed as part of writing it. Exact variants, versions, baseline features, and executable test paths must be verified during implementation. Only this planning document is being added; no build behavior is changed.

## References

- [Spack environments](https://spack.readthedocs.io/en/latest/environments.html)
- [Package settings and externals](https://spack.readthedocs.io/en/latest/packages_yaml.html)
- [Defining Spack packages](https://spack.readthedocs.io/en/latest/packaging_guide_creation.html)