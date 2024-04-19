#!/usr/bin/make
ifeq ($(ENABLE_PETSC),ON)
  FMRFLAGS += -DFMR_HAS_PETSC
#  ifeq ($(USE_STATIC_LIBS),ON)
#    LDLIBS += $(INSTALL_CPU)/lib/libpetsc.a $(INSTALL_CPU)/lib/libdl.a
#  else
    LDLIBS += -lpetsc -ldl
#  endif
  LIST_EXTERNAL += petsc
  EXTERNAL_DOT+="Femera" -> "PETSc" [color="cyan"]\n
  EXTERNAL_DOT+="PETSc" -> "Bison" [color="blue"]\n
  EXTERNAL_DOT+="PETSc" -> "Sowing" [color="blue"]\n
  
  PETSC_FLAGS += PETSC_ARCH=$(CPUMODEL)
  PETSC_FLAGS += --prefix=$(INSTALL_CPU)
  PETSC_FLAGS += --with-packages-download-dir=$(FMRDIR)/external/
  PETSC_FLAGS += --with-packages-build-dir=$(BUILD_CPU)/external/
  
  # Build static libs
  PETSC_FLAGS += --with-shared-libraries=0
  # PETSC_FLAGS += --with-scalar-type=complex
  
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
  PETSC_FLAGS += CXXOPTFLAGS='$(OPTFLAGS)'
  
  PETSC_FFLAGS:= $(filter-out -fvisibility-inlines-hidden,$(OPTFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-sin,$(PETSC_FFLAGS))
  PETSC_FFLAGS:= $(filter-out -fno-builtin-cos,$(PETSC_FFLAGS))
  
  PETSC_FLAGS += FOPTFLAGS='$(PETSC_FFLAGS)'
  
  ifeq ($(ENABLE_PETSC_OMP),ON)
    EXTERNAL_DOT+="PETSc" -> "OpenMP"\n
    PETSC_FLAGS += --with-openmp
  endif
  #TODO switch to mpich?
  ifeq ($(ENABLE_MPI),ON)
    ifeq ($(ENABLE_PETSC_MPI),ON)
      EXTERNAL_DOT+="PETSc" -> "MPI" [color="blue"]\n
      FIXME PETSC_FLAGS += --download-openmpi
    else
      EXTERNAL_DOT+="PETSc" -> "MPI"\n
      PETSC_FLAGS += --with-mpi
      # EXTERNAL_DOT+="PETSc" -> "MPI" [color="blue";style="dashed"]\n
      # PETSC_FLAGS += --with-mpi-dir="$(MPI_DIR)"
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
    ifeq (TODO,ON)
      #TODO check for blas, and download one if not available
      # ENABLE_PETSC_BLASLAPACK:=ON
    endif
  endif
  ifeq ($(ENABLE_CGNS),ON)
    #NOTE PETSc does not recognize high order branch
    EXTERNAL_DOT+="PETSc" -> "CGNS" [color="blue"]\n
    PETSC_REQUIRES += cgns
    # PETSC_MAKE_FLAGS += --with-cgns-dir=$(INSTALL_CPU)
    PETSC_FLAGS += --download-cgns
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
  ifeq ($(ENABLE_PETSC_NVIDIA),ON)
    EXTERNAL_DOT+="PETSc" -> "CUDA"\n
    PETSC_FLAGS += --with-cuda
  endif
  # PETSc installs the rest ===================================================
  #
  # PETSC_FLAGS += --with-packages-build-dir="$(BUILD_CPU)"# no worky
  #
  ifeq ($(ENABLE_PETSC_BLASLAPACK),ON)
    EXTERNAL_DOT+="PETSc" -> "BLAS" [color="blue"]\n
    EXTERNAL_DOT+="PETSc" -> "LAPACK" [color="blue"]\n
    EXTERNAL_DOT+="LAPACK" -> "BLAS"\n
    # PETSC_REQUIRES += fblas
    # PETSC_REQUIRES += flapack
    # PETSC_FLAGS += --download-fblaslapack
    PETSC_REQUIRES += openblas
    PETSC_FLAGS += --download-openblas
  endif
  ifeq ($(ENABLE_PETSC_SUITESPARSE),ON)
    EXTERNAL_DOT+="PETSc" -> "SuiteSparse" [color="blue"]\n
    PETSC_FLAGS += --download-suitesparse
    PETSC_INSTALLS += suitesparse
  endif  
  ifeq ($(ENABLE_PETSC_MPI4PY),ON)
    EXTERNAL_DOT+="PETSc" -> "mpi4py" [color="blue"]\n
    EXTERNAL_DOT+="mpi4py" -> "MPI"\n
    EXTERNAL_DOT+="mpi4py" -> "Python"\n
    PETSC_FLAGS += --download-mpi4py
    ifeq ($(ENABLE_PYMERA),ON)
      EXTERNAL_DOT+="pymera" -> "mpi4py"\n
    endif
  endif
  ifeq ($(ENABLE_PETSC4PY),ON)
    # petsc4py requires numpy and (optional but highly recommended) mpi4py 
    EXTERNAL_DOT+="PETSc" -> "petsc4py" [color="blue"]\n
    EXTERNAL_DOT+="petsc4py" -> "numpy"\n
    EXTERNAL_DOT+="numpy" -> "Python"\n
    EXTERNAL_DOT+="Femera" -> "numpy" [color="cyan"]\n
    PETSC_FLAGS += --with-petsc4py
    ifeq ($(ENABLE_PYMERA),ON)
      EXTERNAL_DOT+="pymera" -> "numpy"\n
      EXTERNAL_DOT+="pymera" -> "petsc4py"\n
    endif
  endif
  ifeq ($(ENABLE_PETSC_HWLOC),ON)
    #TODO use hwloc instead of libnuma?
    EXTERNAL_DOT+="PETSc" -> "hwloc" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "hwloc" [style="dotted"]\n
    EXTERNAL_DOT+="hwloc" -> "libxml2"\n
    EXTERNAL_DOT+="libxml2" -> "Femera" [color="cyan"]\n
    PETSC_INSTALLS += hwloc
    PETSC_FLAGS += --download-hwloc
  endif
  ifeq ($(ENABLE_PETSC_ZIPLIBS),ON)
    PETSC_INSTALLS += zlib szlib
    PETSC_FLAGS += --download-zlib --download-szlib
    EXTERNAL_DOT+="PETSc" -> "zlib" [color="blue"]\n
    EXTERNAL_DOT+="PETSc" -> "szlib" [color="red"]\n
  endif
  ifeq ($(ENABLE_HDF5),ON)
    EXTERNAL_DOT+="HDF5" -> "zlib"\n
    EXTERNAL_DOT+="HDF5" -> "szlib"\n
    ifeq ($(ENABLE_MPI),ON)
      EXTERNAL_DOT+="HDF5" -> "MPI"\n
    endif
  endif
  ifeq ($(ENABLE_PETSC_IMAGELIBS),ON)
    PETSC_INSTALLS += libjpeg libpng giflib
    PETSC_FLAGS += --download-libjpeg --download-libpng --download-giflib
    EXTERNAL_DOT+="PETSc" -> "libjpeg" [color="blue"]\n
    EXTERNAL_DOT+="PETSc" -> "libpng" [color="blue"]\n
    EXTERNAL_DOT+="PETSc" -> "giflib" [color="red"]\n
  endif
  ifeq ($(ENABLE_OCCT),ON)
    EXTERNAL_DOT+="PETSc" -> "OpenCASCADE" [color="blue"]\n
    PETSC_INSTALLS += occt
    PETSC_FLAGS += --download-opencascade
  endif
  ifeq ($(ENABLE_PYBIND11),ON)
    EXTERNAL_DOT+="PETSc" -> "Boost" [color="blue"]\n
    PETSC_FLAGS += --download-boost
  endif
  ifeq ($(ENABLE_GOOGLETEST),ON)
    EXTERNAL_DOT+="PETSc" -> "GoogleTest" [color="blue"]\n
    PETSC_INSTALLS += GoogleTest
    PETSC_FLAGS += --download-googletest
  endif
  ifeq ($(ENABLE_HDF5),ON)
    EXTERNAL_DOT+="PETSc" -> "HDF5" [color="blue"]\n
#    ifeq ($(USE_STATIC_LIBS),ON)
#      LDLIBS += $(INSTALL_CPU)/lib/libhdf5_hl.a
#    else
      LDLIBS += -lhdf5
#    endif
    PETSC_INSTALLS += hdf5
    PETSC_FLAGS += --download-hdf5
  endif
  ifeq ($(ENABLE_PETSC_MOAB),ON)
    EXTERNAL_DOT+="PETSc" -> "MOAB" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "MOAB" [style="dotted"]\n
    ifeq ($(ENABLE_HDF5),ON)
      EXTERNAL_DOT+="MOAB" -> "HDF5"\n
    endif
    PETSC_INSTALLS += moab
    PETSC_FLAGS += --download-moab
  endif
  ifeq ($(ENABLE_PETSC_PNETCDF),ON)
    EXTERNAL_DOT+="PETSc" -> "PnetCDF" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "PnetCDF" [style="dotted"]\n
    PETSC_INSTALLS += pnetcdf
    PETSC_FLAGS += --download-pnetcdf
  endif
  ifeq ($(ENABLE_PETSC_EXODUSII),ON)
    EXTERNAL_DOT+="PETSc" -> "EXODUS II" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "EXODUS II" [style="dotted"]\n
    PETSC_INSTALLS += exodusii
    PETSC_FLAGS += --download-exodusii
  endif
  ifeq ($(ENABLE_PETSC_LIBMESH),ON)
    EXTERNAL_DOT+="PETSc" -> "libmesh" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "libmesh" [style="dotted"]\n
    PETSC_INSTALLS += libmesh
    PETSC_FLAGS += --download-libmesh
  endif
  ifeq ($(ENABLE_PETSC_CHACO),ON)
    EXTERNAL_DOT+="PETSc" -> "Chaco" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "Chaco" [style="dotted"]\n
    PETSC_INSTALLS += chaco
    PETSC_FLAGS += --download-chaco
  endif
  ifeq ($(ENABLE_PETSC_SCOTCH),ON)
    EXTERNAL_DOT+="PETSc" -> "PT-Scotch" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "PT-Scotch" [style="dotted"]\n
    PETSC_INSTALLS += ptscotch
    PETSC_FLAGS += --download-ptscotch
  endif
  ifeq ($(ENABLE_PETSC_METIS),ON)
    EXTERNAL_DOT+="PETSc" -> "METIS" [color="blue"]\n
    PETSC_FLAGS += --download-metis
    PETSC_INSTALLS += metis
  endif
  ifeq ($(ENABLE_PETSC_PARMETIS),ON)
    EXTERNAL_DOT+="PETSc" -> "ParMETIS" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "ParMETIS" [style="dotted"]\n
    EXTERNAL_DOT+="ParMETIS" -> "MPI"\n
    PETSC_INSTALLS += parmetis
    PETSC_FLAGS += --download-parmetis
  endif
  ifeq ($(ENABLE_PETSC_FFTW),ON)
    EXTERNAL_DOT+="PETSc" -> "FFTW" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "FFTW" [style="dotted"]\n
    PETSC_INSTALLS += fftw
    PETSC_FLAGS += --download-fftw
  endif
  ifeq ($(ENABLE_PETSC_ML),ON)
    # Trilonos/ML multilevel preconditioning
    EXTERNAL_DOT+="PETSc" -> "ml" [color="blue"]\n
    EXTERNAL_DOT+="Femera" -> "ml" [style="dotted"]\n
    PETSC_FLAGS += --download-ml
    PETSC_INSTALLS += ml
  endif
  ifeq ($(ENABLE_NEPER),ON)
    # GSL required by Neper
    EXTERNAL_DOT+="PETSc" -> "GSL" [color="blue"]\n
    EXTERNAL_DOT+="Neper" -> "GSL"\n
    PETSC_FLAGS += --download-gsl
    PETSC_INSTALLS += gsl
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
  # Direct solver
  #PETSC_FLAGS += --download-mumps
  #
  # PDE solver (FE/FD) packages
  #PETSC_FLAGS += --download-moose
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
