#!/usr/bin/make
.DEFAULT_GOAL := all
SHELL:= bash
FMRDIR:=$(shell pwd)

include examples/config.recommended
-include config.local

include tools/set-undefined.mk
include tools/functions.mk

#------------------------------------------------------------------------------
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

#------------------------------------------------------------------------------
ifeq ($(CXX),g++)
  ifeq ($(ENABLE_MPI),ON)
    CXX:= mpic++
  endif
  OPTFLAGS:= $(shell cat data/gcc4.flags | tr '\n' ' ' | tr -s ' ')
  CXXFLAGS+= -std=c++11 -g -MMD -MP
  ifeq ($(ENABLE_LTO),ON)
    CXXFLAGS+= -flto
  endif
  CXXFLAGS+= $(OPTFLAGS) -fstrict-enums
  # Dependency file generation: -MMD -MP
  #NOTE -fpic needed for shared libs, but may degrade static lib performance.
  ifeq ($(ENABLE_OMP),ON)
    CXXFLAGS+= -fopenmp
    FMRFLAGS+= -D_GLIBCXX_PARALLEL
  endif
  # Warning flags
  CXXWARNS+= -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wfloat-equal
  CXXWARNS+= -Wdouble-promotion -Wconversion -Wcast-qual -Wcast-align
  CXXWARNS+= -Wlogical-op -Woverloaded-virtual -Wstrict-null-sentinel
  CXXWARNS+= -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
  CXXWARNS+= -Wunused-macros -Wzero-as-null-pointer-constant -Wundef -Weffc++
  # CXXWARNS+= -Winline
  # Library archiver
  AREXE:= gcc-ar
endif
ifeq ($(CXX),icpc)
  OPTFLAGS:= $(shell cat data/icpc.flags | tr '\n' ' ' | tr -s ' ')
  CXXFLAGS:= c++11 -restrict -g $(OPTFLAGS)
  #NOTE -fPIC needed for shared libs, but may degrade static lib performance.
  CXXWARNS+= -Wall -Wextra -Wshadow
  ifeq ($(ENABLE_MKL),ON)
    FMRFLAGS+= -mkl=sequential -DMKL_DIRECT_CALL_SEQ
  endif
  AREXE:= xiar
endif
ifneq ($(FMR_MICRO_UCHAR),)
  FMRFLAGS += -DFMR_MICRO_UCHAR=$(FMR_MICRO_UCHAR)
endif
FMRFLAGS += -I"$(STAGE_CPU)/include" -I"$(STAGE_DIR)/include"
FMRFLAGS += -I"$(INSTALL_CPU)/include" -I"$(INSTALL_DIR)/include"
LDFLAGS += -L"$(STAGE_CPU)/lib" -L"$(STAGE_DIR)/lib"

# Directories -----------------------------------------------------------------
# Directories for generic components
BUILD_DIR  := build
STAGE_DIR  := $(BUILD_DIR)/stage
INSTALL_DIR:= $(PREFIX)

# Directories for CPU-model specific components
BUILD_CPU  := $(BUILD_DIR)/$(CPUMODEL)
STAGE_CPU  := $(STAGE_DIR)/$(CPUMODEL)
INSTALL_CPU:= $(INSTALL_DIR)/$(CPUMODEL)
TEMP_DIR   := $(BUILD_CPU)/tmp

#NOTE Subdirectories needed in the build directory have a trailing slash.
BUILD_TREE+= $(BUILD_DIR)/external/tools/ $(BUILD_DIR)/docs/
BUILD_TREE+= $(BUILD_CPU)/external/ $(BUILD_CPU)/tests/ $(BUILD_CPU)/tools/
BUILD_TREE+= $(BUILD_CPU)/femera/proc/ $(BUILD_CPU)/femera/data/
BUILD_TREE+= $(BUILD_CPU)/femera/file/ $(BUILD_CPU)/femera/test/
BUILD_TREE+= $(BUILD_CPU)/fmr/perf/

STAGE_TREE+= $(STAGE_DIR)/bin/ $(STAGE_DIR)/lib/
STAGE_TREE+= $(STAGE_CPU)/bin/ $(STAGE_CPU)/lib/

# -----------------------------------------------------------------------------
ifeq ("$(findstring $(INSTALL_DIR)/bin:,$(PATH):)","")
  ADD_TO_PATH:= $(INSTALL_DIR)/bin:$(ADD_TO_PATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/bin:,$(PATH):)","")
  ADD_TO_PATH:= $(INSTALL_CPU)/bin:$(ADD_TO_PATH)
endif
ifneq ("$(ADD_TO_PATH)","")# only true once during build
  export PATH:= $(ADD_TO_PATH)$(PATH)
  #NOTE export does not work for this when in a recipe.
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
ifneq ("$(ADD_TO_LDPATH)","")# only true once during build
  export LD_LIBRARY_PATH:= $(ADD_TO_LDPATH)$(LD_LIBRARY_PATH)
endif
# TMP_LIBRARY_PATH_=$(LD_LIBRARY_PATH);
  LDFLAGS+= -L$(INSTALL_CPU)/lib -L$(INSTALL_CPU)/lib64
  LDFLAGS+= -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/lib64

ifeq ("$(FMR_COPYRIGHT)","")# only true once during build
  export FMR_COPYRIGHT:= cat data/copyright.txt | tr '\n' ' ' | tr -s ' '
  NOSA_SEE:= See the NASA open source agreement (NOSA-1-3.txt) for details.
endif
-include $(BUILD_CPU)/external.config.mk

#NOTE CGNS, FLTK, FreeType, PETSc build in external/*/

# Libraries and applications available ----------------------------------------
ifeq ($(ENABLE_OMP),ON)
  EXTERNAL_DOT+="OpenMP" -> "Femera"\n
    FMRFLAGS+= -DFMR_HAS_OMP
endif
ifeq ($(ENABLE_MPI),ON)
  EXTERNAL_DOT+="MPI" -> "Femera"\n
  FMRFLAGS+= -DFMR_HAS_MPI
  #FIXME Find include and lib dirs automatically
  ifeq ($(CXX),mpic++)
    FMRFLAGS+= -isystem"/usr/include/openmpi-x86_64"
  else
    FMRFLAGS+= -I"/usr/include/openmpi-x86_64"
  endif
  LDFLAGS+= -L/usr/lib64/openmpi/lib
  LDLIBS+= -lmpi
endif
ifeq ($(ENABLE_LIBNUMA),ON)
  EXTERNAL_DOT+="libnuma" -> "Femera"\n
  FMRFLAGS+= -DFMR_HAS_LIBNUMA
  LDLIBS+= -lnuma
endif
# Build tools to download and install -----------------------------------------
ifeq ($(ENABLE_BATS),ON)
  MAKE_DOT+="Bats" -> "Makefile"\n
  BATS_MODS:= bats-core bats-support bats-assert bats-file
  label_bats = $(call label_test,$(1),$(2),$(3),$(4))
else
  label_bats = $(info \
    $(DBUG) Set ENABLE_BATS:=ON in config.local for $(notdir $(3)).)
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  # Flags for compiling Femera tests
  CXXGTEST := $(CXXFLAGS) $(CXXWARNS)
  CXXGTEST := $(filter-out -Wundef,$(CXXGTEST))
  CXXGTEST := $(filter-out -Weffc++,$(CXXGTEST))
  CXXGTEST := $(filter-out -Winline,$(CXXGTEST))
  CXXGTEST := $(filter-out -flto,$(CXXGTEST))
  #NOTE Parallel make breaks link-time optimization of gtests.
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  LIST_EXTERNAL += googletest
  FMRFLAGS+= -DFMR_HAS_GTEST
  EXTERNAL_DOT+="GoogleTest" -> "Femera"\n
  MAKE_DOT+="GoogleTest" -> "Makefile"\n
  GTEST_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  LDLIBS += -lpthread -lgtest -lgmock
  GTEST_FLAGFILE:= $(BUILD_CPU)/external/install-googletest.flags
endif
ifeq ($(ENABLE_PYBIND11),ON)
#  LIST_EXTERNAL += pybind11
  INSTALL_EXTERNAL+= $(BUILD_DIR)/external/install-pybind11.out
#  PYBIND11_REQUIRES += boost-headers
  EXTERNAL_DOT+="pybind11" -> "Femera"\n
  # BUILD_TREE += $(BUILD_DIR)/external/pybind11/
#  EXTERNAL_DOT+="Boost" -> "pybind11"\n
  # FMRFLAGS += -DFMR_HAS_PYBIND11
  PYBIND11_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  PYBIND11_FLAGS += -DDOWNLOAD_CATCH=0
#  PYBIND11_FLAGS += -DPYBIND11_PYTHON_VERSION=3.6
  PYBIND11_FLAGS += -DPYBIND11_HAS_OPTIONAL=0
  PYBIND11_FLAGS += -DPYBIND11_HAS_EXP_OPTIONAL=0
  PYBIND11_FLAGS += -DPYBIND11_HAS_VARIANT=0
  #FIXME Find include and lib dirs automatically
#  PYBIND11_FLAGS += -DBOOST_ROOT:PATHNAME=/usr/local/pkgs-modules/boost_1.66.0
  PYBIND11_FLAGFILE:= $(BUILD_DIR)/external/install-pybind11.flags
#  PYBIND11_DEPS:=$(patsubst \
#    %,$(BUILD_DIR)/external/install-%.out,$(PYBIND11_REQUIRES))
endif
#ifeq ($(ENABLE_BOOST_HEADERS),ON)
#  LIST_EXTERNAL += boost-headers
#  BOOST_LIBS:=$(shell cat external/boost-headers.dat | tr '\n' ',' | tr -d ' ')
#  BOOST_FLAGS += --prefix="$(INSTALL_DIR)"
#  BOOST_FLAGS += --show-libraries --with-libraries=$(BOOST_LIBS)
#  BOOST_FLAGFILE:= $(BUILD_DIR)/external/install-boost.flags
#endif
# Developer tools
ifeq ($(ENABLE_DOT),ON)
  ifeq ($(shell which dot 2>/dev/null),"")# dot is part of graphviz
    ENABLE_DOT:=OFF
  else
    MAKE_DOT+="dot" -> "Makefile"\n
    HEAD_DOT+=overlap=scale;\n
    HEAD_DOT+=size="6,3";\n
    HEAD_DOT+=ratio="fill";\n
    HEAD_DOT+=fontsize="12";\n
    HEAD_DOT+=fontname="Helvetica";\n
    HEAD_DOT+=clusterrank="local";\n
  endif
  DOT_FLAGFILE:=$(BUILD_DIR)/external/install-cinclude2dot.flags
endif
ifeq ($(ENABLE_DOT),ON)
  GET_EXTERNAL+= $(BUILD_DIR)/external/get-cinclude2dot.out
  INSTALL_EXTERNAL+= $(BUILD_DIR)/external/install-cinclude2dot.out
  MAKE_DOT+="cinclude2dot" -> "Makefile"\n
  EXTERNAL_DOT:= digraph "Femera external dependencies as built" \
    {\n $(HEAD_DOT) $(EXTERNAL_DOT) }\n
  MAKE_DOT:= digraph "Makefile dependencies" {\n $(HEAD_DOT) $(MAKE_DOT) }\n
  EXTERNAL_DOTFILE:= $(BUILD_DIR)/external/external.dot
  MAKE_DOTFILE:= $(BUILD_DIR)/make.dot
endif

CXXFLAGS+= $(CXXWARNS)
CXX_VERSION   := $(shell $(CXX) -dumpversion)

# Files -----------------------------------------------------------------------
# Generic Femera tools
LIST_TOOLS:= fmrmodel fmrcores fmrexec fmrnumas
INSTALL_TOOLS:= $(patsubst %,$(INSTALL_DIR)/bin/%,$(LIST_TOOLS))

# External packages
GET_EXTERNAL+= $(patsubst %,$(BUILD_DIR)/external/get-%.out,$(LIST_EXTERNAL))
INSTALL_EXTERNAL+= $(patsubst %,$(BUILD_CPU)/external/install-%.out, \
  $(LIST_EXTERNAL))

GET_BATS:= $(patsubst %,get-%,$(BATS_MODS))

LIBFEMERA:=$(STAGE_CPU)/lib/libfemera.a

# Changing any of these should cause a full rebuld.
TOPDEPS += $(BUILD_CPU)/femera.flags

# C++11 code to compile and gtest
FMRCPPS:= $(shell find src/ -maxdepth 4 -name "*.cpp" -printf "%p ")
DEPFILE:= $(patsubst src/%.cpp,$(BUILD_CPU)/%.d,$(FMRCPPS))
FMRGTST:= $(filter %.gtst.cpp,$(FMRCPPS))
FMROUTS:= $(patsubst src/%.gtst.cpp,$(BUILD_CPU)/%.gtst.out,$(FMRGTST))

# make recipes ================================================================
# Handle dependancies generated by CXX -MMD -MP ...
-include $(DEPFILE)
#NOTE This needs to be before "no test" recipes.

# These .PHONY targets are intended for users.
# external libfemera test tune
.PHONY: all tools external mini femera install
.PHONY: remove reinstall clean cleaner cleanest purge
# -----------------------------------------------------------------------------
# The rest are for developers...
.PHONY: docs hash patch
.PHONY: install-tools test-tools remove-tools reinstall-tools clean-tools
.PHONY: get-bats install-bats get-external # install-external
# ...and internal makefile use.
.PHONY: intro
.PHONY: all-done build-done docs-done
.PHONY: install-tools-done install-done remove-done

# Real files, but considered always out of date.
.PHONY: build/.md5 # src/docs/src.dot

# libmini libfull
# pre-build-tests post-build-tests post-install-tests
# done
# $(BUILD_DIR)/VERSION

# -----------------------------------------------------------------------------
.SILENT:

# $(STAGE_DIR)/% Does not work as target for .SECONDARY or .PRECIOUS.
# It looks like .PRECIOUS prerequisites must match a target pattern exactly.
.PRECIOUS: $(STAGE_DIR)/bin/fmr% $(LIBFEMERA)
.PRECIOUS: $(BUILD_DIR)/external/install-%.flags
.PRECIOUS: $(BUILD_CPU)/external/install-%.flags

#FIXME fixes error make[2] unlink /home/dwagner5/local/bin/ Is a directory
#      while first running make tools?
.PRECIOUS: $(INSTALL_DIR)/bin/

# Primary named targets -------------------------------------------------------
# These are intended for users.
# Many launch parallel make jobs to simplify command-line use.
all: | intro
	$(call timestamp,$@,-$(MAKEFLAGS))
	$(MAKE) $(JSER) external
	$(MAKE) $(JPAR) mini
	#$(MAKE) test tune
	$(MAKE) $(JPAR) docs
	$(MAKE) $(JSER) install

femera: mini full
	$(call timestamp,$@,$^)

mini: | intro
	$(call timestamp,$@,$^)
	$(MAKE) $(JLIM) $(FMROUTS)
	$(MAKE) $(JPAR) build-done

tools: | intro
	$(MAKE) $(JPAR) install-tools
	$(call timestamp,$@,$^)

external: | tools
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) get-external
	$(MAKE) $(JSER) install-external
	$(MAKE) $(JPAR) external-done

install: docs | tools intro $(STAGE_TREE)
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) install-done

remove:
	$(call timestamp,$@,$^)
	-rm -rf "$(INSTALL_CPU)/share/doc/femera"
	#(MAKE) $(JPAR) remove-mini
	$(MAKE) $(JPAR) remove-tools
	$(MAKE) $(JPAR) remove-done

reinstall: | intro
	$(call timestamp,$@,$^)
	$(MAKE) $(JPAR) remove
	$(MAKE) $(JSER) install

clean: $(BUILD_CPU)/femera/ $(BUILD_CPU)/fmr/ $(STAGE_CPU)/lib/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

cleaner: $(BUILD_CPU)/femera/ $(BUILD_CPU)/fmr/ $(STAGE_CPU)/ # for make external
	$(call timestamp,$@,$^)
	-rm -rf $^ $(STAGE_DIR)/bin/fmr*
	-rm -f $(BUILD_CPU)/*/*.out $(BUILD_CPU)/*/*.err
	-rm -f $(BUILD_CPU)/*/*.flags $(BUILD_CPU)/*/*.new
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

cleanest: build/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) on $(HOSTNAME) $@)

purge:
	$(MAKE) $(JPAR) remove
	$(MAKE) $(JPAR) cleanest
	-rm -rf external/*/*

# Developer named targets =====================================================
docs: | intro build/docs/ build/src-notest.eps
	$(call timestamp,$@,)
	$(MAKE) $(JPAR) docs-done

# Internal named targets ======================================================
intro: build/copyright.txt build/docs/find-tdd-files.csv
intro: $(BUILD_CPU)/femera.flags.new
intro: | $(BUILD_TREE) $(STAGE_TREE)
ifneq ("$(NOSA_SEE)","") # Run once during a build
	$(info $(INFO) $(NOSA_SEE))
	cat external/config.*.mk > "$(BUILD_CPU)/external.config.mk"
	-rm -f "$(BUILD_DIR)/external/install-*.flags.new"
	-rm -f "$(BUILD_CPU)/external/install-*.flags.new"
	$(MAKE) $(JPAR) external-flags
ifeq ($(ENABLE_DOT),ON)
	@printf '%s' '$(MAKE_DOT)' | sed 's/\\n/\n/g' > '$(MAKE_DOTFILE)'
	@dot '$(MAKE_DOTFILE)' -Teps -o $(BUILD_DIR)/make.eps
	@dot external/external.dot -Teps -o $(BUILD_DIR)/external/external.eps
	#dot external/external.dot -Tpng -o $(BUILD_DIR)/external/external.png
	@printf '%s' '$(EXTERNAL_DOT)' | sed 's/\\n/\n/g' > '$(EXTERNAL_DOTFILE)'
	@dot '$(EXTERNAL_DOTFILE)' -Teps -o $(BUILD_DIR)/external/build-external.eps
endif
endif
ifneq ("$(ADD_TO_PATH)","")
	$(info $(NOTE) temporarily prepended PATH with:)
	$(info $(SPCS) $(ADD_TO_PATH))
endif

info: | intro
	$(info CXXFLAGS: $(CXXFLAGS))
	$(info FMRFLAGS: $(FMRFLAGS))
	$(info LDFLAGS: $(LDFLAGS))
	$(info LDLIBS: $(LDLIBS))
	$(info exported LD_LIBRARY_PATH: $(LD_LIBRARY_PATH))
	$(info exported PATH: $(PATH))
	$(MAKE) $(JPAR) build-done

docs-done: install-docs
docs-done: build/docs/femera-guide.pdf build/docs/femera-quick-start.pdf
ifeq ($(ENABLE_LYX),ON)
	$(info $(DONE) making $(FEMERA_VERSION) XHTML documentation in \
	external/docs/)
	$(info $(SPCS) and PDFs in build/docs/)
else
	$(info $(HINT) Set ENABLE_LYX to ON in config.local)
	$(info $(SPCS) to generate $(FEMERA_VERSION) documentation.)
endif
	$(info $(NOTE) $(FEMERA_VERSION) XHTML documentation is in:)
	$(info $(SPCS) $(INSTALL_CPU)/share/doc/femera/)
	$(call timestamp,$@,)

install-docs: docs/femera-guide.xhtml docs/femera-quick-start.xhtml
install-docs: LICENSE NOSA-1-3.txt README.md | $(INSTALL_CPU)/share/doc/femera/
	-cp docs/*.xhtml "$(INSTALL_CPU)/share/doc/femera/"
	-cp LICENSE "$(INSTALL_CPU)/share/doc/femera/"
	-cp NOSA-1-3.txt "$(INSTALL_CPU)/share/doc/femera/"
	-cp README.md "$(INSTALL_CPU)/share/doc/femera/"

install-done:
	$(info $(DONE) installing $(FEMERA_VERSION) on $(HOSTNAME) to:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(patsubst %,%;,$(LIST_TOOLS)))
	$(call timestamp,$@,)

remove-done:
	#rm -f $(INSTALL_CPU)/bin/mini
	$(info $(DONE) removing $(FEMERA_VERSION) on $(HOSTNAME) from:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(patsubst %,%;,$(LIST_TOOLS)))
	$(call timestamp,$@,)

build-done: build/src-notest.eps # $(BUILD_CPU)/make-build.post.test.out
	$(call timestamp,$@,$?)
	$(info $(DONE) building $(FEMERA_VERSION) with $(CXX) $(CXX_VERSION))
	$(info $(SPCS) on $(HOSTNAME) for $(CPUMODEL))
ifneq ($(HOST_MD5),$(REPO_MD5))
	$(info $(SPCS) as modified by <$(BUILT_BY_EMAIL)>)
endif
	$(info $(E_G_) tools/fmrexec.sh auto -d -t -D examples/cube.fmr)

# Femera tools ----------------------------------------------------------------
install-tools: get-bats
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) install-tools-done

clean-tools: $(BUILD_CPU)/tools/ | $(STAGE_CPU)/bin/
	$(call timestamp,$@,$^)
	-rm -rf $^ $(STAGE_CPU)/bin/fmr*

remove-tools: clean-tools | $(BUILD_DIR)/external/ $(BUILD_CPU)/external/
	$(call timestamp,$@,$<)
	-rm -f $(INSTALL_DIR)/bin/fmr* $(INSTALL_CPU)/bin/fmr*
	$(MAKE) $(JPAR) remove-tools-done

reinstall-tools:
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) remove-tools
	$(MAKE) $(JPAR) install-tools
	#grep -h . build/i7-7820HQ/tools/* build/i7-7820HQ/tests/*

test-tools:
	$(call timestamp,$@,$(LIST_TOOLS))
	$(MAKE) $(JPAR) $(TEST_TOOLS)

install-tools-done: $(TEST_TOOLS) $(INSTALL_TOOLS)
	$(MAKE) $(JPAR) $(BUILD_CPU)/external/make-install-tools.test.out

remove-tools-done: $(BUILD_DIR)/external/make-remove-tools.test.out
remove-tools-done: $(BUILD_CPU)/external/make-remove-tools.test.out

# External build tools --------------------------------------------------------

ifeq ($(ENABLE_DOT),ON)

.PHONY: $(DOT_FLAGFILE).new

external-flags: $(DOT_FLAGFILE).new

$(DOT_FLAGFILE).new:
	printf "%s" '$(DOT_FLAGS)' > $(@)
endif
ifeq ($(ENABLE_GOOGLETEST),ON)

.PHONY: $(GTEST_FLAGFILE).new

external-flags: $(GTEST_FLAGFILE).new

$(GTEST_FLAGFILE).new:
	printf "%s" '$(GTEST_FLAGS)' > $(@)
endif
ifeq ($(ENABLE_PYBIND11),ON)

.PHONY: $(PYBIND11_FLAGFILE).new

$(BUILD_DIR)/external/install-pybind11.out : | $(PYBIND11_DEPS)

external-flags: $(PYBIND11_FLAGFILE).new

$(PYBIND11_FLAGFILE).new:
	printf "%s" '$(PYBIND11_FLAGS)' > $(@)
endif
ifeq ($(ENABLE_BOOST_HEADERS),ON)

.PHONY: $(BOOST_FLAGFILE).new

external-flags: $(BOOST_FLAGFILE).new

$(PYBIND11_BOOST).new:
	printf "%s" '$(BOOST_FLAGS)' > $(@)
endif

install-bats: external/get-bats.test.sh
install-bats: get-bats external/install-bats.sh external/install-bats.test.bats
	$(call label_test,$(PASS),$(FAIL),external/get-bats.test.sh, \
	  $(BUILD_DIR)/external/get-bats.test)
	$(call label_test,$(PASS),$(FAIL), \
	  external/install-bats.sh "$(INSTALL_DIR)", \
	  $(BUILD_DIR)/external/install-bats)
	$(call label_bats,$(PASS),$(FAIL),external/install-bats.test.bats, \
	  $(BUILD_DIR)/external/install-bats.test)
	$(call timestamp,$@,$<)

get-bats:
	$(call timestamp,$@,$<)
	$(MAKE) $(JPAR) $(GET_BATS)

get-external: | intro external/tools/
	$(call timestamp,$@,)
	$(MAKE) $(JPAR) $(GET_EXTERNAL)

get-%: | intro $(BUILD_DIR)/external/
	#(call timestamp,$@,$<)
	external/get-external.sh $(*)

install-external: $(INSTALL_EXTERNAL) | intro
	$(call timestamp,$@,make $(JEXT))

install-%: $(BUILD_CPU)/external/install-%.out | intro $(BUILD_DIR)/external/
	$(call timestamp,$@,)

install-%: $(BUILD_DIR)/external/install-%.out | intro $(BUILD_CPU)/external/
	$(call timestamp,$@,)

external-done:
	$(info $(DONE) building and installing externals on $(HOSTNAME) to:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(LIST_EXTERNAL))
	$(call timestamp,$@,)

$(BUILD_DIR)/external/get-bats-%.out: external/get-external.sh
	#(info $(INFO) checking bats-$(*)...)
	-tools/label-test.sh "$(PASS)" "$(FAIL)" \
	  "external/get-external.sh bats-$(*)" \
	  "$(BUILD_DIR)/external/get-bats-$(*)"

$(BUILD_DIR)/external/get-%.out: external/get-external.sh external/get-%.dat
	#(info $(INFO) checking $(*)...)
	-tools/label-test.sh "$(PASS)" "$(FAIL)" \
	  "external/get-external.sh $(*)" \
	  "$(BUILD_DIR)/external/get-$(*)"

$(BUILD_CPU)/external/install-%.flags: $(BUILD_CPU)/external/install-%.flags.new
	tools/update-file-if-diff.sh "$(@)"
	#rm -f $(<)

$(BUILD_DIR)/external/install-%.flags: $(BUILD_DIR)/external/install-%.flags.new
	tools/update-file-if-diff.sh "$(@)"
	#rm -f $(<)

$(BUILD_CPU)/external/install-%.out: external/install-%.sh
$(BUILD_CPU)/external/install-%.out: $(BUILD_CPU)/external/install-%.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_CPU)/external/$(*)
	-tools/label-watch.sh "$(PASS)" "$(FAIL)" \
	  "external/install-$(*).sh $(INSTALL_CPU) $(<) $(JEXT)" \
	  "$(BUILD_CPU)/external/install-$(*)"

$(BUILD_DIR)/external/install-%.out: external/install-%.sh
$(BUILD_DIR)/external/install-%.out: $(BUILD_DIR)/external/install-%.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_DIR)/external/$(*)
	-tools/label-test.sh "$(PASS)" "$(FAIL)" \
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

ifeq (1,0) #FIXME No longer needed if lscpu is available
ifeq ($(ENABLE_LIBNUMA),ON)
$(STAGE_CPU)/bin/fmrnumas: $(BUILD_CPU)/tools/fmrnumas.test.out
	cp -f $(BUILD_CPU)/tools/fmrnumas "$(@)"

$(INSTALL_CPU)/bin/fmr%: $(STAGE_CPU)/bin/fmr% | $(INSTALL_CPU)/bin/
	#(info $(INFO) installing fmr$(*) to $(INSTALL_CPU)/bin/...)
	cp -f "$(STAGE_CPU)/bin/fmr$(*)" "$(@)"
endif

# CPU-specific test targets (*.test.out) --------------------------------------
ifeq ($(ENABLE_LIBNUMA),ON)
$(BUILD_CPU)/tools/fmrnumas.test.out: src/tools/fmrnumas.c tools/fmrnumas.test.bats
	$(info $(CC__) $(CC) $(<) .. -o $(BUILD_CPU)/tools/fmrnumas)
	$(CC) $(<) $(FMRFLAGS) $(LDLIBS) -o $(BUILD_CPU)/tools/fmrnumas
	$(call label_bats,$(PASS),$(FAIL),tools/fmrnumas.test.bats, \
	  $(BUILD_CPU)/tools/fmrnumas.test)
endif
endif

$(BUILD_DIR)/tests/make-remove-tools.test.out: tests/make-remove-tools.test.bats
	$(call label_bats,$(PASS),$(FAIL),DIR="$(INSTALL_DIR)/bin" $(<), \
	  $(BUILD_CPU)/tests/make-remove-tools.test)
$(BUILD_CPU)/tests/make-remove-tools.test.out: tests/make-remove-tools.test.bats
	$(call label_bats,$(PASS),$(FAIL),DIR="$(INSTALL_CPU)/bin" $(<), \
	  $(BUILD_CPU)/tests/make-remove-tools.test)
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

#NOTE Make target export requires at least Make 3.81.

$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : $(BUILD_CPU)/fmr/test.o
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.cpp src/%.hpp src/%.ipp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXGTEST) $(FMRFLAGS) $< -o $@
else
	touch $@
endif

# No .ipp
$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXGTEST) $(FMRFLAGS) $< -o $@
else
	touch $@
endif

# Header-only
$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.hpp src/%.ipp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXGTEST) $(FMRFLAGS) $< -o $@
else
	touch $@
endif

# Header-only
$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXGTEST) $(FMRFLAGS) $< -o $@
else
	touch $@
endif

$(BUILD_CPU)/%.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.o : src/%.cpp src/%.hpp src/%.ipp $(TOPDEPS)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXFLAGS) $(FMRFLAGS) $< -o $@

# Use gtest flags for this one.
$(BUILD_CPU)/%/Test.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%/Test.o : src/%/Test.cpp src/%/Test.hpp $(TOPDEPS)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXGTEST) $(FMRFLAGS) $< -o $@

$(BUILD_CPU)/%.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.o : src/%.cpp src/%.hpp $(TOPDEPS)
	#-python tools/testy/check_code_graffiti.py $^
	# rm -f $*.err $*.gtst.err
	#(info $(CXX_) $(CXX) -c $< .. -o $(notdir $@))
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXFLAGS) $(FMRFLAGS) $< -o $@

# Untested targets ------------------------------------------------------------

%.test.bats: # Continue when tests do not exist.
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))
%.test.py:
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))
%.test.sh:
	#(call label_test,$(PASS),$(FAIL),$(@),$(@))

$(BUILD_CPU)/%.test.out: # Warn if no test.
	$(info $(DBUG) did not find test: $(*).test.*)

# Library targets -------------------------------------------------------------
$(LIBFEMERA)(build/%.o) : build/%.o
	#(info $(LIBS) $(AREXE) -cr libfemera.a <-- $^)
	$(call col2lib,$(LIBS),$(AREXE) -cr libfemera.a <--,$^)
	# Serialize archive operations.
	flock "$(STAGE_CPU)/libfemera.lck" $(AREXE) -cr $(LIBFEMERA) $^
# Executable targets ----------------------------------------------------------
#(BUILD_CPU)/%.gtst _ export LD_LIBRARY_PATH = $(TMP_LIBRARY_PATH)

build/%.gtst.out : build/%.gtst
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call label_test,$(PASS),$(FAIL),$(^),$(^))
else
	echo "build/$*.o not tested: GoogleTest disabled" > $@
	echo "WARNING: build/$*.o not tested: GoogleTest disabled" > $^.err
endif

build/%.gtst : export TMPDIR := $(TEMP_DIR)
build/%.gtst : build/%.gtst.o $(LIBFEMERA)(build/%.o) \
  $(LIBFEMERA)($(BUILD_CPU)/femera/Test.o)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(notdir $@).o .. -lfemera,$(notdir $@))
	#(info $(LINK) $(CXX) $(notdir $@).o .. -lfemera .. -o $(notdir $@))
	-$(CXX) $(CXXGTEST) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	-echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

# Header-only
build/%.gtst : export TMPDIR := $(TEMP_DIR)
build/%.gtst : build/%.gtst.o $(LIBFEMERA)($(BUILD_CPU)/femera/Test.o)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(notdir $@).o .. -lfemera,$(notdir $@))
	#(info $(LINK) $(CXX) $(notdir $@).o .. -lfemera .. -o $(notdir $@))
	-$(CXX) $(CXXGTEST) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	-echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

# hm ==========================================================================
%/:
	mkdir -p $(*)

build/.md5: | build/
	touch $@
build/%/.md5: | build/%/
	touch $@

$(BUILD_CPU)/femera.flags.new:
	echo "$(CXXFLAGS) $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS)" > "$(@)"
	tools/update-file-if-diff.sh "$(BUILD_CPU)/femera.flags"
	rm $(@)

build/modification.txt: data/modification.txt build/.md5
ifeq ($(HOST_MD5),$(REPO_MD5))
	> "$@" # Empty the file.
else
	cp "data/modification.txt" "$@"
endif

build/copyright.txt: data/copyright.txt build/modification.txt
	cp "data/copyright.txt" $@
ifneq ("$(NOSA_SEE)","")
	printf "$(INFO_COLOR)";
	cat build/modification.txt build/copyright.txt \
	  | tr '\n' ' ' | tr -s ' ' | fold -s -w 80
	printf "$(NORM_COLOR)\n";
endif

build/test-files.txt: tools/list-test-files.sh build/.md5
	-tools/list-test-files.sh > $@

src/docs/src.dot: build/.md5
	@external/tools/cinclude2dot --src src >$@ 2>build/src.dot.err
	@dot $@ -Gsize="10.0,8.0" -Teps -o build/src-test.eps
          #  --groups is nice, too
	#(info $(INFO) Source dependencies: $@)

src/docs/src-notest.dot: src/docs/src.dot
	@grep -v '\.gtst\.' $< > $@
	#(info $(INFO) Dependencies without tests: $@)

build/src-notest.eps: src/docs/src-notest.dot tools/src-inherit.sh
ifeq ($(ENABLE_DOT),ON)
	@tools/src-inherit.sh
	@dot $< -Gsize="10.0,8.0" -Teps -o $@
	#  -Gratio="fill" -Gsize="11.7,8.267!" -Gmargin=0
	$(info $(INFO) dependency graph: $@)
endif

build/docs/tdd-tests.txt: tools/list-tdd-tests.sh build/docs/.md5
	-tools/list-tdd-tests.sh src/docs/*.tex src/docs/*.lyx > $@

build/docs/find-tdd-files.csv: build/docs/tdd-tests.txt build/test-files.txt
build/docs/find-tdd-files.csv: tools/compare-lists.py
	-tools/compare-lists.py build/docs/tdd-tests.txt build/test-files.txt \
	  >$@ 2>build/docs/find-tdd-files.err

ifeq ($(ENABLE_LYX),ON)
build/docs/%.pdf: src/docs/%.lyx src/docs/quick-start.tex
	-tools/label-test.sh "$(EXEC)" "$(FAIL)" \
	  "lyx -batch -n -f all -E pdf $(@) $(<)" \
	  "build/docs/$(*)-pdf"
else
build/docs/%.pdf:
	echo "Documentation is not yet available." > "build/docs/$(*)-pdf.out"
endif

ifeq ($(ENABLE_LYX),ON)
docs/%.xhtml: src/docs/%.lyx src/docs/quick-start.tex
	-tools/label-test.sh "$(PASS)" "$(FAIL)" \
	  "lyx -batch -n -f all -E xhtml $(@) $(<)" \
	  "build/docs/$(*)-xhtml"
else
docs/%.xhtml:
	echo "Documentation is not yet available." > "build/docs/$(*)-xhtml.out"
endif
