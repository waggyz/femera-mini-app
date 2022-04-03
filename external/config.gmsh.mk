#!/usr/bin/make
# Gmsh and its dependencies
ifeq ($(ENABLE_GMSH),ON)
  FMRFLAGS += -DFMR_HAS_GMSH
  LDLIBS += -lgmsh
#  ifeq ("$(CXX) $(CXX_VERSION)","g++ 4.8.5") # g+= or mpic++
  ifeq ("$(CXX_VERSION)","4.8.5")
    LIST_EXTERNAL += gmsh471
    GMSH_FLAGFILE:=$(BUILD_CPU)/external/install-gmsh471.flags
    FMRFLAGS += -DFMR_HAS_GMSH_GCC48_PATCH
  else
    LIST_EXTERNAL += gmsh
    GMSH_FLAGFILE:=$(BUILD_CPU)/external/install-gmsh.flags
  endif
  EXTERNAL_DOT+="Gmsh" -> "Femera"\n
  GMSH_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  GMSH_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_CPU)"
  GMSH_FLAGS += -DENABLE_BUILD_LIB=1
  GMSH_FLAGS += -DENABLE_BUILD_SHARED=1
  GMSH_FLAGS += -DENABLE_BUILD_DYNAMIC=1
  ifeq ($(ENABLE_GMSH_OMP),ON)
    GMSH_FLAGS += -DENABLE_OPENMP=1
    EXTERNAL_DOT+="OpenMP" -> "Gmsh"\n
  endif
  ifeq ($(ENABLE_PYBIND11),ON)
    # GMSH_REQUIRES += pybind11
    GMSH_DEPS += $(BUILD_DIR)/external/install-pybind11.out
    EXTERNAL_DOT+="pybind11" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_WRAP_PYTHON=1 -DENABLE_NUMPY=1
  endif
  ifeq ($(ENABLE_CGNS),ON)
    GMSH_REQUIRES += cgns
    EXTERNAL_DOT+="CGNS" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_CGNS=1
    GMSH_FLAGS += -DENABLE_CGNS_CPEX0045=0
  endif
  ifeq (0,1)# Disable experimental MPI in Gmsh for now.
    ifeq ($(ENABLE_MPI),ON)
      EXTERNAL_DOT+="MPI" -> "Gmsh"\n
      GMSH_FLAGS += -DENABLE_MPI=1
    endif
  endif
  ifeq (1,0) # Disable PETSc in Gmsh
  ifeq ($(ENABLE_PETSC),ON)
    GMSH_REQUIRES += petsc
    EXTERNAL_DOT+="PETSc" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_PETSC=1
    ifeq ($(ENABLE_PYBIND11),ON)
      GMSH_FLAGS += -DENABLE_PETSC4PY=1
    endif
    GMSH_FLAGS += -DENABLE_MPI=0
  endif
  endif
  ifeq ($(ENABLE_FLTK),ON)
    GMSH_REQUIRES += fltk
    EXTERNAL_DOT+="FLTK" -> "Gmsh"\n
    EXTERNAL_DOT+="X" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_FLTK=1
    #TODO It looks like native off-screen rendering does not work.
    #rhel7: sudo yum install mesa-libOSMesa-devel
    GMSH_FLAGS += -DENABLE_OSMESA=1 -DENABLE_GRAPHICS=1
  else
    GMSH_FLAGS += -DENABLE_FLTK=0
  endif
  ifeq ($(ENABLE_OCCT),ON)
    GMSH_REQUIRES += occt
    EXTERNAL_DOT+="OpenCASCADE" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_OCC=1 -DENABLE_OCC_CAF=1 -DENABLE_OCC_STATIC=1
  endif
  # Disable Cairo fonts for now. Just use FreeType (required by OCCT).
  GMSH_FLAGS += -DENABLE_CAIRO=0
  GMSH_DEPS+=$(patsubst %,$(BUILD_CPU)/external/install-%.out,$(GMSH_REQUIRES))
#  $(shell printf "%s" "$(gmsh_FLAGS)" > $(GMSH_FLAGFILE))
endif
ifeq ($(ENABLE_OCCT),ON)
  LIST_EXTERNAL += occt
  OCCT_REQUIRES += freetype
  EXTERNAL_DOT+="FreeType" -> "OpenCASCADE"\n
  ENABLE_FREETYPE:=ON
  OCCT_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  OCCT_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_CPU)"
  OCCT_FLAGS += -DBUILD_LIBRARY_TYPE=Static
  OCCT_FLAGS += -DCMAKE_BUILD_TYPE=Release
  OCCT_FLAGS += -DBUILD_MODULE_Draw=0
  OCCT_FLAGS += -DBUILD_MODULE_Visualization=0
  OCCT_FLAGS += -DBUILD_MODULE_ApplicationFramework=0
  OCCT_DEPS:=$(patsubst %,$(BUILD_CPU)/external/install-%.out,$(OCCT_REQUIRES))
  OCCT_FLAGFILE := $(BUILD_CPU)/external/install-occt.flags
endif
#FIXME CGNS, FLTK, and FreeType build in external/*/
ifeq ($(ENABLE_FREETYPE),ON)
  LIST_EXTERNAL += freetype
  FREETYPE_FLAGS += --prefix="$(INSTALL_CPU)"
  FREETYPE_FLAGFILE := $(BUILD_CPU)/external/install-freetype.flags
endif
ifeq ($(ENABLE_FLTK),ON)
  LIST_EXTERNAL += fltk
  FLTK_FLAGS += --prefix="$(INSTALL_CPU)"
  FLTK_FLAGS += --enable-shared
  FLTK_FLAGFILE := $(BUILD_CPU)/external/install-fltk.flags
endif

ifeq ($(ENABLE_GMSH),ON)

.PHONY: $(GMSH_FLAGFILE).new

external-flags: $(GMSH_FLAGFILE).new

$(BUILD_CPU)/external/install-gmsh.out : | $(GMSH_DEPS)

$(BUILD_CPU)/external/install-gmsh471.out : | $(GMSH_DEPS)

$(GMSH_FLAGFILE).new: external/config.gmsh.mk
	printf "%s" '$(GMSH_FLAGS)' > $(@)
endif

ifeq ($(ENABLE_OCCT),ON)

.PHONY: $(OCCT_FLAGFILE).new

external-flags: $(OCCT_FLAGFILE).new

$(BUILD_CPU)/external/install-occt.out : $(OCCT_DEPS)

$(OCCT_FLAGFILE).new:
	printf "%s" '$(OCCT_FLAGS)' > $(@)
endif

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
