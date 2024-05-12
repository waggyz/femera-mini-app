#!/usr/bin/make
# Gmsh and its dependencies
ifeq ($(ENABLE_GMSH),ON)
  FMRFLAGS += -DFMR_HAS_GMSH
  FMRFLAGS += -DFMR_GMSH_HAS_METIS
#  ifeq ($(USE_STATIC_LIBS),ON)
#    LDLIBS += $(INSTALL_CPU)/lib64/libgmsh.a
#  else
#TODO fix gmsh dependency linking errors to use static gmsh library
    LDLIBS += -lgmsh
#  endif
#  ifeq ("$(CXX) $(CXX_VERSION)","g++ 4.8.5") # g+= or mpic++
  ifeq ("$(CXX_VERSION)","4.8.5")
    LIST_EXTERNAL += gmsh471
    GMSH_FLAGFILE:=$(BUILD_CPU)/external/gmsh471-install.flags
    FMRFLAGS += -DFMR_HAS_GMSH_GCC48_PATCH
  else
    LIST_EXTERNAL += gmsh
    GMSH_FLAGFILE:=$(BUILD_CPU)/external/gmsh-install.flags
  endif
  EXTERNAL_DOT+="Femera" -> "Gmsh" [color="cyan"]\n
  EXTERNAL_DOT+="Gmsh" -> "libjpeg"\n
  EXTERNAL_DOT+="Gmsh" -> "libpng"\n
  EXTERNAL_DOT+="Gmsh" -> "METIS" [color="green"]\n
  # EXTERNAL_DOT+="Gmsh" -> "giflib"\n
  # [color="red"]
  GMSH_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  GMSH_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_CPU)"
  GMSH_FLAGS += -DENABLE_BUILD_LIB=ON
  GMSH_FLAGS += -DENABLE_BUILD_SHARED=ON
  GMSH_FLAGS += -DENABLE_BUILD_DYNAMIC=ON
  GMSH_FLAGS += -DENABLE_PRIVATE_API=ON
  #TODO Check the next
  GMSH_FLAGS += -DENABLE_ZIPPER=ON
  ifeq ($(ENABLE_GMSH_OMP),ON)
    GMSH_FLAGS += -DENABLE_OPENMP=ON
    EXTERNAL_DOT+="Gmsh" -> "OpenMP"\n
  endif
  ifeq ($(ENABLE_PYBIND11),ON)
    GMSH_REQUIRES += pybind11
    GMSH_DEPS += $(BUILD_DIR)/external/pybind11-install.out
    EXTERNAL_DOT+="Gmsh" -> "pybind11"\n
    EXTERNAL_DOT+="Gmsh" -> "numpy"\n
    GMSH_FLAGS += -DENABLE_WRAP_PYTHON=ON -DENABLE_NUMPY=ON
  endif
  ifeq ($(ENABLE_CGNS),ON)
    GMSH_REQUIRES += cgns
    EXTERNAL_DOT+="Gmsh" -> "CGNS"\n
    GMSH_FLAGS += -DENABLE_CGNS=ON
    GMSH_FLAGS += -DENABLE_CGNS_CPEX0045=OFF
  endif
  ifeq (0,1)# Disable experimental MPI in Gmsh for now.
    ifeq ($(ENABLE_MPI),ON)
      EXTERNAL_DOT+="Gmsh" -> "MPI"\n
      GMSH_FLAGS += -DENABLE_MPI=ON
    endif
  endif
  ifeq (1,0) # Disable PETSc in Gmsh for now
  ifeq ($(ENABLE_PETSC),ON)
    GMSH_REQUIRES += petsc
    EXTERNAL_DOT+="Gmsh" -> "PETSc"\n
    GMSH_FLAGS += -DENABLE_PETSC=ON
    ifeq ($(ENABLE_PYBIND11),ON)
      GMSH_FLAGS += -DENABLE_PETSC4PY=ON
    endif
    GMSH_FLAGS += -DENABLE_MPI=OFF
  endif
  endif
  ifeq ($(ENABLE_FLTK),ON)
    GMSH_REQUIRES += fltk
    EXTERNAL_DOT+="Gmsh" -> "X"\n
    EXTERNAL_DOT+="Gmsh" -> "FLTK"\n
    EXTERNAL_DOT+="FLTK" -> "OpenGL"\n
    EXTERNAL_DOT+="Femera" -> "FLTK" [color="cyan"]\n
    GMSH_FLAGS += -DENABLE_FLTK=ON
    #TODO It looks like native gmsh off-screen rendering does not work.
    #rhel7: sudo yum install mesa-libOSMesa-devel
    #GMSH_FLAGS += -DENABLE_OSMESA=ON -DENABLE_GRAPHICS=ON
  else
    GMSH_FLAGS += -DENABLE_FLTK=OFF
  endif
  ifeq ($(ENABLE_OCCT),ON)
#    GMSH_REQUIRES += occt
    EXTERNAL_DOT+="Gmsh" -> "OpenCASCADE"\n
    GMSH_FLAGS += -DENABLE_OCC=ON -DENABLE_OCC_CAF=ON -DENABLE_OCC_STATIC=ON
  endif
  ifeq ($(ENABLE_CAIRO),ON)
    # Can just use FreeType (required by OCCT).
    EXTERNAL_DOT+="Gmsh" -> "Cairo"\n
    GMSH_FLAGS += -DENABLE_CAIRO=ON
    GMSH_DEPS+=$(patsubst %,$(BUILD_CPU)/external/%-install.out,$(GMSH_REQUIRES))
  endif
  #  $(shell printf "%s" "$(gmsh_FLAGS)" > $(GMSH_FLAGFILE))
endif
ifeq ($(ENABLE_OCCT),ON)
  FMRFLAGS += -DFMR_HAS_OCC
#  LIST_EXTERNAL += occt
  OCCT_REQUIRES += freetype
  EXTERNAL_DOT+="OpenCASCADE" -> "FreeType"\n
  EXTERNAL_DOT+="Femera" -> "FreeType" [color="cyan"]\n
  ENABLE_FREETYPE:=ON
#  OCCT_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
#  OCCT_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_CPU)"
#  OCCT_FLAGS += -DBUILD_LIBRARY_TYPE=Static
#  OCCT_FLAGS += -DCMAKE_BUILD_TYPE=Release
#  OCCT_FLAGS += -DBUILD_MODULE_Draw=OFF
#  OCCT_FLAGS += -DBUILD_MODULE_Visualization=OFF
#  OCCT_FLAGS += -DBUILD_MODULE_ApplicationFramework=OFF
#  OCCT_DEPS:=$(patsubst %,$(BUILD_CPU)/external/%-install.out,$(OCCT_REQUIRES))
#  OCCT_FLAGFILE := $(BUILD_CPU)/external/occt-install.flags
endif
#FIXME CGNS, FLTK, and FreeType build in external/*/
ifeq ($(ENABLE_FREETYPE),ON)
  LIST_EXTERNAL += freetype
  FREETYPE_FLAGS += --prefix="$(INSTALL_CPU)"
  FREETYPE_FLAGFILE := $(BUILD_CPU)/external/freetype-install.flags
endif
ifeq ($(ENABLE_FLTK),ON)
  LIST_EXTERNAL += fltk
  FLTK_FLAGS += --prefix="$(INSTALL_CPU)"
  FLTK_FLAGS += --enable-shared
  FLTK_FLAGFILE := $(BUILD_CPU)/external/fltk-install.flags
endif

ifeq ($(ENABLE_GMSH),ON)

.PHONY: $(GMSH_FLAGFILE).new

external-flags: $(GMSH_FLAGFILE).new

$(BUILD_CPU)/external/gmsh-install.out : | $(GMSH_DEPS)

$(BUILD_CPU)/external/gmsh471-install.out : | $(GMSH_DEPS)

$(GMSH_FLAGFILE).new: external/gmsh-config.mk
	printf "%s" '$(GMSH_FLAGS)' > $(@)
endif

#ifeq ($(ENABLE_OCCT),ON)
#
#.PHONY: $(OCCT_FLAGFILE).new
#
#external-flags: $(OCCT_FLAGFILE).new
#
#$(BUILD_CPU)/external/occt-install.out : $(OCCT_DEPS)
#
#$(OCCT_FLAGFILE).new:
#	printf "%s" '$(OCCT_FLAGS)' > $(@)
#endif

ifeq ($(ENABLE_FREETYPE),ON)

.PHONY: $(FREETYPE_FLAGFILE).new

external-flags: $(FREETYPE_FLAGFILE).new

$(FREETYPE_FLAGFILE).new:
	printf "%s" '$(FREETYPE_FLAGS)' > $(@)
endif

ifeq ($(ENABLE_FLTK),ON)
external-flags: $(FLTK_FLAGFILE).new

$(FLTK_FLAGFILE).new:
	printf "%s" '$(FLTK_FLAGS)' > $(@)
endif
