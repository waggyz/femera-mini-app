#!/usr/bin/make
ifeq ($(ENABLE_PETSC),ON)
  FMRFLAGS += -DFMR_HAS_PETSC
  LDLIBS += -lpetsc -ldl
  LIST_EXTERNAL += petsc
  EXTERNAL_DOT+="PETSc" -> "Femera"\n
  
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
    EXTERNAL_DOT+="OpenMP" -> "PETSc"\n
    PETSC_FLAGS += --with-openmp
  endif
  #TODO switch to mpich?
  ifeq ($(ENABLE_MPI),ON)
    EXTERNAL_DOT+="MPI" -> "PETSc"\n
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
    EXTERNAL_DOT+="MKL" -> "PETSc"\n
    PETSC_FLAGS += --with-blaslapack-dir=$(INSTALL_CPU)/mkl/latest
  else
    #TODO check for blas, and download one if not available
    #PETSC_FLAGS += --download-fblaslapack=1
  endif
  ifeq ($(ENABLE_CGNS),ON)# Does not recognize high order branch
    EXTERNAL_DOT+="CGNS" -> "PETSc"\n
   PETSC_REQUIRES += cgns
    PETSC_MAKE_FLAGS += --with-cgns-dir=$(INSTALL_CPU)
  endif
  ifeq ($(ENABLE_GMSH),ON)
    EXTERNAL_DOT+="Gmsh" -> "PETSc"\n
    ifeq ($(ENABLE_GMSH_METIS),ON)
      EXTERNAL_DOT+="METIS" -> "Gmsh"\n
      ENABLE_PETSC_METIS:=ON
    endif
    PETSC_REQUIRES += gmsh
    PETSC_MAKE_FLAGS += --with-gmsh-dir=$(INSTALL_CPU)
  endif
  
  # PETSC_FLAGS += --with-packages-build-dir="$(BUILD_CPU)"# no worky
  
    EXTERNAL_DOT+="petsc4py" -> "PETSc"\n
    PETSC_FLAGS += --with-petsc4py
    
    EXTERNAL_DOT+="X" -> "PETSc"\n
    PETSC_FLAGS += --with-x
    LDLIBS += -lX11
    
    # EXTERNAL_DOT+="CUDA" -> "PETSc"\n
    #TODO PETSC_FLAGS += --with-cuda=1
  
  # PETSc installs the rest ===================================================
  #
  ifeq ($(ENABLE_PETSC_HWLOC),ON)
    #TODO use hwloc instead of libnuma?
    EXTERNAL_DOT+="hwloc" -> "PETSc"\n
    EXTERNAL_DOT+="hwloc" -> "Femera"\n
    PETSC_INSTALLS += hwloc
    PETSC_FLAGS += --download-hwloc
  endif
  #
  #TODO condition to install zlib & szlib, below
  ifeq (ON,ON)  
    EXTERNAL_DOT+="zlib" -> "PETSc"\n
    PETSC_FLAGS += --download-zlib
    EXTERNAL_DOT+="szlib" -> "PETSc"\n
    PETSC_FLAGS += --download-szlib
    ifeq ($(ENABLE_HDF5),ON)
      EXTERNAL_DOT+="zlib" -> "HDF5"\n
      EXTERNAL_DOT+="szlib" -> "HDF5"\n
    endif
  endif
  #
  ifeq ($(ENABLE_OCCT),ON)
    EXTERNAL_DOT+="OpenCASCADE" -> "PETSc"\n
    PETSC_INSTALLS += occt
    PETSC_FLAGS += --download-opencascade
  endif
  ifeq ($(ENABLE_GOOGLETEST),ON)
#FIXME Change this so PETSc will download and install GoogleTest
    EXTERNAL_DOT+="GoogleTest" -> "PETSc"\n
    LDLIBS += -lgtest -lgmock
    PETSC_INSTALLS += GoogleTest
    PETSC_FLAGS += --download-googletest
  endif
  ifeq ($(ENABLE_HDF5),ON)
#FIXME Change this so PETSc will download and install HDF5
    EXTERNAL_DOT+="HDF5" -> "PETSc"\n
    LDLIBS += -lhdf5
    PETSC_INSTALLS += hdf5
    PETSC_FLAGS += --download-hdf5
  endif
  ifeq ($(ENABLE_PETSC_MOAB),ON)
    EXTERNAL_DOT+="MOAB" -> "PETSc"\n
    ifeq ($(ENABLE_HDF5),ON)
      EXTERNAL_DOT+="HDF5" -> "MOAB"\n
    endif
    PETSC_INSTALLS += moab
    PETSC_FLAGS += --download-moab
  endif
  ifeq ($(ENABLE_PETSC_EXODUSII),ON)
    EXTERNAL_DOT+="EXODUS II" -> "PETSc"\n
    PETSC_INSTALLS += exodusii
    PETSC_FLAGS += --download-exodusii
  endif
  ifeq ($(ENABLE_PETSC_CHACO),ON)
    EXTERNAL_DOT+="Chaco" -> "PETSc"\n
    PETSC_INSTALLS += chaco
    PETSC_FLAGS += --download-chaco
  endif
  ifeq ($(ENABLE_PETSC_SCOTCH),ON)
    EXTERNAL_DOT+="PTScotch" -> "PETSc"\n
    PETSC_INSTALLS += scotch
    PETSC_FLAGS += --download-ptscotch
  endif
  ifeq ($(ENABLE_PETSC_METIS),ON)
    EXTERNAL_DOT+="METIS" -> "PETSc"\n
    PETSC_FLAGS += --download-metis
    PETSC_INSTALLS += metis
  endif
  ifeq ($(ENABLE_PETSC_PARMETIS),ON)
    EXTERNAL_DOT+="ParMETIS" -> "PETSc"\n
    EXTERNAL_DOT+="MPI" -> "ParMETIS"\n
    PETSC_INSTALLS += parmetis
    PETSC_FLAGS += --download-parmetis
  endif
  ifeq (1,0)
  #****************************************************************************
  #TODO Check versions and have PETSc download if needed
  #PETSC_FLAGS += --download-make=1
  #PETSC_FLAGS += --download-cmake=1
  #PETSC_FLAGS += --download-git=1
  PETSC_FLAGS += --download-libjpeg
  PETSC_FLAGS += --download-libpng
  PETSC_FLAGS += --download-giflib
  #
  # needed by other external packages
  PETSC_FLAGS += --download-googletest
  PETSC_FLAGS += --download-boost
  #
  # partitioners
  #PETSC_FLAGS += --download-party
  #
  # future expansion
  PETSC_FLAGS += --download-ml
  PETSC_FLAGS += --download-fftw
  PETSC_FLAGS += --download-mpi4py
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
