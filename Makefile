#!/usr/bin/make
SHELL:= bash
FMRDIR:=$(shell pwd)

include examples/config.recommended
-include config.local
include tools/set-undefined.mk

include tools/functions.mk
#------------------------------------------------------------------------------
CXX_VERSION   := $(shell $(CXX) -dumpversion)
BUILD_VERSION := $(shell tools/build-version-number.sh)
FEMERA_VERSION:= Femera $(BUILD_VERSION)

BUILT_BY_EMAIL:= $(call parse_email,$(BUILT_BY))

IS_IN_REPO:= $(shell git rev-parse --is-inside-work-tree 2>/dev/null)
# contains "true" or "".

# Check if the user has altered content in src/ data/, tools/,
# or some content in extras/, even when not a git repository.
#FIXME Move to recipe?
REPO_MD5:= $(shell cat .md5)
ifeq ($(shell test -e file_name && echo -n yes),yes)
  #(shell tools/md5-all.sh build/.md5)
  $(shell echo "host hash" > build/.md5)
  HOST_MD5:= $(shell cat build/.md5)
endif
# Directories -----------------------------------------------------------------
# Directories for generic components
BUILD_DIR  := build
STAGE_DIR  := $(BUILD_DIR)/stage
INSTALL_DIR:= $(PREFIX)

# Directories for CPU-model specific components
BUILD_CPU  := $(BUILD_DIR)/$(CPUMODEL)
STAGE_CPU  := $(STAGE_DIR)/$(CPUMODEL)
INSTALL_CPU:= $(INSTALL_DIR)/$(CPUMODEL)

# Subdirectories needed
BUILD_TREE+= $(BUILD_DIR)/external/ $(BUILD_DIR)/docs/
BUILD_TREE+= $(BUILD_DIR)/external/tools/
BUILD_TREE+= $(BUILD_CPU)/tests/ $(BUILD_CPU)/tools/

STAGE_TREE+= $(STAGE_DIR)/bin/ $(STAGE_DIR)/libs/
STAGE_TREE+= $(STAGE_CPU)/bin/ $(STAGE_CPU)/libs/

# INSTALL_TREE:= $(patsubst $(STAGE_DIR)%,$(INSTALL_DIR)%,$(STAGE_TREE))

# Libraries and applications available ----------------------------------------
EXT_DOT:=digraph "external dependencies" {\n
EXT_DOT+=overlap=scale;\n
EXT_DOT+=size="6,3";\n
EXT_DOT+=ratio="fill";\n
EXT_DOT+=fontsize="12";\n
EXT_DOT+=fontname="Helvetica";\n
EXT_DOT+=clusterrank="local";\n

MAKE_DOT:=digraph "external dependencies" {\n
MAKE_DOT+=overlap=scale;\n
MAKE_DOT+=size="6,3";\n
MAKE_DOT+=ratio="fill";\n
MAKE_DOT+=fontsize="12";\n
MAKE_DOT+=fontname="Helvetica";\n
MAKE_DOT+=clusterrank="local";\n
ifeq ($(shell which dot 2>/dev/null),"")# dot is part of graphviz
  ENABLE_DOT:=OFF
else
  ENABLE_DOT:=ON
  MAKE_DOT+="dot" -> "Makefile"\n
endif
ifeq ($(ENABLE_OMP),ON)
  EXT_DOT+="OpenMP" -> "Femera"\n
endif
ifeq ($(ENABLE_MPI),ON)
  EXT_DOT+="MPI" -> "Femera"\n
endif
ifeq ($(ENABLE_LIBNUMA),ON)
  EXT_DOT+="libnuma" -> "Femera"\n
  FMRFLAGS += -DFMR_HAS_LIBNUMA
  LDLIBS += -lnuma
endif
# External code to build and install ------------------------------------------
ifeq ($(ENABLE_BATS),ON)
  # LIST_EXTERNAL +=
  # EXT_DOT+={ rank = sink; "Bats"; }\n
  MAKE_DOT+="Bats" -> "Makefile"\n
  BATS_MODS:= bats-core bats-support bats-assert bats-file
  label_bats = $(call label_test,$(1),$(2),$(3),$(4))
else
  label_bats = $(info \
    $(DBUG) Set ENABLE_BATS:=ON in config.local for $(notdir $(3)).)
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  EXT_DOT+="GoogleTest" -> "Femera"\n
  MAKE_DOT+="GoogleTest" -> "Makefile"\n
  LIST_EXTERNAL += googletest
  GOOGLETEST_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  $(shell printf "%s" "$(GOOGLETEST_FLAGS)" \
    > $(BUILD_DIR)/external/googletest.flags.new)
endif
# ENABLE_PYBIND11=ON
ifeq ($(ENABLE_PYBIND11),ON)
  LIST_EXTERNAL += pybind11
  EXT_DOT+="pybind11" -> "Femera"\n
  # FMRFLAGS += -DFMR_HAS_PYBIND11
  PYBIND11_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  PYBIND11_FLAGS += -DBOOST_ROOT:PATHNAME=/usr/local/pkgs-modules/boost_1.66.0
  PYBIND11_FLAGS += -DDOWNLOAD_CATCH=0
  $(shell printf "%s" "$(PYBIND11_FLAGS)" \
    > $(BUILD_DIR)/external/pybind11.flags.new)
endif
ifeq ($(ENABLE_GMSH),ON)
  ifeq ("$(CXX) $(CXX_VERSION)","g++ 4.8.5")
    LIST_EXTERNAL += gmsh471
  else
    LIST_EXTERNAL += gmsh
  endif
  EXT_DOT+="Gmsh" -> "Femera"\n
  GMSH_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  GMSH_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_DIR)"
  GMSH_FLAGS += -DENABLE_BUILD_LIB=1
  GMSH_FLAGS += -DENABLE_BUILD_SHARED=1
  GMSH_FLAGS += -DENABLE_BUILD_DYNAMIC=1
  ifeq ($(ENABLE_OMP),ON)
    GMSH_FLAGS += -DENABLE_OPENMP=1
  endif
  ifeq ($(ENABLE_PYBIND11),ON)
    GMSH_REQUIRES += pybind11
    EXT_DOT+="pybind11" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_WRAP_PYTHON=1 -DENABLE_NUMPY=1
  endif
  ifeq ($(ENABLE_CGNS),ON)
    GMSH_REQUIRES += CGNS hdf5
    EXT_DOT+="CGNS" -> "Gmsh"\n
    EXT_DOT+="HDF5" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_CGNS=1
    GMSH_FLAGS += -DENABLE_CGNS_CPEX0045=0
  endif
  ifeq ($(ENABLE_PETSC),ON)
    GMSH_REQUIRES += petsc
    EXT_DOT+="PETSc" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_PETSC=1
    ifeq ($(ENABLE_PYBIND11),ON)
      GMSH_FLAGS += -DENABLE_PETSC4PY=1
    endif
    GMSH_FLAGS += -DENABLE_MPI=1
  else
    # Disable MPI in Gmsh unless PETSc is used. It is probably not needed.
    GMSH_FLAGS += -DENABLE_MPI=0
  endif
  ifeq ($(ENABLE_FLTK),ON)
    GMSH_REQUIRES += fltk
    EXT_DOT+="FLTK" -> "Gmsh"\n
    EXT_DOT+="X" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_FLTK=1
    #TODO It looks like native off-screen rendering does not work.
    #rhel7: sudo yum install mesa-libOSMesa-devel
    GMSH_FLAGS += -DENABLE_OSMESA=1 -DENABLE_GRAPHICS=1
  else
    GMSH_FLAGS += -DENABLE_FLTK=0
  endif
  ifeq ($(ENABLE_OCCT),ON)
    GMSH_REQUIRES += occt
    EXT_DOT+="OpenCASCADE" -> "Gmsh"\n
    GMSH_FLAGS += -DENABLE_OCC=1 -DENABLE_OCC_CAF=1 -DENABLE_OCC_STATIC=1
  endif
  # Disable Cairo fonts for now. Just use FreeType (required by OCCT).
  GMSH_FLAGS += -DENABLE_CAIRO=0
  GMSH_DEPS:=$(patsubst %,$(BUILD_DIR)/external/install-%.out,$(GMSH_REQUIRES))
  $(shell printf "%s" "$(gmsh_FLAGS)" \
    > $(BUILD_DIR)/external/gmsh.flags.new)
endif
ifeq ($(ENABLE_OCCT),ON)
  LIST_EXTERNAL += occt
  OCCT_REQUIRES += freetype
  EXT_DOT+="FreeType" -> "OpenCASCADE"\n
  ENABLE_FREETYPE:=ON
  OCCT_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  OCCT_FLAGS += -DCMAKE_PREFIX_PATH="$(INSTALL_DIR)"
  OCCT_FLAGS += -DBUILD_LIBRARY_TYPE=Static
  OCCT_FLAGS += -DCMAKE_BUILD_TYPE=Release
  OCCT_FLAGS += -DBUILD_MODULE_Draw=0
  OCCT_FLAGS += -DBUILD_MODULE_Visualization=0
  OCCT_FLAGS += -DBUILD_MODULE_ApplicationFramework=0
  OCCT_DEPS:=$(patsubst %,$(BUILD_DIR)/external/install-%.out,$(OCCT_REQUIRES))
  $(shell printf "%s" "$(OCCT_FLAGS)" \
    > $(BUILD_DIR)/external/occt.flags.new)
endif
ifeq ($(ENABLE_HDF5),ON)
  LIST_EXTERNAL += hdf5
  HDF5_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  HDF5_FLAGS += -DHDF5_BUILD_CPP_LIB:BOOL=OFF
  HDF5_FLAGS += -DHDF5_ENABLE_PARALLEL:BOOL=ON
  HDF5_FLAGS += -DMPIEXEC_MAX_NUMPROCS:STRING=4
  HDF5_FLAGS += -DBUILD_STATIC_LIBS:BOOL=ON
  HDF5_FLAGS += -DBUILD_SHARED_LIBS:BOOL=ON
  HDF5_FLAGS += -DCTEST_BUILD_CONFIGURATION=Release
  HDF5_FLAGS += -DHDF5_NO_PACKAGES:BOOL=ON
  $(shell printf "%s" "$(HDF5_FLAGS)" \
    > $(BUILD_DIR)/external/hdf5.flags.new)
endif
ifeq ($(ENABLE_CGNS),ON)
  LIST_EXTERNAL += CGNS
  EXT_DOT+="CGNS" -> "Femera"\n
  ifeq ($(ENABLE_HDF5),ON)
    CGNS_REQUIRES += hdf5
    EXT_DOT+="HDF5" -> "CGNS"\n
    CGNS_FLAGS += -DCGNS_ENABLE_HDF5:BOOL=ON
  else
    CGNS_FLAGS += -DCGNS_ENABLE_HDF5:BOOL=OFF
  endif
  CGNS_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  CGNS_FLAGS += -DCGNS_ENABLE_LFS:BOOL=ON
  CGNS_FLAGS += -DCGNS_ENABLE_64BIT:BOOL=ON
  CGNS_FLAGS += -DCGNS_ENABLE_LEGACY:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_SCOPING:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_BASE_SCOPE:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_TESTS:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_FORTRAN:BOOL=OFF
  CGNS_DEPS:=$(patsubst %,$(BUILD_DIR)/external/install-%.out,$(CGNS_REQUIRES))
  $(shell printf "%s" "$(CGNS_FLAGS)" \
    > $(BUILD_DIR)/external/CGNS.flags.new)
endif
#FIXME PETSc, CGNS, FLTK, and FreeType build in external/*/
ifeq ($(ENABLE_PETSC),ON)
  LIST_EXTERNAL += petsc
  EXT_DOT+="PETSc" -> "Femera"\n
  PETSC_FLAGS += --prefix="$(INSTALL_DIR)"
  ifeq ($(ENABLE_OMP),ON)
    PETSC_FLAGS += --with-openmp
  endif
  ifeq ($(ENABLE_HDF5),ON)
    PETSC_REQUIRES += hdf5
    PETSC_FLAGS += --with-hdf5
    EXT_DOT+="PETSc" -> "HDF5"\n
  endif
  ifeq ($(ENABLE_CGNS),ON)
    PETSC_REQUIRES += CGNS
    EXT_DOT+="PETSc" -> "CGNS"\n
    PETSC_FLAGS += --with-cgns --with-zlib
  endif
  PETSC_DEPS:=$(patsubst %,$(BUILD_DIR)/external/install-%.out,$(PETSC_REQUIRES))
  $(shell printf "%s" "$(PETSC_FLAGS)" \
    > $(BUILD_DIR)/external/petsc.flags.new)
endif
ifeq ($(ENABLE_FLTK),ON)
  LIST_EXTERNAL += fltk
  FLTK_FLAGS += --prefix="$(INSTALL_DIR)"
  FLTK_FLAGS += --enable-shared
  $(shell printf "%s" "$(FLTK_FLAGS)" \
    > $(BUILD_DIR)/external/fltk.flags.new)
endif
ifeq ($(ENABLE_FREETYPE),ON)
  LIST_EXTERNAL += freetype
  FREETYPE_FLAGS += --prefix="$(INSTALL_DIR)"
  $(shell printf "%s" "$(FREETYPE_FLAGS)" \
    > $(BUILD_DIR)/external/freetype.flags.new)
endif
ifeq ($(ENABLE_MKL),ON)
  LIST_EXTERNAL += mkl
  EXT_DOT+="MKL" -> "Femera"\n
  # No point saving extracted files. The installer removes them automatically.
  MKL_FLAGS += --extract-folder $(FMRDIR)/external/mkl
  MKL_FLAGS += --remove-extracted-files no
  MKL_FLAGS += -a --silent --eula accept --install-dir $(INSTALL_DIR)
  # https://www.intel.com/content/www/us/en/develop/documentation
  # /installation-guide-for-intel-oneapi-toolkits-linux/top/installation
  # /install-with-command-line.html#install-with-command-line
  $(shell printf "%s" "$(MKL_FLAGS)" \
    > $(BUILD_DIR)/external/install-mkl.flags.new)
endif
# Developer tools
ifeq ($(ENABLE_DOT),ON)
  MAKE_DOT+="cinclude2dot" -> "Makefile"\n
  LIST_EXTERNAL += cinclude2dot
  $(shell printf "%s" "$(DOT_FLAGS)"  \
    > $(BUILD_DIR)/external/install-cinclude2dot.flags.new)
endif
EXT_DOT+=}\n
EXT_DOTFILE:=$(BUILD_DIR)/external/external.dot
MAKE_DOT+=}\n
MAKE_DOTFILE:=$(BUILD_DIR)/make.dot

# Files -----------------------------------------------------------------------
# Generic Femera tools
LIST_TOOLS:= fmrmodel fmrcores fmrexec fmrnumas
INSTALL_TOOLS:= $(patsubst %,$(INSTALL_DIR)/bin/%,$(LIST_TOOLS))

# CPU-specific Femera tools
# LIST_TOOLS+= fmrnumas
# INSTALL_TOOLS+= $(INSTALL_CPU)/bin/fmrnumas
# TEST_TOOLS:= $(patsubst %,$(BUILD_CPU)/tools/%.test.out,$(LIST_TOOLS))

# External packages
GET_EXTERNAL:= $(patsubst %,$(BUILD_DIR)/external/get-%.out,$(LIST_EXTERNAL))
INSTALL_EXTERNAL:= $(patsubst %,$(BUILD_DIR)/external/install-%.out, \
  $(LIST_EXTERNAL))

# GET_BATS:= $(patsubst %,$(BUILD_DIR)/external/get-%.out,$(BATS_MODS))
GET_BATS:= $(patsubst %,get-%,$(BATS_MODS))

# GIT_SUBMODULES:= #TODO?

# -----------------------------------------------------------------------------
ifeq ("$(findstring $(INSTALL_DIR)/bin:,$(PATH):)","")
  ADD_TO_PATH:= $(INSTALL_DIR)/bin:$(ADD_TO_PATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/bin:,$(PATH):)","")
  ADD_TO_PATH:= $(INSTALL_CPU)/bin:$(ADD_TO_PATH)
endif
ifneq ("$(ADD_TO_PATH)","")
  export PATH:= $(ADD_TO_PATH)$(PATH)
  #NOTE export does not work for this in a recipe below.
endif

ifeq ("$(findstring $(INSTALL_DIR)/lib64:,$(LDPATH):)","")
  ADD_TO_LDPATH:= $(INSTALL_DIR)/lib64:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_DIR)/lib:,$(LDPATH):)","")
  ADD_TO_LDPATH:= $(INSTALL_DIR)/lib:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/lib64:,$(LDPATH):)","")
  ADD_TO_LDPATH:= $(INSTALL_CPU)/lib64:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/lib:,$(LDPATH):)","")
  ADD_TO_LDPATH:= $(INSTALL_CPU)/lib:$(ADD_TO_LDPATH)
endif
ifneq ("$(ADD_TO_LDPATH)","")
  export LD_LIBRARY_PATH:= $(ADD_TO_LDPATH)$(LD_LIBRARY_PATH)
endif

ifeq ("$(FMR_COPYRIGHT)","")
  export FMR_COPYRIGHT:= cat data/copyright.txt | tr '\n' ' ' | tr -s ' '
  NOSA_INFO:= See the NASA open source agreement (NOSA-1-3.txt) for details.
endif

# make recipes ================================================================
# These .PHONY targets are intended for users.
# external libfemera test tune
.PHONY: all tools external mini femera install
.PHONY: uninstall reinstall clean cleaner cleanest purge
# -----------------------------------------------------------------------------
# The rest are for developers...
.PHONY: docs hash patch
.PHONY: install-tools test-tools uninstall-tools reinstall-tools clean-tools
.PHONY: get-bats install-bats get-external # install-external
# ...and internal makefile use.
.PHONY: intro
.PHONY: all-done build-done docs-done
.PHONY: install-tools-done install-done uninstall-done

# Real files, but considered always out of date.
.PHONY: build/.md5

# -----------------------------------------------------------------------------
.SILENT:

# $(STAGE_DIR)/% Does not work as target for .SECONDARY or .PRECIOUS.
# It looks like .PRECIOUS prerequisites must match a target pattern exactly.
.PRECIOUS: $(STAGE_DIR)/bin/fmr% $(BUILD_DIR)/external/install-%.flags

# libmini libfull
# pre-build-tests post-build-tests post-install-tests
# done
# $(BUILD_DIR)/VERSION

# Primary named targets -------------------------------------------------------
# These are intended for users.
# Many launch parallel make jobs to simplify command-line use.
femera: tools
	$(call timestamp,$@,$^)

all: | intro
	$(call timestamp,$@,-$(MAKEFLAGS))
	$(MAKE) $(JSER) external
	#(MAKE) $(JPAR) mini
	#$(MAKE) test tune
	$(MAKE) $(JPAR) docs
	$(MAKE) $(JSER) install

tools: | intro $(STAGE_TREE)
	$(MAKE) $(JPAR) install-tools
	$(call timestamp,$@,$^)

external: tools get-external
	$(call timestamp,$@,$^)
	#(MAKE) $(JPAR) get-external
	$(MAKE) $(JSER) install-external
	$(MAKE) $(JPAR) external-done

docs: | intro build/docs/
	$(call timestamp,$@,)
	$(MAKE) $(JPAR) docs-done

install: tools docs | $(STAGE_TREE)
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) install-done

uninstall:
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) uninstall-tools
	$(MAKE) $(JPAR) uninstall-done

reinstall: | intro
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) uninstall
	$(MAKE) $(JSER) install

clean: $(BUILD_CPU)/femera/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

cleaner: $(BUILD_CPU)/ $(STAGE_CPU)/
	$(call timestamp,$@,$^)
	-rm -rf $^ $(STAGE_DIR)/bin/fmr*
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

cleanest: build/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

purge:
	$(MAKE) $(JPAR) uninstall
	$(MAKE) $(JPAR) cleanest

# Internal named targets ======================================================
intro: build/copyright.txt build/docs/find-tdd-files.csv | $(BUILD_TREE)
ifneq ("$(NOSA_INFO)","")
	$(info $(INFO) $(NOSA_INFO))
endif
ifneq ("$(ADD_TO_PATH)","")
	$(info $(NOTE) temporarily prepended PATH with:)
	$(info $(SPCS) $(ADD_TO_PATH))
endif
ifeq ($(ENABLE_DOT),ON)
	@printf '%s' '$(MAKE_DOT)' | sed 's/\\n/\n/g' > '$(MAKE_DOTFILE)'
	@dot '$(MAKE_DOTFILE)' -Teps -o $(BUILD_DIR)/make.eps
	@dot external/external.dot -Teps -o $(BUILD_DIR)/external/external.eps
	@printf '%s' '$(EXT_DOT)' | sed 's/\\n/\n/g' > '$(EXT_DOTFILE)'
	@dot '$(EXT_DOTFILE)' -Teps -o $(BUILD_DIR)/external/build-external.eps
endif


docs-done: build/docs/femera-guide.pdf build/docs/femera-quick-start.pdf
docs-done: docs/femera-guide.xhtml docs/femera-quick-start.xhtml
ifeq ($(ENABLE_LYX),ON)
	$(info $(DONE) making $(FEMERA_VERSION) XHTML documentation in \
	external/docs/)
	$(info $(SPCS) and PDFs in build/docs/)
else
	$(info $(WARN) Set ENABLE_LYX to ON in config.local to make \
	  $(FEMERA_VERSION) documentation)
endif
	$(call timestamp,$@,)

install-done:
	$(info $(DONE) installing $(FEMERA_VERSION) on $(HOSTNAME) to:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(patsubst %,%;,$(LIST_TOOLS)))
	$(call timestamp,$@,)

uninstall-done:
	-rm -f $(INSTALL_CPU)/bin/mini
	$(info $(DONE) uninstalling $(FEMERA_VERSION) on $(HOSTNAME) from:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(patsubst %,%;,$(LIST_TOOLS)))
	$(call timestamp,$@,)

build-done: # $(BUILD_CPU)/make-build.post.test.out
	$(call timestamp,$@,$?)
	$(info $(DONE) building $(FEMERA_VERSION))
	$(info $(SPCS) on $(HOSTNAME) for $(CPUMODEL) with $(CXX) $(CXX_VERSION))
ifneq ($(HOST_MD5),$(REPO_MD5))
	$(info $(SPCS) modified by <$(BUILT_BY_EMAIL)>.)
endif
	$(info $(E_G_) tools/fmrexec.sh auto -d -t -D examples/cube.fmr)


# Femera tools ----------------------------------------------------------------
install-tools: get-bats
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) install-tools-done

clean-tools: $(BUILD_CPU)/tools/ | $(STAGE_CPU)/bin/
	$(call timestamp,$@,$^)
	-rm -rf $^ $(STAGE_CPU)/bin/fmr*

uninstall-tools: clean-tools | $(BUILD_DIR)/tests/ $(BUILD_CPU)/tests/
	$(call timestamp,$@,$<)
	-rm -f $(INSTALL_DIR)/bin/fmr* $(INSTALL_CPU)/bin/fmr*
	$(MAKE) $(JPAR) uninstall-tools-done

reinstall-tools:
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) uninstall-tools
	$(MAKE) $(JPAR) install-tools
	#grep -h . build/i7-7820HQ/tools/* build/i7-7820HQ/tests/*

test-tools:
	$(call timestamp,$@,$(LIST_TOOLS))
	$(MAKE) $(JPAR) $(TEST_TOOLS)

install-tools-done: $(TEST_TOOLS) $(INSTALL_TOOLS)
	$(MAKE) $(JPAR) $(BUILD_CPU)/tests/make-install-tools.test.out

uninstall-tools-done: $(BUILD_DIR)/tests/make-uninstall-tools.test.out
uninstall-tools-done: $(BUILD_CPU)/tests/make-uninstall-tools.test.out

# External --------------------------------------------------------------------

install-bats: external/get-bats.test.sh
install-bats: get-bats external/install-bats.sh external/install-bats.test.bats
	$(call label_test,$(PASS),$(FAIL),external/get-bats.test.sh, \
	  $(BUILD_DIR)/external/get-bats.test)
	$(call label_test,$(EXEC),ERROR , \
	  external/install-bats.sh "$(INSTALL_DIR)", \
	  $(BUILD_DIR)/external/install-bats)
	$(call label_bats,$(PASS),$(FAIL),external/install-bats.test.bats, \
	  $(BUILD_DIR)/external/install-bats.test)
	$(call timestamp,$@,$<)

get-bats:
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) $(GET_BATS)

get-external: | external/tools/
	$(MAKE) $(JPAR) $(GET_EXTERNAL)
	$(call timestamp,$@,)

install-external: $(INSTALL_EXTERNAL)
	$(call timestamp,$@,make $(JEXT))

external-done:
	$(info $(DONE) building and installing externals on $(HOSTNAME) to:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(patsubst %,%,$(LIST_EXTERNAL)))
	$(call timestamp,$@,)

get-%: | $(BUILD_DIR)/external/
	#(call timestamp,$@,$<)
	external/get-external.sh $(*)

$(BUILD_DIR)/external/get-bats-%.out: external/get-external.sh
	#(info $(INFO) looking for bats-$(*)...)
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "external/get-external.sh bats-$(*)"   \
	  "$(BUILD_DIR)/external/get-bats-$(*)"

$(BUILD_DIR)/external/get-%.out: external/get-external.sh external/get-%.dat
	$(info $(INFO) looking for $(*)...)
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "external/get-external.sh $(*)"   \
	  "$(BUILD_DIR)/external/get-$(*)"

install-%: $(BUILD_DIR)/external/install-%.out | $(BUILD_DIR)/external/
	$(call timestamp,$@,)

$(BUILD_DIR)/external/install-%.flags: $(BUILD_DIR)/external/install-%.flags.new
	tools/update-file-if-diff.sh "$(@)"
	rm -f $(<)

$(BUILD_DIR)/external/install-%.out: external/install-%.sh
$(BUILD_DIR)/external/install-%.out: $(BUILD_DIR)/external/install-%.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_DIR)/external/$(*)
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "external/install-$(*).sh $(INSTALL_DIR) $(<) $(JEXT)" \
	  "$(BUILD_DIR)/external/install-$(*)"

# Specialized targets =========================================================
$(STAGE_DIR)/bin/fmr%: $(BUILD_CPU)/tools/fmr%.test.out
	#(info $(INFO) staging fmr$(*) to $(STAGE_CPU)/bin/...)
	cp -f "tools/fmr$(*).sh" "$(@)"
	# should only copy fmrmodel, fmrcores, fmrnumas, fmrexec

$(INSTALL_DIR)/bin/fmr%: $(STAGE_DIR)/bin/fmr% | $(INSTALL_DIR)/bin/
	#(info $(INFO) installing fmr$(*) to $(INSTALL_DIR)/bin/...)
	cp -f "$(STAGE_DIR)/bin/fmr$(*)" "$(@)"

#ifeq ($(ENABLE_LIBNUMA),ON)
#$(STAGE_CPU)/bin/fmrnumas: $(BUILD_CPU)/tools/fmrnumas.test.out
#	cp -f $(BUILD_CPU)/tools/fmrnumas "$(@)"
#
#$(INSTALL_CPU)/bin/fmr%: $(STAGE_CPU)/bin/fmr% | $(INSTALL_CPU)/bin/
#	#(info $(INFO) installing fmr$(*) to $(INSTALL_CPU)/bin/...)
#	cp -f "$(STAGE_CPU)/bin/fmr$(*)" "$(@)"
#endif

# CPU-specific test targets (*.test.out) --------------------------------------
#ifeq ($(ENABLE_LIBNUMA),ON)
#$(BUILD_CPU)/tools/fmrnumas.test.out: src/tools/fmrnumas.c tools/fmrnumas.test.bats
#	$(info $(CC__) $(CC) $(<) .. -o $(BUILD_CPU)/tools/fmrnumas)
#	$(CC) $(<) $(FMRFLAGS) $(LDLIBS) -o $(BUILD_CPU)/tools/fmrnumas
#	$(call label_bats,$(PASS),$(FAIL),tools/fmrnumas.test.bats, \
#	  $(BUILD_CPU)/tools/fmrnumas.test)
#endif

$(BUILD_DIR)/tests/make-uninstall-tools.test.out: tests/make-uninstall-tools.test.bats
	$(call label_bats,$(PASS),$(FAIL),DIR="$(INSTALL_DIR)/bin" $(<), \
	  $(BUILD_CPU)/tests/make-uninstall-tools.test)
$(BUILD_CPU)/tests/make-uninstall-tools.test.out: tests/make-uninstall-tools.test.bats
	$(call label_bats,$(PASS),$(FAIL),DIR="$(INSTALL_CPU)/bin" $(<), \
	  $(BUILD_CPU)/tests/make-uninstall-tools.test)
	#NOTE Pass test parameters through the environment, e.g.,
	#     MYVAR="myval" my.test.bats;
	#     because Bats scripts do not support test arguments.
	# https://bats-core.readthedocs.io/en/stable/gotchas.html#i-cannot-pass-parameters-to-test-or-bats-files

# Tested shell scripts
$(BUILD_CPU)/%.test.out: %.sh %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_CPU)/$(*).test)
	#touch $(@) # TODO Why does this do nothing?
$(BUILD_CPU)/%.test.out: %.sh %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_CPU)/$(*).test)
$(BUILD_CPU)/%.test.out: %.sh %.test.sh
	$(call label_test,$(PASS),$(FAIL),$(*).test.sh,$(BUILD_CPU)/$(*).test)

# Tested Python scripts
$(BUILD_CPU)/%.test.out: %.py %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_CPU)/$(*).test)
$(BUILD_CPU)/%.test.out: %.py %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_CPU)/$(*).test))

# Integration tests
$(BUILD_CPU)/%.test.out: %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_CPU)/$(*).test)
$(BUILD_CPU)/%.test: %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_CPU)/$(*).test))

# Warn if no test.
$(BUILD_CPU)/%.test.out: %.sh
	$(info $(DBUG) $(<) needs a test, e.g., $(*).test.bats)
	echo "$(<) needs a test, e.g., $(*).test.bats" \
	  >> $(BUILD_CPU)/$(*).test.out
$(BUILD_CPU)/%.test.out: %.py
	$(info $(DBUG) $(<) needs a test, e.g., $(*).test.py)
	echo "$(<) needs a test, e.g., $(*).test.py" \
	  >> $(BUILD_CPU)/$(*).test.out

# Generic test targets (*.test.out) -------------------------------------------

# Tested shell scripts
$(BUILD_DIR)/%.test.out: %.sh %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_DIR)/$(*).test)
	#touch $(@) # TODO Why does this do nothing?
$(BUILD_DIR)/%.test.out: %.sh %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_DIR)/$(*).test)
$(BUILD_DIR)/%.test.out: %.sh %.test.sh
	$(call label_test,$(PASS),$(FAIL),$(*).test.sh,$(BUILD_DIR)/$(*).test)

# Tested Python scripts
$(BUILD_DIR)/%.test.out: %.py %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_DIR)/$(*).test)
$(BUILD_DIR)/%.test.out: %.py %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_DIR)/$(*).test)

# Integration tests
$(BUILD_DIR)/%.test.out: %.test.bats
	$(call label_bats,$(PASS),$(FAIL),$(*).test.bats,$(BUILD_DIR)/$(*).test)
$(BUILD_DIR)/%.test: %.test.py
	$(call label_test,$(PASS),$(FAIL),$(*).test.py,$(BUILD_DIR)/$(*).test)

# Warn if no test.
$(BUILD_DIR)/%.test.out: %.sh
	$(info $(DBUG) $(<) needs a test, e.g., $(*).test.bats)
	echo "$(<) needs a test, e.g., $(*).test.bats" \
	  >> $(BUILD_DIR)/$(*).test.out
$(BUILD_DIR)/%.test.out: %.py
	$(info $(DBUG) $(<) needs a test, e.g., $(*).test.py)
	echo "$(<) needs a test, e.g., $(*).test.py" \
	  >> $(BUILD_DIR)/$(*).test.out

# Untested targets ------------------------------------------------------------

%.test.bats: # Continue when tests do not exist.
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))
%.test.py:
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))
%.test.sh:
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))

$(BUILD_CPU)/%.test,out: # Warn if no test.
	$(info $(DBUG) did not find test: $(*).test.*)

# hm ==========================================================================
%/:
	mkdir -p $(@)

build/.md5: | build/
	touch $@
build/%/.md5: | build/%/
	touch $@

build/modification.txt: data/modification.txt build/.md5
	touch "build/modification.txt"
ifneq ($(HOST_MD5),$(REPO_MD5))
	cp "data/modification.txt" "build/modification.txt"
endif

build/copyright.txt: data/copyright.txt build/modification.txt
	cp "data/copyright.txt" $@
ifneq ("$(NOSA_INFO)","")
	printf "$(INFO_COLOR)";
	cat build/modification.txt build/copyright.txt \
	  | tr '\n' ' ' | tr -s ' ' | fold -s -w 80
	printf "$(NORM_COLOR)\n";
endif

build/test-files.txt: tools/list-test-files.sh build/.md5
	-tools/list-test-files.sh > $@

build/docs/tdd-tests.txt: tools/list-tdd-tests.sh build/docs/.md5
	-tools/list-tdd-tests.sh src/docs/*.tex src/docs/*.lyx > $@

build/docs/find-tdd-files.csv: build/docs/tdd-tests.txt build/test-files.txt
build/docs/find-tdd-files.csv: tools/compare-lists.py
	-tools/compare-lists.py build/docs/tdd-tests.txt build/test-files.txt \
	  >$@ 2>build/docs/find-tdd-files.err
	#(call label_test,$(PASS),$(DBUG), \
	#  tools/compare-lists.py build/docs-tests.txt build/test-files.txt, \
	#  build/find-docs-tdd-files)

build/docs/%.pdf: src/docs/%.lyx src/docs/quick-start.tex
ifeq ($(ENABLE_LYX),ON)
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "lyx -batch -n -f all -E pdf $(@) $(<)" \
	  "build/docs/$(*)-pdf"
endif

docs/%.xhtml: src/docs/%.lyx src/docs/quick-start.tex
ifeq ($(ENABLE_LYX),ON)
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "lyx -batch -n -f all -E xhtml $(@) $(<)" \
	  "build/docs/$(*)-xhtml"
endif
