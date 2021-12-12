#!/usr/bin/make
ifeq ($(ENABLE_PETSC),ON)
  LIST_EXTERNAL += petsc
  EXT_DOT+="PETSc" -> "Femera"\n
  PETSC_FLAGS += PETSC_ARCH=$(CPUMODEL)
  #-O3 -march=native -mtune=native'
  PETSC_CFLAGS:= $(filter-out -fvisibility-inlines-hidden,$(OPTFLAGS))
  PETSC_FFLAGS:= $(filter-out -fvisibility-inlines-hidden,$(OPTFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-sin,$(PETSC_FFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-cos,$(PETSC_FFLAGS))
  
  PETSC_FLAGS += CXXOPTFLAGS='$(OPTFLAGS)'
  PETSC_FLAGS += COPTFLAGS='$(PETSC_CFLAGS)'
  PETSC_FLAGS += FOPTFLAGS='$(PETSC_FFLAGS)'
  PETSC_FLAGS += --prefix=$(INSTALL_CPU)
  # Build static libs
  PETSC_FLAGS += --with-shared-libraries=0
  # PETSC_FLAGS += --with-packages-build-dir="$(BUILD_CPU)"# no worky
  PETSC_FLAGS += --with-scalar-type=complex
  ifeq ($(ENABLE_PETSC_DEBUG),ON)
    PETSC_FLAGS += --with-debugging
  else
    PETSC_FLAGS += --with-debugging=0
  endif
  ifeq ($(ENABLE_PETSC_OMP),ON)
    EXT_DOT+="OpenMP" -> "PETSc"\n
    PETSC_FLAGS += --with-openmp
  endif
  ifeq ($(ENABLE_MPI),ON)
    EXT_DOT+="MPI" -> "PETSc"\n
    ifeq ($(HAS_MPI),ON)
      # PETSC_FLAGS += --with-mpi-dir="$(MPI_DIR)"
    else
      # PETSC_FLAGS += --download-openmpi
    endif
  else
    PETSC_FLAGS += --with-mpi=0
  endif
  ifeq ($(ENABLE_MKL),ON)
    PETSC_REQUIRES += mkl
    EXT_DOT+="MKL" -> "PETSc"\n
    PETSC_FLAGS += --with-blaslapack-dir=$(INSTALL_CPU)/mkl/latest
  endif
  ifeq (1,0) # Disable for now ------------------------------------------------
    ifeq ($(ENABLE_OCCT),ON)# Incompatible OpenCASCADE
      PETSC_REQUIRES += occt
      EXT_DOT+="OpenCASACADE" -> "PETSc"\n
      PETSC_FLAGS += --with-opencascade-dir=$(INSTALL_CPU)
    endif
    ifeq ($(ENABLE_CGNS),ON)# Does not recognize high order branch
      PETSC_REQUIRES += cgns
      EXT_DOT+="CGNS" -> "PETSc"\n
      EXT_DOT+="zlib" -> "PETSc"\n
      PETSC_FLAGS += --with-cgns-dir=$(INSTALL_CPU) --with-zlib
    endif
  endif # end disabled --------------------------------------------------------
  ifeq ($(ENABLE_GMSH),ON)
    # Require Gmsh so HDF5 will get built before both PETSc & Gmsh
    PETSC_REQUIRES += gmsh
    EXT_DOT+="Gmsh" -> "PETSc"\n
    PETSC_FLAGS += --with-gmsh-dir=$(INSTALL_CPU)
  endif
  ifeq ($(ENABLE_HDF5),ON)
    PETSC_REQUIRES += hdf5
    EXT_DOT+="HDF5" -> "PETSc"\n
    PETSC_FLAGS += --with-hdf5-dir=$(INSTALL_CPU)
  endif
  # PETSc installs the rest
  ifeq ($(ENABLE_PETSC_MOAB),ON)
    PETSC_INSTALLS += moab
    EXT_DOT+="MOAB" -> "PETSc"\n
    PETSC_FLAGS += --download-moab
  endif
  ifeq ($(ENABLE_PETSC_CHACO),ON)
    PETSC_INSTALLS += chaco
    EXT_DOT+="Chaco" -> "PETSc"\n
    PETSC_FLAGS += --download-chaco
  endif
  ifeq ($(ENABLE_PETSC_SCOTCH),ON)
    PETSC_INSTALLS += scotch
    EXT_DOT+="PTScotch" -> "PETSc"\n
    PETSC_FLAGS += --download-ptscotch
  endif
  ifeq ($(ENABLE_PETSC_PARMETIS),ON)
    PETSC_INSTALLS += parmetis
    EXT_DOT+="ParMETIS" -> "PETSc"\n
    EXT_DOT+="METIS" -> "ParMETIS"\n
    PETSC_FLAGS += --download-metis
    PETSC_FLAGS += --download-parmetis
  endif
  PETSC_DEPS:=$(patsubst %,$(BUILD_CPU)/external/install-%.out,$(PETSC_REQUIRES))
  PETSC_FLAGFILE:=$(BUILD_CPU)/external/install-petsc.flags
endif
ifeq ($(ENABLE_PETSC),ON)

.PHONY: $(PETSC_FLAGFILE).new

$(BUILD_CPU)/external/install-petsc.out : | $(PETSC_DEPS)

external-flags: $(PETSC_FLAGFILE).new

$(PETSC_FLAGFILE).new:
	printf "%s" "$(PETSC_FLAGS)" > $(@)


endif
