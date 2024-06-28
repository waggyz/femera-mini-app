#!/usr/bin/make
.DEFAULT_GOAL:=all
SHELL        :=bash

include examples/config.recommended
-include config.local

include tools/set-undefined.mk
include tools/functions.mk
#------------------------------------------------------------------------------
BUILD_VERSION  :=$(shell tools/build-version-number)
FEMERA_VERSION :=Femera $(BUILD_VERSION)

BUILT_BY_EMAIL :=$(call parse_email,$(BUILT_BY))

IS_IN_REPO     :=$(shell git rev-parse --is-inside-work-tree 2>/dev/null)
# contains "true" or ""

# Check if there is altered content in src/ data/ tools/
# or some content in extras/ even when not a git repository.
REPO_MD5   :=$(shell cat .md5)
ifeq ($(shell test -e file_name && echo -n yes),yes)
  #(shell tools/md5-all build/.md5)
  $(shell echo "host hash" > build/.md5)
  HOST_MD5 :=$(shell cat build/.md5)
endif
#TODO Move to recipe?

# Directories -----------------------------------------------------------------
FMRDIR     :=$(shell pwd)
# Directories for generic components
BUILD_DIR  :=build
STAGE_DIR  :=$(BUILD_DIR)/stage
INSTALL_DIR:=$(PREFIX)

# Directories for CPU-specific components
BUILD_CPU  :=$(BUILD_DIR)/$(CPUMODEL)
STAGE_CPU  :=$(STAGE_DIR)/$(CPUMODEL)
INSTALL_CPU:=$(INSTALL_DIR)/$(CPUMODEL)

# Export TMPDIR to avoid system tmpdir overflow. (MPI needs it, maybe others.)
TEMP_DIR   :=$FMRDIR/$(BUILD_DIR)/tmp

#NOTE Subdirectories needed in the build directory have a trailing slash.
BUILD_TREE += $(BUILD_DIR)/external/tools/
BUILD_TREE += $(BUILD_DIR)/docs/

BUILD_TREE += $(BUILD_CPU)/tmp/
BUILD_TREE += $(BUILD_CPU)/external/
BUILD_TREE += $(BUILD_CPU)/tests/
BUILD_TREE += $(BUILD_CPU)/tools/
BUILD_TREE += $(BUILD_CPU)/femera/proc/
BUILD_TREE += $(BUILD_CPU)/femera/data/
BUILD_TREE += $(BUILD_CPU)/femera/test/
BUILD_TREE += $(BUILD_CPU)/femera/task/
BUILD_TREE += $(BUILD_CPU)/fmr/perf/
BUILD_TREE += $(BUILD_CPU)/zyclops/

STAGE_TREE += $(STAGE_DIR)/bin/ $(STAGE_DIR)/lib/
STAGE_TREE += $(STAGE_CPU)/bin/ $(STAGE_CPU)/lib/
#------------------------------------------------------------------------------
ifeq ($(CXX),g++)
  ifeq ($(ENABLE_MPI),ON)
    CXX      :=mpic++
    # TDDEXEC:=/bin/time
    # TDDEXEC:=mpiexec -n $(TDD_MPI_N) --bind-to core -map-by node:pe=$(TDD_OMP_N)
  endif
  #TODO replace below with data/gcc8.flags
  OPTFLAGS   := $(shell cat data/gcc4.flags | tr '\n' ' ' | tr -s ' ')
  CXXFLAGS   += -std=c++11 -g -MMD -MP -fPIC
  # Dependency file generation: -MMD -MP
  #WAS: -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
  #NOTE -fPIC (or -fpic?) needed for shared libs, but may degrade static lib performance.
  #TODO Consider finding all dependency static libs and building -static.
  ifeq ($(ENABLE_LTO),ON)
    CXXFLAGS += -flto
  endif
  ifeq ($(ENABLE_OMP),ON)
    CXXFLAGS += -D_GLIBCXX_PARALLEL
    CXXFLAGS += -fopenmp
  endif
  ifeq ($(ENABLE_GCC_PROFILE),ON)
  # CXXFLAGS += -fprofile-abs-path fprofile-dir=$(BUILD_CPU)/%p/
    CXXFLAGS += -fprofile-dir=$(abspath $(BUILD_CPU))
    CXXFLAGS += -fprofile-generate
  endif
  CXXFLAGS   += $(OPTFLAGS) -fstrict-enums
  # Warning flags
  CXXWARNS   += -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wfloat-equal
  CXXWARNS   += -Wdouble-promotion -Wconversion -Wsign-conversion -Wlogical-op
  CXXWARNS   += -Wcast-qual -Wcast-align -Woverloaded-virtual -Wundef
  CXXWARNS   += -Wmissing-declarations -Wredundant-decls -Wunused-macros
  CXXWARNS   += -Wzero-as-null-pointer-constant -Wstrict-null-sentinel -Weffc++
  CXXWARNS   += -Wdisabled-optimization
  CXXWARNS   += -Winline
  #
  # CXXFLAGS+= -finline-limit=1000
  # CXXFLAGS+= --param inline-min-speedup=2
  # CXXFLAGS+= --param inline-unit-growth=500
  # CXXFLAGS+= --param large-function-growth=2000
  # Library archiver
  AREXE      :=gcc-ar
endif
#TODO To use static libraries instead of dynamic when both are available, use:
# <full path>/lib<name>.a instead of -l<name>
# in LDLIBS.
ifeq ($(CXX),icx)
  OPTFLAGS   := $(shell cat data/icpc.flags | tr '\n' ' ' | tr -s ' ')
  CXXFLAGS   := c++11 -restrict -g $(OPTFLAGS)
  #NOTE -fPIC needed for shared libs, but may degrade static lib performance.
  ENABLE_GCC_PROFILE :=OFF
  # CXXWARNS   += -Wall -Wextra -Wshadow
  ifeq ($(ENABLE_MKL),ON)
    LDLIBS   += -mkl=sequential
    CXXFLAGS += -DMKL_DIRECT_CALL_SEQ
  endif
  AREXE      :=xiar
endif
CXX_VERSION  :=$(shell $(CXX) -dumpversion)
# Flags for compiling tests
CXXTESTS     :=$(CXXFLAGS) $(CXXWARNS)
CXXPERFS     :=$(CXXFLAGS) $(CXXWARNS)
FMRFLAGS     += -DFMR_VERSION="$(FEMERA_VERSION)"
FMRFLAGS     += -DFMR_CPUMODEL="$(CPUMODEL)"
ifneq ($(CPUCOUNT),)
  FMRFLAGS   += -DFMR_CORE_N=$(CPUCOUNT)
endif
ifneq ($(FMR_LOCAL_INT),)
  FMRFLAGS   += -DFMR_LOCAL_INT=$(FMR_LOCAL_INT)
endif
ifneq ($(FMR_GLOBAL_INT),)
  FMRFLAGS   += -DFMR_GLOBAL_INT=$(FMR_GLOBAL_INT)
endif
ifneq ($(FMR_MICRO_UCHAR),)
  FMRFLAGS   += -DFMR_MICRO_UCHAR=$(FMR_MICRO_UCHAR)
endif
ifneq ($(FMR_TIMES_UCHAR),)
  FMRFLAGS   += -DFMR_TIMES_UCHAR=$(FMR_TIMES_UCHAR)
endif
INCFLAGS     += -I"$(STAGE_CPU)/include"
INCFLAGS     += -I"$(STAGE_DIR)/include"
INCFLAGS     += -isystem"$(INSTALL_CPU)/include"
INCFLAGS     += -isystem"$(INSTALL_DIR)/include"
LDFLAGS      += -L"$(STAGE_CPU)/lib"
LDFLAGS      += -L"$(STAGE_DIR)/lib"
# -----------------------------------------------------------------------------
ifeq ("$(findstring $(INSTALL_DIR)/bin:,$(PATH):)","")
  ADD_TO_PATH   :=$(INSTALL_DIR)/bin:$(ADD_TO_PATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/bin:,$(PATH):)","")
  ADD_TO_PATH   :=$(INSTALL_CPU)/bin:$(ADD_TO_PATH)
endif
ifneq ("$(ADD_TO_PATH)","")# only true once during build
  export PATH   :=$(ADD_TO_PATH):$(PATH)
  #NOTE export does not work for this when in a recipe.
endif

ifeq ("$(findstring $(INSTALL_DIR)/lib64:,$(LD_RUN_PATH):)","")
  ADD_TO_LDPATH :=$(INSTALL_DIR)/lib64:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_DIR)/lib:,$(LD_RUN_PATH):)","")
  ADD_TO_LDPATH :=$(INSTALL_DIR)/lib:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/lib64:,$(LD_RUN_PATH):)","")
  ADD_TO_LDPATH :=$(INSTALL_CPU)/lib64:$(ADD_TO_LDPATH)
endif
ifeq ("$(findstring $(INSTALL_CPU)/lib:,$(LD_RUN_PATH):)","")
  ADD_TO_LDPATH :=$(INSTALL_CPU)/lib:$(ADD_TO_LDPATH)
endif
ifeq ($(ENABLE_MKL),ON)
  ifeq ("$(findstring $(INSTALL_CPU)/mkl/2021.4.0/lib/intel64:,$(LD_RUN_PATH):)","")
    ADD_TO_LDPATH :=$(INSTALL_CPU)/mkl/2021.4.0/lib/intel64:$(ADD_TO_LDPATH)
  endif
endif
ifneq ("$(ADD_TO_LDPATH)","")# only true once during build
  export LD_RUN_PATH     :=$(LD_RUN_PATH):$(ADD_TO_LDPATH)
  export LD_LIBRARY_PATH :=$(LD_LIBRARY_PATH):$(ADD_TO_LDPATH)
  export OMP_PROC_BIND=spread
  export OMP_PLACES=cores;
endif

# TMP_LIBRARY_PATH_=$(LD_RUN_PATH);
ifeq ("$(findstring $(INSTALL_CPU)/lib -L,$(LDFLAGS) -L)","")
  LDFLAGS   += -L"$(INSTALL_CPU)/lib"
endif
ifeq ("$(findstring $(INSTALL_CPU)/lib64 -L,$(LDFLAGS) -L)","")
  LDFLAGS   += -L"$(INSTALL_CPU)/lib64"
endif
ifeq ("$(findstring $(INSTALL_DIR)/lib -L,$(LDFLAGS) -L)","")
  LDFLAGS   += -L"$(INSTALL_DIR)/lib"
endif
ifeq ("$(findstring $(INSTALL_DIR)/lib64 -L,$(LDFLAGS) -L)","")
  LDFLAGS   += -L"$(INSTALL_DIR)/lib64"
endif
ifeq ($(ENABLE_MKL),ON)
  ifeq ("$(findstring $(INSTALL_CPU)/mkl/2021.4.0/lib/intel64 -L,$(LDFLAGS) -L)","")
    LDFLAGS += -L"$(INSTALL_CPU)/mkl/2021.4.0/lib/intel64"
  endif
endif
ifeq ("$(FMR_COPYRIGHT)","")# only true once during build
  export FMR_COPYRIGHT := cat data/copyright.txt | tr '\n' ' ' | tr -s ' '
  NOSA_SEE := See the NASA open source agreement (LICENSE) file for details.
endif

-include $(BUILD_CPU)/external-config.mk

#NOTE CGNS, FLTK, FreeType, PETSc build in external/*/
# Libraries and applications available ----------------------------------------
ifeq ($(ENABLE_ZYCLOPS),ON)
  EXTERNAL_DOT+="Femera" -> "Zyclops" [color="cyan"]\n
  FMRFLAGS+= -DFMR_HAS_ZYCLOPS
endif
  ifeq ($(ENABLE_PYMERA),ON)
    EXTERNAL_DOT+="Femera" -> "pymera" [color="cyan"] [style="dotted"]\n
  endif
ifeq ($(ENABLE_OMP),ON)
  EXTERNAL_DOT+="Femera" -> "OpenMP"\n
  # FMRFLAGS+= -DFMR_HAS_OMP not needed; OpenMP defines _OPENMP:
endif
ifeq ($(ENABLE_NVIDIA),ON)
#TODO move to external.config-nvidia.mk?
  ifeq ("$(NVIDIA_DIR)","")
    NVIDIA_DIR:="/usr/local/cuda-11.6"
  endif
  # EXTERNAL_DOT+="Femera" -> "NVIDIA"\n
  FMRFLAGS+= -DFMR_HAS_NVIDIA
  CUXX:=nvcc
  INCFLAGS+= -isystem"$(NVIDIA_DIR)/include"
  LDFLAGS+= -L"$(NVIDIA_DIR)/lib64"
  LDLIBS+= -lcuda -lcudart
  CUFLAGS+= --std=c++11 -g -MMD -MP -O3
  CUFLAGS+= -DFMR_HAS_NVIDIA
  CUFLAGS+= -L"$(NVIDIA_DIR)/lib64" -lcuda -lcudart
endif
ifeq ($(ENABLE_MPI),ON)
  #TODO lstopo --of xml "$FMRDIR/share/mpich-hwloc.xml"
  EXTERNAL_DOT+="Femera" -> "MPI"\n
  #TODO check for libs before setting FMR_HAS_MPI?
  FMRFLAGS+= -DFMR_HAS_MPI
  LDLIBS+= -lmpi
  ifeq ($(ENABLE_VALGRIND),ON)# for valgrind with OpenMPI
    VGMPISUPP:=$(shell tools/valgrind-mpi-supp)
  endif
endif
ifeq ($(ENABLE_LIBNUMA),ON)
  EXTERNAL_DOT+="Femera" -> "libnuma"\n
  FMRFLAGS+= -DFMR_HAS_LIBNUMA
  LDLIBS+= -lnuma
endif
# Build tools to download and install -----------------------------------------
ifeq ($(ENABLE_BATS),ON)
  MAKE_DOT+="Makefile" -> "bats"\n
  BATS_MODS:= bats-core bats-support bats-assert bats-file # bats-mock
  label_bats = $(call label_test,$(1),$(2),$(3),$(4))
else
  label_bats = $(info \
    $(DBUG) Set ENABLE_BATS:=ON in config.local for $(notdir $(3)).)
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  CXXTESTS := $(filter-out -Winline,$(CXXTESTS))
  CXXTESTS := $(filter-out -Weffc++,$(CXXTESTS))
endif
ifeq ($(ENABLE_GOOGLETEST),ON-FIXME-DISABLED)
  CXXTESTS := $(filter-out -Wundef,$(CXXTESTS))
  CXXTESTS := $(filter-out -flto,$(CXXTESTS))
  #NOTE Parallel library build breaks link-time optimization of gtests. (gcc 4.8)
  # CXXTESTS := $(filter-out -fprofile-generate,$(CXXTESTS))
  #
  CXXPERFS := $(filter-out -Weffc++,$(CXXPERFS))
  # CXXPERFS := $(filter-out -Winline,$(CXXPERFS))
  CXXPERFS := $(filter-out -fprofile-generate,$(CXXPERFS))
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  LIST_EXTERNAL += googletest
  FMRFLAGS+= -DFMR_HAS_GTEST
  FMRFLAGS+= -DFMR_GTEST_VERSION=$(shell external/get-googletest-version)
  EXTERNAL_DOT+="Femera" -> "GoogleTest"\n
#  MAKE_DOT+="Makefile" -> "GoogleTest"\n
#  GTEST_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  LDLIBS += -lpthread -lgtest -lgmock
#  GTEST_FLAGFILE:= $(BUILD_CPU)/external/googletest-install.flags
endif
ifeq ($(ENABLE_PYBIND11),ON)
#  LIST_EXTERNAL += pybind11
  INSTALL_EXTERNAL+= $(BUILD_DIR)/external/pybind11-install.out
#  PYBIND11_REQUIRES += boost-headers
  EXTERNAL_DOT+="Femera" -> "pybind11" [color="cyan"]\n
  # BUILD_TREE += $(BUILD_DIR)/external/pybind11/
  EXTERNAL_DOT+="pybind11" -> "Boost"\n
  EXTERNAL_DOT+="pybind11" -> "Python"\n
  ifeq ($(ENABLE_PYMERA),ON)
    EXTERNAL_DOT+="pymera" -> "pybind11"\n
  endif
  # FMRFLAGS += -DFMR_HAS_PYBIND11
  PYBIND11_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_DIR)"
  PYBIND11_FLAGS += -DDOWNLOAD_CATCH=0
#  PYBIND11_FLAGS += -DPYBIND11_PYTHON_VERSION=3.6
  PYBIND11_FLAGS += -DPYBIND11_HAS_OPTIONAL=0
  PYBIND11_FLAGS += -DPYBIND11_HAS_EXP_OPTIONAL=0
  PYBIND11_FLAGS += -DPYBIND11_HAS_VARIANT=0
  #FIXME Find include and lib dirs automatically
#  PYBIND11_FLAGS += -DBOOST_ROOT:PATHNAME=/usr/local/pkgs-modules/boost_1.66.0
  PYBIND11_FLAGFILE:= $(BUILD_DIR)/external/pybind11-install.flags
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
#TODO Remove following 2 lines.
EXTERNAL_DOT+="Femera" -> "CUDA" [style=dotted]\n
EXTERNAL_DOT+="Femera" -> "ParaView" [style=dotted]\n
ifeq ($(ENABLE_PARAVIEW),ON)
  #TODO move to external/paraview-* files
  LIST_EXTERNAL += paraview
  FMRFLAGS+= -DFMR_HAS_PARAVIEW
  EXTERNAL_DOT+="Femera" -> "ParaView" [color="cyan"] [style=dotted]\n
endif
ifeq ($(ENABLE_POVRAY),ON)
  #TODO move to external/povray-* files
  LIST_EXTERNAL += povray
  FMRFLAGS+= -DFMR_HAS_POVRAY
  EXTERNAL_DOT+="Femera" -> "POV-Ray" [color="cyan"] [style=dotted]\n
  EXTERNAL_DOT+="Femera" -> "OpenEXR" [color="cyan"] [style=dotted]\n
  EXTERNAL_DOT+="Femera" -> "SDL" [color="cyan"] [style=dotted]\n
  EXTERNAL_DOT+="POV-Ray" -> "Boost"\n
  EXTERNAL_DOT+="POV-Ray" -> "zlib"\n
  EXTERNAL_DOT+="POV-Ray" -> "libpng"\n
  EXTERNAL_DOT+="POV-Ray" -> "giflib"\n
  EXTERNAL_DOT+="POV-Ray" -> "libtiff"\n
  EXTERNAL_DOT+="POV-Ray" -> "libjpeg"\n
  EXTERNAL_DOT+="POV-Ray" -> "OpenEXR"\n
  EXTERNAL_DOT+="OpenEXR" -> "lmath" [color="green"]\n
  EXTERNAL_DOT+="POV-Ray" -> "SDL"\n
  EXTERNAL_DOT+="POV-Ray" -> "pthreads"\n
endif
ifeq ($(ENABLE_NEPER),ON)
  #TODO move to external/neper-* files
  LIST_EXTERNAL += neper
  LIST_EXTERNAL += gsl
  FMRFLAGS+= -DFMR_HAS_NEPER
  EXTERNAL_DOT+="Femera" -> "Neper" [color="cyan"] [style=dotted]\n
  # EXTERNAL_DOT+="Neper" -> "GSL" [color="blue"]\n
  EXTERNAL_DOT+="Neper" -> "Scotch" [color="green"]\n
  EXTERNAL_DOT+="Neper" -> "NLopt" [color="green"]\n
  EXTERNAL_DOT+="Neper" -> "pthreads"\n
  EXTERNAL_DOT+="Neper" -> "Gmsh"\n
  ifeq ($(ENABLE_POVRAY),ON)
    EXTERNAL_DOT+="Neper" -> "POV-Ray"\n
  endif
endif
# Developer tools
SRC_STAT_FILE:=data/src/femera-$(CPUMODEL)-build-stats.csv
ifeq ($(ENABLE_DOT),ON)
  ifeq ($(shell which dot 2>/dev/null),"")# dot is part of graphviz
    ENABLE_DOT:=OFF
  else
    MAKE_DOT+="Makefile" -> "dot"\n
    HEAD_DOT+=overlap=scale;\n
    HEAD_DOT+=size="16,8";\n
    HEAD_DOT+=ratio="fill";\n
    HEAD_DOT+=fontsize="14";\n
    HEAD_DOT+=fontname="Helvetica";\n
    HEAD_DOT+=clusterrank="local";\n
    #
    LEGEND_DOT+={
    #LEGEND_DOT+=subgraph cluster_01 { \n
    #LEGEND_DOT+=graph [labelloc="b" labeljust="l" label="Legend"];
    #LEGEND_DOT+=rankdir=LR\n
    LEGEND_DOT+=node [shape=plaintext]\n
    #LEGEND_DOT+=  labelloc="b" labeljust="l" label="Legend";\n
    LEGEND_DOT+=  key [label=<<table border="0" cellpadding="2" cellspacing="0" cellborder="0">\n
    LEGEND_DOT+=    <tr><td align="right" port="i1">Installs during Femera build</td></tr>\n
    LEGEND_DOT+=    <tr><td align="right" port="i2">Installs during PETSc config</td></tr>\n
    LEGEND_DOT+=    <tr><td align="right" port="i3">Uses</td></tr>\n
    LEGEND_DOT+=    <tr><td align="right" port="i4">Not implemented yet</td></tr>\n
    LEGEND_DOT+=    </table>>]\n
    LEGEND_DOT+=  key2 [label=<<table border="0" cellpadding="2" cellspacing="0" cellborder="0">\n
    LEGEND_DOT+=    <tr><td port="i1">&nbsp;</td></tr>\n
    LEGEND_DOT+=    <tr><td port="i2">&nbsp;</td></tr>\n
    LEGEND_DOT+=    <tr><td port="i3">&nbsp;</td></tr>\n
    LEGEND_DOT+=    <tr><td port="i4">&nbsp;</td></tr>\n
    LEGEND_DOT+=    </table>>]\n
    LEGEND_DOT+=  key:i1:e -> key2:i1:w [color=cyan] {rank=same; key, key2 }\n
    LEGEND_DOT+=  key:i2:e -> key2:i2:w [color=blue] {rank=same; key, key2 }\n
    LEGEND_DOT+=  key:i3:e -> key2:i3:w {rank=same; key, key2 }\n
    LEGEND_DOT+=  key:i4:e -> key2:i4:w [style="dotted"] {rank=same; key, key2 }\n
    LEGEND_DOT+=}\n
    #LEGEND_DOT+=}
  endif
  DOT_FLAGFILE:=$(BUILD_DIR)/external/cinclude2dot-install.flags
endif
ifeq ($(ENABLE_DOT),ON)
  #TODO REMOVE GET_EXTERNAL+= $(BUILD_DIR)/external/cinclude2dot-get.out
  INSTALL_EXTERNAL+= $(BUILD_DIR)/external/cinclude2dot-install.out
  MAKE_DOT+="Makefile" -> "cinclude2dot"\n
  EXTERNAL_DOT:= digraph "Femera external dependencies as built" \
    {\n $(HEAD_DOT) $(LEGEND_DOT) $(EXTERNAL_DOT) }\n
  #EXTERNAL_DOT+=$(shell cat external/build-external-legend.dot)
  MAKE_DOT:= digraph "Makefile dependencies" {\n $(HEAD_DOT) $(MAKE_DOT) }\n
  EXTERNAL_DOTFILE:= $(BUILD_DIR)/external/external.dot
  MAKE_DOTFILE:= $(BUILD_DIR)/make.dot
endif

ifeq ($(ENABLE_VALGRIND),ON)
  VALGRIND_SUPP := valgrind.supp
  VALGRIND_SUPP_EXE := $(BUILD_CPU)/$(VALGRIND_SUPP).exe
  VGMPI := fmrexec tdd
  VGSUPP_FLAGS += --suppressions=$(BUILD_CPU)/$(VALGRIND_SUPP)
  # Use (mostly) the same flags to compile the suppression file.
  VGFLAGS := $(CXXFLAGS) $(filter-out -Winline,$(CXXWARNS))
ifeq ($(FMR_MPI_TYPE),MPICH)
  # This is for MPICH.
  VGSUPP := valgrind --leak-check=full --show-reachable=yes --error-limit=no \
    --show-leak-kinds=all --gen-suppressions=all \
    mpiexec -n $(TDD_MPI_N)                      \
    $(VALGRIND_SUPP_EXE) 3>&1 1>&2 2>&3          \
    | tools/grindmerge.pl -f $(VGMPISUPP)        \
    > $(BUILD_CPU)/$(VALGRIND_SUPP) 2>/dev/null;
  VGEXEC := valgrind --leak-check=full --track-origins=yes        \
    $(VGSUPP_FLAGS) --log-file=$(BUILD_CPU)/mini.valgrind.log     \
    $(VGMPI):$(BUILD_CPU)/mini -n$(TDD_OMP_NP) -v0 $(TDD_FMRFILE) \
    > $(BUILD_CPU)/mini.valgrind.out; #\
    #sed -i '/invalid file descriptor 10[0-4][0-9] /d'      \
    #  $(BUILD_CPU)/mini.valgrind.log;                      \
    #sed -i '/invalid file descriptor 2[56][0-9] /d'        \
    #  $(BUILD_CPU)/mini.valgrind.log;                      \
    #sed -i '/select an alternative log fd/d'               \
    #  $(BUILD_CPU)/mini.valgrind.log
else
  # This is for OpenMPI
  VGSUPP := valgrind --leak-check=full --show-reachable=yes --error-limit=no \
    --show-leak-kinds=all --gen-suppressions=all \
    mpiexec -n $(TDD_MPI_N) --bind-to core -map-by node:pe=$(TDD_OMP_N) \
    $(VALGRIND_SUPP_EXE) 3>&1 1>&2 2>&3 \
    | tools/grindmerge.pl -f $(VGMPISUPP) \
    > $(BUILD_CPU)/$(VALGRIND_SUPP) 2>/dev/null;
  VGEXEC := valgrind --leak-check=full --track-origins=yes         \
    $(VGSUPP_FLAGS) --log-file=$(BUILD_CPU)/mini.valgrind.log      \
    $(VGMPI):$(BUILD_CPU)/mini -n$(TDD_OMP_NP) -v0 $(TDD_FMRFILE) \
    > $(BUILD_CPU)/mini.valgrind.out; \
    sed -i '/invalid file descriptor 10[0-4][0-9] /d'      \
      $(BUILD_CPU)/mini.valgrind.log;                      \
    sed -i '/invalid file descriptor 2[56][0-9] /d'        \
      $(BUILD_CPU)/mini.valgrind.log;                      \
    sed -i '/select an alternative log fd/d'               \
      $(BUILD_CPU)/mini.valgrind.log
endif
endif

#TODO CXXMINI flags should show -Winline errors?
CXXMINI := $(CXXFLAGS) $(FMRFLAGS) $(filter-out -Winline,$(CXXWARNS))
CXXFLAGS+= $(CXXWARNS)

# Files -----------------------------------------------------------------------
# Generic Femera tools
LIST_TOOLS:= fmrmodel fmrcores fmrexec fmrnumas
INSTALL_TOOLS:= $(patsubst %,$(INSTALL_DIR)/bin/%,$(LIST_TOOLS))

# External packages
#TODO REMOVE GET_EXTERNAL+= $(patsubst %,$(BUILD_DIR)/external/get-%.out,$(LIST_EXTERNAL))
INSTALL_EXTERNAL+= $(patsubst %,$(BUILD_CPU)/external/%-install.out, \
  $(LIST_EXTERNAL))

#TODO REMVOE GET_BATS:= $(patsubst %,%-get,$(BATS_MODS))

LIBFEMERA:=$(STAGE_CPU)/lib/libfemera.a

# Changing any of these should cause a full Femera rebuild.
TOPDEPS += Makefile config.local $(BUILD_CPU)/femera.flags

# C++11 code to compile and gtest
FMRCPPS:= $(shell find src/ -maxdepth 4 -name "*.cpp" -printf "%p ")
# FMRCUS:= $(shell find src/ -maxdepth 4 -name "*.cu.cpp" -printf "%p ")
DEPFILE:= $(patsubst src/%.cpp,$(BUILD_CPU)/%.d,$(FMRCPPS))
# DEPFILE+= $(patsubst src/%.cu.cpp,$(BUILD_CPU)/%.d,$(FMRCUS))
FMRGTST:= $(filter %.gtst.cpp,$(FMRCPPS))
FMRPERF:= $(filter %.perf.cpp,$(FMRCPPS))
FMROUTS:= $(patsubst src/%.gtst.cpp,$(BUILD_CPU)/%.gtst.out,$(FMRGTST))
PRFOUTS:= $(patsubst src/%.perf.cpp,$(BUILD_CPU)/%.perf.out,$(FMRPERF))

# make recipes ================================================================
# Handle dependancies generated by CXX -MMD -MP ...
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
-include $(wildcard $(DEPFILE))
# wildcard avoids failing on non-existent files
#NOTE This needs to be before "no test" recipes.

# These .PHONY targets are intended for users.
# external libfemera test tune
.PHONY: all tools external mini femera install
.PHONY: remove reinstall clean cleaner cleanest purge
# -----------------------------------------------------------------------------
# The rest are for developers...
.PHONY: docs hash patch perf
.PHONY: install-tools test-tools remove-tools reinstall-tools clean-tools
.PHONY: install-bats # install-external
# ...and internal makefile use.
.PHONY: intro info
.PHONY: all-done build-done docs-done perf-done
.PHONY: install-tools-done install-done remove-done

# Real files, but considered always out of date.
.PHONY: build/.md5 
# src/docs/src.dot
.PHONY: code-stats $(SRC_STAT_FILE)

# libmini libfull
# pre-build-tests post-build-tests post-install-tests
# done
# $(BUILD_DIR)/VERSION

# -----------------------------------------------------------------------------
.SILENT:

# $(STAGE_DIR)/% Does not work as target for .SECONDARY or .PRECIOUS.
# It looks like .PRECIOUS prerequisites must match a target pattern exactly.
.PRECIOUS: $(STAGE_DIR)/bin/fmr% $(LIBFEMERA)
.PRECIOUS: $(BUILD_DIR)/external/%-install.flags
.PRECIOUS: $(BUILD_CPU)/external/%-install.flags

#TODO fixes error make[2] unlink /home/dwagner5/.local/bin/ Is a directory
#     while first running make tools?
.PRECIOUS: $(INSTALL_DIR)/bin/ build/%.gtst build/%.perf

# Compile and add the exception handler (Errs) to libfemera early.
$(BUILD_CPU)/femera/Test.o: $(LIBFEMERA)($(BUILD_CPU)/femera/Errs.o)

# Primary named targets -------------------------------------------------------
# These are intended for users.
# Many launch serial or parallel make jobs to simplify command-line use.
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
	$(MAKE) $(JLIM) $(BUILD_CPU)/mini
	$(MAKE) $(JPAR) build-done

perf: | intro
	$(call timestamp,$@,$^)
	$(MAKE) $(JLIM) $(FMROUTS)
	$(MAKE) $(JLIM) $(BUILD_CPU)/mini
	$(MAKE) $(JLIM) $(PRFOUTS) #TODO *.perf executable should run serially
	$(MAKE) $(JPAR) perf-done

tools: | intro
	$(MAKE) $(JPAR) install-tools
	$(call timestamp,$@,$^)

external: | tools
	$(call timestamp,$@,$^)
	#TODO REMOVE $(MAKE) $(JPAR) get-external
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
clean: $(BUILD_CPU)/zyclops/ $(BUILD_CPU)/build/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) $@)

cleaner: $(BUILD_CPU)/femera/ $(BUILD_CPU)/fmr/ $(STAGE_CPU)/
	# for make external
	$(call timestamp,$@,$^)
	-rm -rf $^ $(STAGE_DIR)/bin/fmr*
	-rm -f $(BUILD_CPU)/*/*.out $(BUILD_CPU)/*/*.err
	-rm -f $(BUILD_CPU)/*/*.flags $(BUILD_CPU)/*/*.new
	$(info $(DONE) made $(FEMERA_VERSION) for $(CPUMODEL) $@)

cleanest: build/
	$(call timestamp,$@,$^)
	-rm -rf $^
	$(info $(DONE) made $(FEMERA_VERSION) $@)

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
ifeq ($(ENABLE_MPI),ON)
intro: $(BUILD_CPU)/sizeof-mpi-comm.exe
endif
ifneq ("$(NOSA_SEE)","") # Run once during a build
	$(info $(INFO) $(NOSA_SEE))
	cat external/*-config.mk > "$(BUILD_CPU)/external-config.mk"
	-rm -f "$(BUILD_DIR)/external/*-install.flags.new"
	-rm -f "$(BUILD_CPU)/external/*-install.flags.new"
ifneq ("$(external-flags)","")
	$(MAKE) $(JPAR) external-flags
endif
	
ifeq ($(ENABLE_DOT),ON)
	@printf '%s' '$(MAKE_DOT)' | sed 's/\\n/\n/g' > '$(MAKE_DOTFILE)'
	@dot '$(MAKE_DOTFILE)' -Teps -o $(BUILD_DIR)/make.eps
	# @dot external/external.dot -Teps -o $(BUILD_DIR)/external/external.eps
# ifeq ($(ENABLE_DOT_PNG),ON)
	# @dot external/external.dot -Tpng -o $(BUILD_DIR)/external/external.png
# endif
	@printf '%s' '$(EXTERNAL_DOT)' | sed 's/\\n/\n/g' > '$(EXTERNAL_DOTFILE)'
	#@cat 'external/build-external-legend.dot' >> '$(EXTERNAL_DOTFILE)'
	@dot '$(EXTERNAL_DOTFILE)' -Teps -o $(BUILD_DIR)/external/external.eps
ifeq ($(ENABLE_DOT_PNG),ON)
	@dot '$(EXTERNAL_DOTFILE)' -Tpng -o $(BUILD_DIR)/external/external.png
endif
endif
endif
ifneq ("$(ADD_TO_PATH)","")
	$(info $(NOTE) temporarily prepended PATH with:)
	$(info $(SPCS) $(ADD_TO_PATH))
endif

info: | intro docs
	printf "CXXFLAGS:\n$(CXXFLAGS)\n"
ifeq ("$(ENABLE_NVIDIA)","ON")
	printf "\nCUFLAGS:\n$(CUFLAGS)\n"
endif
	printf "\nFMRFLAGS:\n"
	echo "$(FMRFLAGS)"
	printf "\nINCFLAGS:\n$(INCFLAGS)\n"
	printf "\nLDFLAGS:\n$(LDFLAGS)\n"
	printf "\nLDLIBS:\n$(LDLIBS)\n"
	printf "\nLD_RUN_PATH:\n$(LD_RUN_PATH)\n"
	printf "\nLD_LIBRARY_PATH:\n$(LD_LIBRARY_PATH)\n"
	printf "\nPATH:\n$(PATH)\n"
	$(MAKE) $(JPAR) build-done

docs-done: install-docs
ifeq ($(ENABLE_LYX),ON)
docs-done: build/docs/femera-guide.pdf build/docs/femera-quick-start.pdf
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
install-docs: LICENSE README.md | $(INSTALL_CPU)/share/doc/femera/
	-cp docs/*.xhtml "$(INSTALL_CPU)/share/doc/femera/"
	-cp LICENSE "$(INSTALL_CPU)/share/doc/femera/"
	# -cp NOSA-1-3.txt "$(INSTALL_CPU)/share/doc/femera/"
	-cp LICENSE "$(INSTALL_CPU)/share/doc/femera/"
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

# $(BUILD_CPU)/make-build.post.test.out
build-done: build/src-notest.eps $(BUILD_CPU)/mini.valgrind.log code-stats
	$(call timestamp,$@,)
	$(info $(DONE) building $(FEMERA_VERSION) with $(CXX) $(CXX_VERSION))
	$(info $(SPCS) on $(HOSTNAME) for $(CPUMODEL))
ifneq ($(HOST_MD5),$(REPO_MD5))
	$(info $(SPCS) as modified by <$(BUILT_BY_EMAIL)>)
endif
	$(info $(E_G_) fmrexec auto examples/cube.fmr)

code-stats: $(SRC_STAT_FILE)
	tools/code-stats

perf-done:
	$(call timestamp,$@,)
	$(info $(DONE) performance testing $(FEMERA_VERSION) built with)
	$(info $(SPCS) $(CXX) $(CXX_VERSION) on $(HOSTNAME) for $(CPUMODEL))

# Femera tools ----------------------------------------------------------------
install-tools:
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
ifeq ($(ENABLE_GOOGLETEST),ON-FIXME-DISABLED)

.PHONY: $(GTEST_FLAGFILE).new

external-flags: $(GTEST_FLAGFILE).new

$(GTEST_FLAGFILE).new:
	printf "%s" '$(GTEST_FLAGS)' > $(@)
endif
ifeq ($(ENABLE_PYBIND11),ON)

.PHONY: $(PYBIND11_FLAGFILE).new

$(BUILD_DIR)/external/pybind11-install.out : | $(PYBIND11_DEPS)

external-flags: $(PYBIND11_FLAGFILE).new

$(PYBIND11_FLAGFILE).new:
	printf "%s" '$(PYBIND11_FLAGS)' > $(@)
endif
#ifeq ($(ENABLE_BOOST_HEADERS),ON)
#
#.PHONY: $(BOOST_FLAGFILE).new
#
#external-flags: $(BOOST_FLAGFILE).new
#
#$(PYBIND11_BOOST).new:
#	printf "%s" '$(BOOST_FLAGS)' > $(@)
#endif

#TODO REMOVE install-bats: external/get-bats.test.sh
install-bats: external/bats-install.sh external/bats-install.test.bats
	#TODO REMOVE $(call label_test,$(PASS),$(FAIL),external/get-bats.test.sh, \
	#  $(BUILD_DIR)/external/get-bats.test)
	$(call label_test,$(PASS),$(FAIL), \
	  external/bats-install.sh "$(INSTALL_DIR)", \
	  $(BUILD_DIR)/external/bats-install)
	$(call label_bats,$(PASS),$(FAIL),external/bats-install.test.bats, \
	  $(BUILD_DIR)/external/bats-install.test)
	$(call timestamp,$@,$<)

#TODO REMOVE get-bats:
#	$(call timestamp,$@,$<)
#	$(MAKE) $(JPAR) $(GET_BATS)
#
#TODO REMOVE get-external: | intro external/tools/
#	$(call timestamp,$@,)
#	$(MAKE) $(JPAR) $(GET_EXTERNAL)
#
#get-%: | intro $(BUILD_DIR)/external/
#	#(call timestamp,$@,$<)
#	external/get-external $(*)

install-external: $(INSTALL_EXTERNAL) | intro
	$(call timestamp,$@,make $(JEXT))

install-%: $(BUILD_CPU)/external/%-install.out | intro $(BUILD_CPU)/external/
	$(call timestamp,$@,)

install-%: $(BUILD_DIR)/external/%-install.out | intro $(BUILD_DIR)/external/
	$(call timestamp,$@,)

external-done:
	$(info $(DONE) building and installing externals on $(HOSTNAME) to:)
	$(info $(SPCS) $(INSTALL_DIR)/)
	$(info $(E_G_) $(LIST_EXTERNAL))
	$(call timestamp,$@,)

#TODO REMOVE $(BUILD_DIR)/external/get-bats-%.out: external/get-external
#	#(info $(INFO) checking bats-$(*)...)
#	-tools/label-test "$(PASS)" "$(FAIL)" \
#	  "external/get-external bats-$(*)" \
#	  "$(BUILD_DIR)/external/get-bats-$(*)"
#
#$(BUILD_DIR)/external/get-%.out: external/get-external external/get-%.dat
#	#(info $(INFO) checking $(*)...)
#	-tools/label-test "$(PASS)" "$(FAIL)" \
#	  "external/get-external $(*)" \
#	  "$(BUILD_DIR)/external/get-$(*)"

$(BUILD_CPU)/external/%-config.flags: $(BUILD_CPU)/external/%-config.flags.new
	tools/update-file-if-diff "$(@)"
	#rm -f $(<)

$(BUILD_CPU)/external/%-config.out: external/%-config.sh
$(BUILD_CPU)/external/%-config.out: $(BUILD_CPU)/external/%-config.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_CPU)/external/$(*)
	-tools/label-watch "$(PASS)" "$(FAIL)" \
	  "external/$(*)-install.sh $(INSTALL_CPU) $(<) $(JEXT)" \
	  "$(BUILD_CPU)/external/$(*)-install"

$(BUILD_CPU)/external/%-install.flags: $(BUILD_CPU)/external/%-install.flags.new
	tools/update-file-if-diff "$(@)"
	#rm -f $(<)

$(BUILD_DIR)/external/%-install.flags: $(BUILD_DIR)/external/%-install.flags.new
	tools/update-file-if-diff "$(@)"
	#rm -f $(<)

$(BUILD_CPU)/external/%-install.out: external/%-install.sh
$(BUILD_CPU)/external/%-install.out: $(BUILD_CPU)/external/%-install.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_CPU)/external/$(*)
	-tools/label-watch "$(PASS)" "$(FAIL)" \
	  "external/$(*)-install.sh $(INSTALL_CPU) $(<) $(JEXT)" \
	  "$(BUILD_CPU)/external/$(*)-install"

$(BUILD_DIR)/external/%-install.out: external/%-install.sh
$(BUILD_DIR)/external/%-install.out: $(BUILD_DIR)/external/%-install.flags
	$(call timestamp,$@,)
	mkdir -p $(BUILD_DIR)/external/$(*)
	-tools/label-test "$(PASS)" "$(FAIL)" \
	  "external/$(*)-install.sh $(INSTALL_DIR) $(<) $(JEXT)" \
	  "$(BUILD_DIR)/external/$(*)-install"

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
	$(CC) $(<) $(FMRFLAGS) $(INCFLAGS) $(LDLIBS) -o $(BUILD_CPU)/tools/fmrnumas
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

$(BUILD_CPU)/%.perf.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.perf.o : src/%.perf.cpp src/%.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXPERFS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

$(BUILD_CPU)/%.perf.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.perf.o : src/%.perf.cpp src/%.cu.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXPERFS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

# Header-only
$(BUILD_CPU)/%.perf.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.perf.o : src/%.perf.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXPERFS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXTESTS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.cu.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXTESTS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

# Header-only
$(BUILD_CPU)/%.gtst.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.gtst.o : src/%.gtst.cpp src/%.hpp $(TOPDEPS)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXTESTS) $(FMRFLAGS) $(INCFLAGS) $< -o $@
else
	touch $@
endif

$(BUILD_CPU)/%.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.o : src/%.cpp src/%.hpp $(TOPDEPS)
	#-python tools/testy/check_code_graffiti.py $^
	# rm -f $*.err $*.gtst.err
	#(info $(CXX_) $(CXX) -c $< .. -o $(notdir $@))
	$(call col2cxx,$(CXX_),$(CXX) -c $<,$(notdir $@))
	$(CXX) -c $(CXXFLAGS) $(FMRFLAGS) $(INCFLAGS) $< -o $@

ifeq ($(ENABLE_NVIDIA),ON)
$(BUILD_CPU)/%.o : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/%.o : src/%.cu.cpp src/%.hpp $(TOPDEPS)
	#-python tools/testy/check_code_graffiti.py $^
	# rm -f $*.err $*.gtst.err
	#(info $(CXX_) $(CXX) -c $< .. -o $(notdir $@))
	$(call col2cxx,$(CXX_),$(CUXX) -c $<,$(notdir $@))
	$(CUXX) -c $(CUFLAGS) $(FMRFLAGS_FIXME) $(INCFLAGS) $< -o $@
endif
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
$(BUILD_CPU)/mini: export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/mini: export PATH:=$(shell pwd)/$(BUILD_CPU):$(PATH)
$(BUILD_CPU)/mini: src/femera/mini.cpp src/femera/femera.hpp $(LIBFEMERA)
	$(call timestamp,$@,$<)
ifeq ($(ENABLE_GCC_PROFILE),ON)
	-$(CXX) $(CXXMINI) $< $(LDFLAGS) -lfemera $(LDLIBS) -o $@.pro
	fmrexec tdd:$(@).pro # >/dev/null
	-$(CXX) $(filter-out -fprofile-generate,$(CXXMINI)) -fprofile-use \
  $< $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	-$(CXX) $(CXXMINI) $< $(LDFLAGS) -lfemera $(LDLIBS) -o $@
	#TODO strip when NOT compiling debug version
	# strip $@
endif
	$(call col2cxx,$(CXX_),$(CXX) $(notdir $<) .. $(notdir $@),$(shell \
	if [ -f "$(@)" ]; then ls -sh $(@) | cut -d " " -f1; fi)B)
	$(call label_test,$(PASS),$(FAIL),fmrexec tdd:$(@),$(@))

$(BUILD_CPU)/mini.valgrind.log: export PATH:=$(shell pwd)/$(BUILD_CPU):$(PATH)
$(BUILD_CPU)/mini.valgrind.log: $(VALGRIND_SUPP_EXE)
ifeq ($(ENABLE_VALGRIND),ON)
	$(info $(GRND) valgrind .. fmrexec tdd:$(<))
	$(VGEXEC) # >> $(BUILD_CPU)/mini.valgrind.out
	-grep -i 'lost: [1-9]' $(BUILD_CPU)/mini.valgrind.log \
  | cut -d " " -f 5- | awk '{print "$(WARN) valgrind:",$$0}'
	-grep -i '[1-9] err' $(BUILD_CPU)/mini.valgrind.log \
  | cut -d " " -f 4- | awk '{print "$(WARN) valgrind:",$$0}'
	$(info $(MORE) $(BUILD_CPU)/mini.valgrind.log)
else
	echo "Set ENABLE_VALGRIND ON in config.local to check for memory leaks." \
	  >$@
endif

build/%.perf.out : build/%.perf
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call label_test,$(PASS),$(FAIL),fmrexec tdd:$(^),$(^))
else
	echo "build/$*.o not tested: GoogleTest disabled" > $@
	echo "WARNING: build/$*.o not tested: GoogleTest disabled" > $^.err
endif

build/%.perf : export TMPDIR := $(TEMP_DIR)
build/%.perf : build/%.perf.o $(LIBFEMERA)(build/%.o)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(@).o,$(notdir $@))
	-$(CXX) $(CXXPERFS) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

# Header-only
build/%.perf : export TMPDIR := $(TEMP_DIR)
build/%.perf : build/%.perf.o
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(@).o,$(notdir $@))
	-$(CXX) $(CXXPERFS) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

build/%.gtst.out : build/%.gtst
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call label_test,$(PASS),$(FAIL),fmrexec tdd:$(^),$(^))
else
	echo "build/$*.o not tested: GoogleTest disabled" > $@
	echo "WARNING: build/$*.o not tested: GoogleTest disabled" > $^.err
endif

build/%.gtst : export TMPDIR := $(TEMP_DIR)
build/%.gtst : build/%.gtst.o $(LIBFEMERA)(build/%.o) \
  $(LIBFEMERA)($(BUILD_CPU)/femera/Test.o) \
  $(LIBFEMERA)($(BUILD_CPU)/femera/task/Jobs.o)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(@).o,$(notdir $@))
	-$(CXX) $(CXXTESTS) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

# Header-only
build/%.gtst : export TMPDIR := $(TEMP_DIR)
build/%.gtst : build/%.gtst.o $(LIBFEMERA)($(BUILD_CPU)/femera/Test.o)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(call col2cxx,$(LINK),$(CXX) $(@).o,$(notdir $@))
	-$(CXX) $(CXXTESTS) $@.o $(FMRFLAGS) $(LDFLAGS) -lfemera $(LDLIBS) -o $@
else
	$(info $(WARN) $@ not tested: GoogleTest disabled)
	echo "WARNING: $@ not tested: GoogleTest disabled" >> $@.err
	touch $@
endif

ifeq ($(ENABLE_MPI),ON)
$(BUILD_CPU)/sizeof-mpi-comm.exe : export TMPDIR := $(TEMP_DIR)
$(BUILD_CPU)/sizeof-mpi-comm.exe : src/sizeof-mpi-comm.c
	$(call col2cxx,$(CXX_),$(CXX) $<,$(notdir $@))
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(INCFLAGS) -lmpi -o $@
endif

$(VALGRIND_SUPP_EXE) : export TMPDIR := $(TEMP_DIR)
$(VALGRIND_SUPP_EXE) : src/$(VALGRIND_SUPP).cpp $(BUILD_CPU)/$(VALGRIND_SUPP)
ifeq ($(ENABLE_VALGRIND),ON)
	$(call col2cxx,$(CXX_),$(CXX) $<,$(notdir $@))
	$(CXX) $(VGFLAGS) $< $(LDFLAGS) -o $@ $(LDLIBS)
	$(info $(GRND) suppression file: $(BUILD_CPU)/$(VALGRIND_SUPP))
	$(VGSUPP)
else
	touch $@
endif

$(BUILD_CPU)/$(VALGRIND_SUPP): $(VALGRIND_SUPP_FILE)
ifeq ("$(VALGRIND_SUPP_FILE)","")
	touch $(*)
else
	cat $(VALGRIND_SUPP_FILE) >> $(*)
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
	tools/update-file-if-diff "$(BUILD_CPU)/femera.flags"
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

$(SRC_STAT_FILE): | build/$(CPUMODEL)/
	touch "$(SRC_STAT_FILE)"; \
	if ! grep "$(FEMERA_VERSION)" "$(SRC_STAT_FILE)" | grep -q "$(HOSTNAME)"; \
	then echo "$(BUILD_DATE)",'"'$(FEMERA_VERSION)'"',\
	"`cat build/src-code-stats.csv`",\
	"`tools/elapsed-time $(BUILD_SECS)`",'"'$(CXX) $(CXX_VERSION)'"',\
	'"'$(HOSTNAME)'"','"'$(CPUMODEL)'"'\
	>> "$(SRC_STAT_FILE)"; fi
	-tools/plot_code_stats.py 2>/dev/null
	$(call timestamp,$@,)

build/test-files.txt: tools/list-test-filesbuild/.md5
	-tools/list-test-files> $@

src/docs/src.dot: build/.md5
	@external/tools/cinclude2dot --src src >$@ 2>build/src.dot.err
	@dot $@ -Gsize="18.0,12.0" -Teps -o build/src-test.eps
ifeq ($(ENABLE_DOT_PNG),ON)
	@dot $@ -Gsize="18.0,12.0" -Tpng -o build/src-test.png
endif
	#cinclude2dot --groups is nice, too
	#(info $(INFO) Source dependencies: $@)

src/docs/src-notest.dot: src/docs/src.dot
	@grep -v '\.gtst\.' $< > $@

src/docs/src-headers.dot: src/docs/src.dot
	@grep -v '\.cpp' $< | grep -v '\.ipp' > $@
	#(info $(INFO) Dependencies without tests: $@)

build/src-notest.eps: src/docs/src-headers.dot
build/src-notest.eps: src/docs/src-notest.dot tools/src-inherit
ifeq ($(ENABLE_DOT),ON)
	@tools/src-inherit
	@dot $< -Gsize="12.0,8.0" -Teps -o $@
	@dot src/docs/src-headers.dot -Gsize="18.0,9.0" -Teps -o build/src-headers.eps
ifeq ($(ENABLE_DOT_PNG),ON)
	@dot $< -Gsize="12.0,8.0" -Tpng -o build/src-notest.png
	@dot src/docs/src-headers.dot -Gsize="18.0,9.0" -Tpng -o build/src-headers.png
endif
	#  -Gratio="fill" -Gsize="11.7,8.267!" -Gmargin=0
	$(info $(INFO) source code include graph: $@)
endif

build/docs/tdd-tests.txt: tools/list-tdd-tests build/docs/.md5
	-tools/list-tdd-tests src/docs/*.tex src/docs/*.lyx > $@

build/docs/find-tdd-files.csv: build/docs/tdd-tests.txt build/test-files.txt
build/docs/find-tdd-files.csv: tools/compare-lists.py
	-tools/compare-lists.py build/docs/tdd-tests.txt build/test-files.txt \
	  >$@ 2>build/docs/find-tdd-files.err

ifeq ($(ENABLE_LYX),ON)
build/docs/%.pdf: src/docs/%.lyx src/docs/quick-start.tex
	-tools/label-test "$(EXEC)" "$(FAIL)" \
	  "lyx -batch -n -f all -E pdf $(@) $(<)" \
	  "build/docs/$(*)-pdf"
else
build/docs/%.pdf:
	echo "Documentation is not yet available." > "build/docs/$(*)-pdf.out"
endif

ifeq ($(ENABLE_LYX),ON)
docs/%.xhtml: src/docs/%.lyx src/docs/quick-start.tex
	-tools/label-test "$(PASS)" "$(FAIL)" \
	  "lyx -batch -n -f all -E xhtml $(@) $(<)" \
	  "build/docs/$(*)-xhtml"
else
docs/%.xhtml:
	echo "Documentation is not yet available." > "build/docs/$(*)-xhtml.out"
endif
