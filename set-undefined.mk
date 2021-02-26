# Set undefined variables needed by the Makefile.

ifndef COMPILED_BY
  COMPILED_BY:="uknown <uknown@example.org>"
endif

ifndef INSTALL_DIR
  INSTALL_DIR:=/usr
endif

ifndef INSTALL_GENERIC_NAME
  INSTALL_GENERIC_NAME:=OFF
endif
ifndef ENABLE_TESTS
  ENABLE_TESTS:=ON
endif
ifndef TDD_MPI_NP
  TDD_MPI_NP:=2
endif
ifndef TDD_OMP_NP
  TDD_OMP_NP:=2
endif

#NOTE The plugin search path can be changed at runtime.
#     These set the defaults.
ifndef PLUGINS_STANDARD
  PLUGINS_STANDARD:=src/plugins
endif
ifndef PLUGINS_CONTRIBUTED
  PLUGINS_CONTRIBUTED:=extras/plugins
endif
ifndef PLUGINS_USER
  PLUGINS_USER:=external/plugins
endif

#TODO Auto-detect these if not set.
ifndef ENABLE_OMP
  ENABLE_OMP:=ON
endif
ifndef ENABLE_LIBNUMA
  ENABLE_LIBNUMA:=ON
endif

ifndef ENABLE_VALGRIND
  ifeq (,$(shell which valgrind 2>/dev/null))
    ENABLE_VALGRIND:=OFF
  else
    ENABLE_VALGRIND:=ON
  endif
endif

# External packages
ifndef ENABLE_GOOGLETEST
  ENABLE_GOOGLETEST:=ON
endif
ifndef ENABLE_INTEL_MKL
  ENABLE_INTEL_MKL:=ON
endif

ifndef ENABLE_PYBIND
  ENABLE_PYBIND:=ON
endif
ifndef ENABLE_MPI
  ENABLE_MPI:=ON
endif
ifndef ENABLE_HDF5
  # Use parallel version if MPI available
  ENABLE_HDF5:=ON
endif
ifndef ENABLE_CGNS
  ENABLE_CGNS:=ON
  #NOTE use parallel version if MPI available
endif
ifndef ENABLE_PETSC
  # Requires MPI
  ENABLE_PETSC:=ON
endif
ifndef ENABLE_GMSH
  ENABLE_GMSH:=ON
endif
ifndef ENABLE_GMSH_OPENMP
  ENABLE_GMSH_OPENMP:=ON
endif
ifndef ENABLE_FLTK
  # for Gmsh
  ENABLE_FLTK:=OFF
endif

#TODO Add these external packages
ifndef ENABLE_HYPERCOMPLEX
  ENABLE_HYPERCOMPLEX:=OFF
endif
ifndef ENABLE_METIS
  ENABLE_METIS:=OFF
  #NOTE use ParMETIS if MPI is available
endif
ifndef ENABLE_GPU
  ENABLE_GPU:=OFF
endif
