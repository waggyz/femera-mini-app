
The codebase appears to be structured as a scientific computing library named "femera", built around a central task management hierarchy. It leverages various technologies like MPI and OpenMP for parallel processing and libraries like CGNS, MOAB, Gmsh, and PETSc for data handling and numerical computations. 

Here's a breakdown of the codebase structure based on the provided files:

**Core Components:**

* **`femera.hpp`:** The primary header file, providing the public interface to the "femera" library. It defines essential types like `Jobs_t` and factory functions like `new_jobs()` for creating simulation job managers.

* **`femera/core.h`:**  Includes headers for the core modules: `test/Beds.hpp`, `proc/Main.hpp`, `data/File.hpp`, and `task/Jobs.hpp`, forming the fundamental structure of the library.

* **`femera/mini.cpp`:**  Likely a minimal example application demonstrating how to use the "femera" library to set up and run a simulation job.

**Task Management Hierarchy:**

* **`femera/Work.hpp`, `femera/Work.cpp`:** Defines the abstract base class `Work` that provides a foundation for task management. It outlines common functionalities like adding, deleting, and getting tasks, managing timers, and interacting with processing, data, and testing modules.

* **`femera/Proc.hpp`, `femera/Proc.cpp`:**  Introduces the abstract class `Proc`, derived from `Work`, focusing on process management. It handles aspects like determining process ranks, counts, and IDs across various parallel computing paradigms.

* **`femera/Data.hpp`, `femera/Data.cpp`:** Defines the abstract class `Data`, also derived from `Work`, for data management. It provides methods for interacting with data handlers and managing input and output data.

* **`femera/Test.hpp`, `femera/Test.cpp`:** Introduces the abstract class `Test`, derived from `Work`, responsible for testing and performance evaluation. It facilitates integration and unit testing and performance benchmarking.

* **`femera/Task.hpp`, `femera/Task.cpp`:**  Defines the abstract class `Task`, derived from `Work`, for specific task handling within the simulation. It offers methods for initializing and exiting tasks and managing their execution.

**Concrete Implementations:**

* **Processing (`femera/proc`)**: 
    * `Main.hpp`, `Main.cpp`: Central process handler, creating and managing other process-related modules like `Root`, `Node`, `Fcpu`, `Fmpi`, `Fomp`, and `Nvid`.
    * `Root.hpp`, `Root.cpp`: Handles operations at the root level of the process hierarchy.
    * `Node.hpp`, `Node.cpp`: Manages processes within a computing node.
    * `Fcpu.hpp`, `Fcpu.cpp`:  Handles CPU-related processes.
    * `Fmpi.hpp`, `Fmpi.cpp`:  Interface for MPI (Message Passing Interface).
    * `Fomp.hpp`, `Fomp.cpp`:  Interface for OpenMP (Open Multi-Processing).
    * `Nvid.hpp`, `Nvid.cpp`:  Handles NVIDIA GPU processing.

* **Data Handling (`femera/data`)**:
    * `File.hpp`, `File.cpp`: Main data handler, interacting with other data-related modules like `Logs`, `Text`, `Dlim`, `Bank`, `Cgns`, `Moab`, `Gmsh`, and `Pets`.
    * `Logs.hpp`, `Logs.cpp`:  Handles logging and messaging.
    * `Text.hpp`, `Text.cpp`:  Manages text-based data.
    * `Dlim.hpp`, `Dlim.cpp`:  Handles delimited data, likely CSV.
    * `Bank.hpp`, `Bank.cpp`: Manages data storage and retrieval.
    * `Cgns.hpp`, `Cgns.cpp`:  Interface for CGNS (CFD General Notation System).
    * `Moab.hpp`, `Moab.cpp`:  Interface for MOAB (Mesh Oriented Database).
    * `Gmsh.hpp`, `Gmsh.cpp`:  Interface for Gmsh (mesh generation).
    * `Pets.hpp`, `Pets.cpp`:  Interface for PETSc (Portable, Extensible Toolkit for Scientific Computation).

* **Testing (`femera/test`)**:
    * `Beds.hpp`, `Beds.cpp`:  Main test handler, incorporating other testing modules like `Gtst`, `Self`, and `Perf`.
    * `Gtst.hpp`, `Gtst.cpp`:  Interface for GoogleTest.
    * `Self.hpp`, `Self.cpp`:  Handles integration self-tests.
    * `Perf.hpp`, `Perf.cpp`:  Manages performance tests and benchmarks.

* **Tasks (`femera/task`)**:
    * `Jobs.hpp`, `Jobs.cpp`:  The central task manager, orchestrating simulations, adding processing, data handling, and testing modules.
    * `Sims.hpp`, `Sims.cpp`:  Handles simulations, likely creating and managing `Runs`.
    * `Runs.hpp`, `Runs.cpp`:  Manages individual simulation runs.

**Auxiliary Modules:**

* **`fmr` Namespace:**  Provides fundamental types, constants, and utilities, forming a lower-level foundation used by the "femera" library.
    * `fmr/fmr.hpp`:  Defines core types like `Exit_int`, `Team_int`, `Align_int`, data types (`Dim_int`, `Enum_int`, `Local_int`, `Global_int`, `Hash_int`), performance types (`Perf_int`, `Perf_float`, etc.), and data type enumerations (`Vals_type`, `Data_type`).
    * `fmr/form.hpp`, `fmr/form.cpp`:  Provides formatting utilities for strings, numbers, and time.
    * `fmr/math.hpp`, `fmr/math.cpp`:  Offers mathematical utilities, including functions for polynomial calculations, integer operations, and floating-point comparisons.
    * `fmr/perf.hpp`, `fmr/perf.cpp`, `fmr/perf/Meter.hpp`, `fmr/perf/Meter.cpp`: Defines a performance measurement framework, including a `Meter` class for tracking time, counts, and speeds.
    * `fmr/proc.hpp`, `fmr/proc.cpp`:  Provides utilities for obtaining system and process information like core counts, memory usage, and NUMA information.

* **`zyclops` Namespace:**  Appears to handle hypercomplex number operations, especially those related to automatic differentiation using dual numbers and Cauchy-Riemann matrices.
    * `zyclops/Zmat.hpp`, `zyclops/Zmat.cpp`:  Defines a `Zmat` class for representing hypercomplex matrices and vectors.
    * `zyclops/Zomplex.hpp`, `zyclops/Zomplex.cpp`: Defines a `Zomplex` struct for storing information about hypercomplex numbers and arrays, including order, algebra, and layout.
    * `zyclops/zyc.hpp`, `zyclops/zyc.cpp`:  Provides core functionalities for working with hypercomplex numbers, including operations like addition, subtraction, multiplication, division, and derivative calculations.


This structure suggests a well-organized and modular codebase designed for high-performance scientific computing. The library appears to be designed to be extensible and adaptable, allowing users to leverage various parallel computing technologies and external libraries.

*Genertaed by Luna, Google Gemini 1.5 Pro, 2024-09-18.*
*Prompted by David Wagner from the entirety of Femera source code and, "Please describe the structure of the entire codebase."*

