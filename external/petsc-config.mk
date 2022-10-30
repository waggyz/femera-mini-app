#!/usr/bin/make
ifeq ($(ENABLE_PETSC),ON)
  FMRFLAGS += -DFMR_HAS_PETSC
  LDLIBS += -lpetsc -ldl
  LIST_EXTERNAL += petsc
  EXTERNAL_DOT+="Femera" -> "PETSc"\n
  
  PETSC_FLAGS += PETSC_ARCH=$(CPUMODEL)
  PETSC_FLAGS += --prefix=$(INSTALL_CPU)
  
  # Build static libs
  PETSC_FLAGS += --with-shared-libraries=0
  PETSC_FLAGS += --with-scalar-type=complex
  
  ifeq ($(ENABLE_PETSC_DEBUG),ON)
    PETSC_FLAGS += --with-debugging
  else
    PETSC_FLAGS += --with-debugging=0
  endif
  
  PETSC_FLAGS += --with-memalign=32
  #TODO match Femera (FMR_ALIGN_VALS): <4,8,16,32,64>
  
  #-O3 -march=native -mtune=native'
  PETSC_CFLAGS:= $(filter-out -fvisibility-inlines-hidden,$(OPTFLAGS))
  PETSC_FLAGS += COPTFLAGS='$(PETSC_CFLAGS)'
  
  PETSC_FFLAGS:= $(filter-out -fvisibility-inlines-hidden,$(OPTFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-sin,$(PETSC_FFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-cos,$(PETSC_FFLAGS))
  PETSC_FLAGS += FOPTFLAGS='$(PETSC_FFLAGS)'
  
  PETSC_FLAGS += CXXOPTFLAGS='$(OPTFLAGS)'

  ifeq ($(ENABLE_PETSC_OMP),ON)
    EXTERNAL_DOT+="PETSc" -> "OpenMP"\n
    PETSC_FLAGS += --with-openmp
  endif
  #TODO switch to mpich?
  ifeq ($(ENABLE_MPI),ON)
    EXTERNAL_DOT+="PETSc" -> "MPI"\n
    ifeq ($(HAS_MPI),ON)
      # PETSC_FLAGS += --with-mpi-dir="$(MPI_DIR)"
      PETSC_FLAGS += --with-mpi
    else
      #FIXME PETSC_FLAGS += --download-openmpi
      PETSC_FLAGS += --with-mpi
    endif
  else
    PETSC_FLAGS += --with-mpi=0
  endif
  ifeq ($(ENABLE_MKL),ON)
    # needed by petsc-install.sh not -config.sh:
    PETSC_REQUIRES += mkl
    LDLIBS += -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -lm
    #-liomp5
    # already added: LDLIBS += -lpthread
    EXTERNAL_DOT+="PETSc" -> "MKL"\n
    PETSC_FLAGS += --with-blaslapack-dir=$(INSTALL_CPU)/mkl/latest
  else
    #TODO check for blas, and download one if not available
    #PETSC_FLAGS += --download-fblaslapack=1
  endif
  ifeq ($(ENABLE_CGNS),ON)
    #NOTE PETSc does not recognize high order branch
    EXTERNAL_DOT+="PETSc" -> "CGNS"\n
    PETSC_REQUIRES += cgns
    PETSC_MAKE_FLAGS += --with-cgns-dir=$(INSTALL_CPU)
  endif
  ifeq ($(ENABLE_GMSH),ON)
    EXTERNAL_DOT+="PETSc" -> "Gmsh"\n
    ifeq ($(ENABLE_GMSH_METIS),ON)
      EXTERNAL_DOT+="Gmsh" -> "METIS"\n
      ENABLE_PETSC_METIS:=ON
    endif
    PETSC_REQUIRES += gmsh
    PETSC_MAKE_FLAGS += --with-gmsh-dir=$(INSTALL_CPU)
  endif
  ifeq ($(ENABLE_PETSC_X),ON)
    EXTERNAL_DOT+="PETSc" -> "X"\n
    PETSC_FLAGS += --with-x
    LDLIBS += -lX11
  endif
  ifeq ($(ENABLE_PETSC_CUDA),ON)
    EXTERNAL_DOT+="PETSc" -> "CUDA"\n
    PETSC_FLAGS += --with-cuda
  endif
  # PETSc installs the rest ===================================================
  #
  # PETSC_FLAGS += --with-packages-build-dir="$(BUILD_CPU)"# no worky
  #
  ifeq ($(ENABLE_PETSC_MPI4PY),ON)
    EXTERNAL_DOT+="PETSc" -> "mpi4py"\n
    EXTERNAL_DOT+="mpi4py" -> "MPI"\n
    EXTERNAL_DOT+="mpi4py" -> "Python"\n
    PETSC_FLAGS += --download-mpi4py
  endif
  ifeq ($(ENABLE_PETSC4PY),ON)
    # petsc4py requires numpy and (optional but highly recommended) mpi4py 
    EXTERNAL_DOT+="PETSc" -> "petsc4py"\n
    EXTERNAL_DOT+="petsc4py" -> "numpy"\n
    EXTERNAL_DOT+="numpy" -> "Python"\n
    PETSC_FLAGS += --with-petsc4py
  endif
  ifeq ($(ENABLE_PETSC_HWLOC),ON)
    #TODO use hwloc instead of libnuma?
    EXTERNAL_DOT+="PETSc" -> "hwloc"\n
    EXTERNAL_DOT+="Femera" -> "hwloc"\n
    PETSC_INSTALLS += hwloc
    PETSC_FLAGS += --download-hwloc
  endif
  ifeq ($(ENABLE_PETSC_ZIPLIBS),ON)
    PETSC_INSTALLS += zlib szlib
    PETSC_FLAGS += --download-zlib --download-szlib
    EXTERNAL_DOT+="PETSc" -> "zlib"\n
    EXTERNAL_DOT+="PETSc" -> "szlib"\n
  endif
  ifeq ($(ENABLE_HDF5),ON)
    EXTERNAL_DOT+="HDF5" -> "zlib"\n
    EXTERNAL_DOT+="HDF5" -> "szlib"\n
  endif
  ifeq ($(ENABLE_PETSC_IMAGELIBS),ON)
    PETSC_INSTALLS += libjpeg libpng giflib
    PETSC_FLAGS += --download-libjpeg --download-libpng --download-giflib
    EXTERNAL_DOT+="PETSc" -> "libjpeg"\n
    EXTERNAL_DOT+="PETSc" -> "libpng"\n
    EXTERNAL_DOT+="PETSc" -> "giflib"\n
  endif
  ifeq ($(ENABLE_GMSH),ON)
    EXTERNAL_DOT+="Gmsh" -> "libjpeg"\n
    EXTERNAL_DOT+="Gmsh" -> "libpng"\n
    EXTERNAL_DOT+="Gmsh" -> "giflib"\n
  endif
  ifeq ($(ENABLE_OCCT),ON)
    EXTERNAL_DOT+="PETSc" -> "OpenCASCADE"\n
    PETSC_INSTALLS += occt
    PETSC_FLAGS += --download-opencascade
  endif
  ifeq ($(ENABLE_GOOGLETEST),ON)
    EXTERNAL_DOT+="PETSc" -> "GoogleTest"\n
    EXTERNAL_DOT+="PETSc" -> "Boost"\n
    PETSC_INSTALLS += GoogleTest
    PETSC_FLAGS += --download-googletest
    PETSC_FLAGS += --download-boost
  endif
  ifeq ($(ENABLE_HDF5),ON)
    EXTERNAL_DOT+="PETSc" -> "HDF5"\n
    LDLIBS += -lhdf5
    PETSC_INSTALLS += hdf5
    PETSC_FLAGS += --download-hdf5
  endif
  ifeq ($(ENABLE_PETSC_MOAB),ON)
    EXTERNAL_DOT+="PETSc" -> "MOAB"\n
    ifeq ($(ENABLE_HDF5),ON)
      EXTERNAL_DOT+="MOAB" -> "HDF5"\n
    endif
    PETSC_INSTALLS += moab
    PETSC_FLAGS += --download-moab
  endif
  ifeq ($(ENABLE_PETSC_EXODUSII),ON)
    EXTERNAL_DOT+="PETSc" -> "EXODUS II"\n
    PETSC_INSTALLS += exodusii
    PETSC_FLAGS += --download-exodusii
  endif
  ifeq ($(ENABLE_PETSC_CHACO),ON)
    EXTERNAL_DOT+="PETSc" -> "Chaco"\n
    PETSC_INSTALLS += chaco
    PETSC_FLAGS += --download-chaco
  endif
  ifeq ($(ENABLE_PETSC_SCOTCH),ON)
    EXTERNAL_DOT+="PETSc" -> "PTScotch"\n
    PETSC_INSTALLS += ptscotch
    PETSC_FLAGS += --download-ptscotch
  endif
  ifeq ($(ENABLE_PETSC_METIS),ON)
    EXTERNAL_DOT+="PETSc" -> "METIS"\n
    PETSC_FLAGS += --download-metis
    PETSC_INSTALLS += metis
  endif
  ifeq ($(ENABLE_PETSC_PARMETIS),ON)
    EXTERNAL_DOT+="PETSc" -> "ParMETIS"\n
    EXTERNAL_DOT+="ParMETIS" -> "MPI"\n
    PETSC_INSTALLS += parmetis
    PETSC_FLAGS += --download-parmetis
  endif
  ifeq ($(ENABLE_PETSC_FFTW),ON)
    EXTERNAL_DOT+="PETSc" -> "fftw"\n
    PETSC_INSTALLS += fftw
    PETSC_FLAGS += --download-fftw
  endif
  ifeq ($(ENABLE_PETSC_ML),ON)
    # Trilonos/ML multilevel preconditioning
    EXTERNAL_DOT+="PETSc" -> "ml"\n
    PETSC_FLAGS += --download-ml
    PETSC_INSTALLS += ml
  endif
  ifeq (1,0)
  #****************************************************************************
  #TODO Check versions and have PETSc download if needed
  #PETSC_FLAGS += --download-make=1
  #PETSC_FLAGS += --download-cmake=1
  #PETSC_FLAGS += --download-git=1
  #
  # partitioners
  #PETSC_FLAGS += --download-party
  #
  # future expansion
  #PETSC_FLAGS += --download-moose
  #PETSC_FLAGS += --download-mumps
  #PETSC_FLAGS += --download-pnetcdf
  #PETSC_FLAGS += --download-libmesh
  #
  #****************************************************************************
  endif
  #
  PETSC_FLAGFILE:=$(BUILD_CPU)/external/petsc-config.flags
  PETSC_DEPS:=$(patsubst %,$(BUILD_CPU)/external/%-install.out,$(PETSC_REQUIRES))
  PETSC_MAKEFLAGFILE:=$(BUILD_CPU)/external/petsc-install.flags
endif
ifeq ($(ENABLE_PETSC),ON)

.PHONY: $(PETSC_FLAGFILE).new $(PETSC_MAKEFLAGFILE).new

$(BUILD_CPU)/external/petsc-install.out:| $(PETSC_DEPS) 
$(BUILD_CPU)/external/petsc-install.out:| $(BUILD_CPU)/external/petsc-config.out 

external-flags: $(PETSC_FLAGFILE).new $(PETSC_MAKEFLAGFILE).new

$(PETSC_FLAGFILE).new: external/petsc-config.mk
	printf "%s" "$(PETSC_FLAGS)" > $(@)

$(PETSC_MAKEFLAGFILE).new: external/petsc-config.mk
	printf "%s" "$(PETSC_FLAGS) $(PETSC_MAKE_FLAGS)" > $(@)

endif
