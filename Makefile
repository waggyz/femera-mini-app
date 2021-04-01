SHELL:=/bin/bash

-include config.local
-include set-undefined.mk
-include labels.mk

SPC40:=$(shell echo "                                        ")
COMPILED_BY_EMAIL:=$(shell echo '$(COMPILED_BY)' \
  | sed 's/ *(.*)//; s/>.*//; s/.*[:<] *//' )

#TODO Check for BLAS and LAPAK
#TODO Check for MPI

CPUCOUNT:=$(shell tools/cpucount.sh)
CPUMODEL:=$(shell tools/cpumodel.sh)
CPUSIMD :=$(shell tools/cpusimd.sh)

TEST_DIR := build/$(CPUMODEL)

ifeq ($(INSTALL_GENERIC_NAME),ON)
  BUILD_DIR := build/femera
  STAGE_DIR := build/stage
  BUILD_EXTERNAL_DIR := build
else
  BUILD_DIR := build/$(CPUMODEL)-$(CXX)/femera
  STAGE_DIR := build/$(CPUMODEL)-$(CXX)/stage
  BUILD_EXTERNAL_DIR := build/$(CPUMODEL)-$(CXX)
  INSTALL_DIR := $(INSTALL_DIR)/$(CPUMODEL)-$(CXX)
  INSTALL_SUFFIX :=-$(CPUMODEL)-$(CXX)
endif
BUILD_TREE:= $(BUILD_DIR)/ $(STAGE_DIR)/ $(TEST_DIR)/
ifndef TDD_FMRFILE
  # TDD_FMRFILE:=tests/mesh/cube-tet6p1n1.cgns
  TDD_FMRFILE:=tests/mesh/cube-tet?p*.* -o'$(BUILD_DIR)/cube-test.cgn'
endif

ifeq ($(shell which dot 2>/dev/null),"")# dot is part of graphviz
  ENABLE_GRAPHIZ:=OFF
else
  ENABLE_GRAPHIZ:=ON
  INSTALL_CINCLUDE2DOT:=external/tools/cinclude2dot
endif

# TODO Change *-ok to *-flags as external requirement.

# ENABLE_GOOGLETEST := OFF

ifeq ($(ENABLE_GOOGLETEST),ON)
  CORE_LEAF += src/Proc/Gtst
  BUILD_TREE += $(BUILD_EXTERNAL_DIR)/gmsh/
  INSTALL_EXTERNAL += $(BUILD_EXTERNAL_DIR)/googletest-ok
endif
ifeq ($(ENABLE_GMSH_OPENMP),ON)
  GMSH_FLAGS += -DENABLE_OPENMP=1
endif
ifeq ($(ENABLE_OMP),ON)
  CORE_LEAF += src/Proc/Pomp
  # export OMP_PLACES=cores
  # export OMP_PROC_BIND=spread
  # export OMP_NUM_THREADS=2
  #NOTE Can not export from here; use the src/ompexec shell script
endif
# For now, disable MPI in Gmsh. It is probably not needed.
GMSH_FLAGS += -DENABLE_MPI=0
ifeq ($(ENABLE_MPI),ON)
  CORE_LEAF += src/Proc/Pmpi
  #TODO Is MPI breaking Gmsh on K?-no. Is it needed for CGNS within Gmsh?-no
  # GMSH_FLAGS += -DENABLE_MPI=1
  PETSC_FLAGS += --with-mpi
  # for valgrind with OpenMPI
  VGMPISUPP:=$(shell tools/valgrind-mpi-supp.sh)
endif
ifeq ($(ENABLE_PYBIND),ON)
  BUILD_TREE += $(BUILD_EXTERNAL_DIR)/pybind11/
  INSTALL_EXTERNAL += $(BUILD_EXTERNAL_DIR)/pybind11-ok
  GMSH_FLAGS += -DENABLE_WRAP_PYTHON=1 -DENABLE_NUMPY=1
  GMSH_REQUIRES += $(BUILD_EXTERNAL_DIR)/pybind11-ok
endif
ifeq ($(ENABLE_CGNS),ON)
  MINI_LEAF += src/Data/Dcgn
  BUILD_TREE += $(BUILD_EXTERNAL_DIR)/CGNS/
  INSTALL_EXTERNAL += $(BUILD_EXTERNAL_DIR)/CGNS-ok
  CGNS_REQUIRES += $(BUILD_EXTERNAL_DIR)/hdf5-ok
  GMSH_FLAGS += -DENABLE_CGNS=1 -DENABLE_CGNS_CPEX0045=1
  GMSH_REQUIRES += $(BUILD_EXTERNAL_DIR)/CGNS-ok
  PETSC_REQUIRES += $(BUILD_EXTERNAL_DIR)/CGNS-ok
  PETSC_FLAGS += --with-cgns --with-hdf5 --with-zlib
endif
ifeq ($(ENABLE_PETSC),ON)#TODO Check for PETSc.
  BUILD_TREE += $(BUILD_EXTERNAL_DIR)/petsc/
  ifeq ("$(wildcard $(BUILD_EXTERNAL_DIR)/petsc-ok)","")
    $(shell touch $(BUILD_EXTERNAL_DIR)/petsc-ok)
  endif
  GMSH_REQUIRES += $(BUILD_EXTERNAL_DIR)/petsc-ok
  GMSH_FLAGS += -DENABLE_PETSC=1
  ifeq ($(ENABLE_PYBIND),ON)
    GMSH_FLAGS += -DENABLE_PETSC4PY=1
  endif
  PETSC_REQUIRES:=$(GMSH_REQUIRES) $(BUILD_EXTERNAL_DIR)/petsc-flags
  INSTALL_EXTERNAL += $(BUILD_EXTERNAL_DIR)/petsc-ok
endif
ifeq ($(ENABLE_GMSH),ON)
  MINI_LEAF += src/Data/Dmsh
  INSTALL_EXTERNAL += $(BUILD_EXTERNAL_DIR)/gmsh-ok
  GMSH_REQUIRES += $(BUILD_EXTERNAL_DIR)/gmsh-flags
endif
ifeq ($(ENABLE_FLTK),ON)
  GMSH_FLAGS += -DENABLE_FLTK=1
else
  GMSH_FLAGS += -DENABLE_FLTK=0
endif

BUILD_HOST := $(shell hostname)
CXXVERSION := $(shell $(CXX) -dumpversion )

BUILD_VERSION  := $(shell tools/build-version-number.sh)
BUILT_BY_INFO_1:=Femera $(BUILD_VERSION)
BUILT_BY_MINI_1:=Femera mini-app $(BUILD_VERSION)
BUILT_ON_INFO  :=$(BUILD_HOST) for $(CPUMODEL) with $(CXX) $(CXXVERSION)
BUILT_BY_INFO_2:=built on $(BUILT_ON_INFO)
# Check if the user has altered content in src/ data/, tools/,
# or some content in extras/, even if not in a git clone.
REPO_MD5:=$(shell cat .md5)
LOCAL_MD5:=$(shell tools/md5-all.sh -)
ifneq ($(LOCAL_MD5),$(REPO_MD5))
  BUILT_BY_INFO_3:=built by <$(COMPILED_BY_EMAIL)>.
endif

#******************
# It's never correct to have .o files depend on other .o files.
# It's never correct to have executable files depend on header files.
# https://stackoverflow.com/questions/37632836/makefile-not-allowing-me-to
#   -specify-directory-for-object-files
#******************

# src/work src/proc src/data src/phys src/main
# API_CXX   := src/proc
CORE_TRUNK  := src/Work src/Proc src/Flog
MINI_BRANCH := src/Data src/Sims src/Phys src/Main
MINI_LEAF   += src/Main/Plug
MINI_LEAF   += src/Sims/Part src/Sims/Frun
MINI_LEAF   += src/Sims/Geom src/Sims/Mesh
#  src/Sims/Cpu1 src/Sims/Fset src/Sims/Fifo
MINI_LEAF   += src/Data/Fake
# CORE_LEAF   +=

MINI_BASE   := $(CORE_TRUNK) $(MINI_BRANCH)
ifeq (1,1) # ENABLED -----------------------------------------------------------

BUILD_TREE += $(patsubst src/%,$(BUILD_DIR)/%/,$(MINI_BASE))
BUILD_TREE += $(BUILD_DIR)/build-data/

CORE_OBJS := $(patsubst src/%,$(BUILD_DIR)/%.o, $(CORE_TRUNK) $(CORE_LEAF) )
CORE_GTST := $(patsubst src/%,$(BUILD_DIR)/%.gtst, $(CORE_TRUNK) $(CORE_LEAF) )
MINI_OBJS := $(patsubst src/%,$(BUILD_DIR)/%.o, $(MINI_BRANCH) $(MINI_LEAF) )
MINI_GTST := $(patsubst src/%,$(BUILD_DIR)/%.gtst, $(MINI_BRANCH) $(MINI_LEAF) )
else # DISABLED ----------------------------------------------------------------
ifeq (1,0)
ifeq (1,1)
  CLASS_HPP := $(addsuffix .hpp,$(MINI_DIRS))
else
  CLASS_HPP := $(shell find src/ -maxdepth 3 -name "[[:upper:]]*.hpp" -printf "%p ")
endif
endif

FULL_DIRS := $(shell find src/* -maxdepth 3 -type d -printf "%p ")
# FULL_DIRS := $(sort $(dir $(wildcard src/*)) $(dir $(wildcard src/*/*)))
BUILD_TREE += $(patsubst src/%,$(BUILD_DIR)/%/,$(FULL_DIRS)) \
  $(BUILD_DIR)/build-data/

CXXSRCS := $(CXXSRCS) $(shell find src/ -maxdepth 3 -name "*.cpp" -printf "%p ")

FULL_OBJS += $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(CXXSRCS))

# Exclude tests and executables from lib.
NOT_IN_LIB := %.noop.o %.stub.o %.mock.o %.dumm.o %.fake.o %.test.o %.gtst.o \
  %/mini.o %/full.o
FULL_OBJS := $(filter-out $(NOT_IN_LIB) $(CORE_OBJS),$(FULL_OBJS))

# FULL_DIRS := $(shell find src/* -maxdepth 3 -type d -printf "%p ")
NOT_MINI := $(filter-out $(MINI_DIRS),$(FULL_DIRS))

MINI_PATTERN := $(patsubst src/%,$(BUILD_DIR)/%%,$(NOT_MINI))
MINI_OBJS := $(filter-out $(MINI_PATTERN),$(FULL_OBJS) )
# MINI_OBJS := $(filter-out $(CORE_OBJS),$(MINI_OBJS) )
# MINI_LIBS := $(addsuffix .a,$(MINI_DIRS))
endif # END DISABLED -----------------------------------------------------------

WRAP_SRCS := \
  $(BUILD_DIR)/build-data/mini-help.en.inc \
  $(BUILD_DIR)/build-data/build-info.inc   \
  $(BUILD_DIR)/build-data/build-detail.inc \
  $(BUILD_DIR)/build-data/built-by.inc     \
  $(BUILD_DIR)/build-data/copyright.inc
#  $(BUILD_DIR)/femera-options.inc
#  $(BUILD_DIR)/build-data/LICENSE.inc
#  $(BUILD_DIR)/build-data/modification.inc

ifeq ($(ENABLE_GOOGLETEST),ON)
  WRAP_SRCS += $(BUILD_DIR)/build-data/googletest-version.inc
endif

# MINI_SRCS := src/mini.cpp # $(WRAP_SRCS)

AREXE := ar
#NOTE There is no switch case in makefile syntax.
ifeq ($(CXX),g++)
  ifeq ($(ENABLE_MPI),ON)
    CXX := mpic++
    # LDLIBS += -lstdc++ # maybe needed?
    #TODO Fix the following hack by figuring out the correct lib order.
    # LDLIBS += -Wl,--copy-dt-needed-entries
  endif
  CXXFLAGS := -std=c++11 -g -Ofast -march=native -mtune=native
  ifeq ($(ENABLE_OMP),ON)
    CXXFLAGS += -fopenmp
    FMRFLAGS += -D_GLIBCXX_PARALLEL
  endif
  CXXFLAGS += -fno-builtin-sin -fno-builtin-cos \
    -fpic -flto -fvisibility=hidden -fvisibility-inlines-hidden \
    -fearly-inlining --param inline-min-speedup=2 \
    --param inline-unit-growth=500 --param large-function-growth=500 \
    -funroll-loops -ftree-vectorize -fstrict-enums
  CXXFLAGS += -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow \
    -Wdouble-promotion -Wfloat-equal -Wconversion -Wcast-qual -Wcast-align \
    -Wlogical-op -Woverloaded-virtual -Wstrict-null-sentinel -Wnoexcept \
    -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization \
    -Wunused-macros -Wzero-as-null-pointer-constant -Wundef -Weffc++
  CXXFLAGS += -MMD -MP
    # -Winline #TODO need to filter warings from system libs.
    # -Wpadded
    #NOTE flags in the -fpic line may be better if only used to build library
    #     perhaps -fpie (or -fPIE) should be used for builing executable
    #     -fpic (platform-dependent) should be smaller and faster than
    #     -fPIC (which "always works")
    # -finline-limit=1000000 --param large-unit-insns=1000
    # -fvisibility=hidden may need to be removed,
    #--param inline-xx=1000 --param large-xxx=1000
    # -fmax-errors=10
    #NOTE --param: numeric args are percent.
    #NOTE Do NOT use these if gtest.h is included: -Wundef -Winline
    #TODO Use only if gcc version does NOT warn about stl library inlining,
    #     and suppresses library warnings with -isystem instead of -I
    #     -Weffc++ -Winline -Wzero-as-null-pointer-constant
    #     or add later and filter out std::string and other unimportant
    #     -Wpadded doesn't seem to get filtered out by -isystem...
    #TODO Check supported gcc version: -Wsuggest-attribute -Walloc-zero
    #     -Wduplicated-branches -Wduplicated-cond  -Wunsafe-loop-optimizations
    #     -Wmultiple-inheritance -Wvirtual-inheritance -Wsized-deallocation
    #     -fno-semantic-interposition -fstrong-eval-order -faligned-new
    #     -Wsuggest-override -Wsuggest-final-methods -Wsuggest-final-types
    #TODO maybe add --no-as-needed for gcc>X to add all symbols to .a
    # Hmm...these are gcc, but supported by many compilers
    #     -Wsuggest-attribute=cold -Wsuggest-attribute=malloc
    #     -Wsuggest-attribute=pure -Wsuggest-attribute=const
    #     -Wsuggest-attribute=format
    #TODO See if these are useful: -Wpadded -fprefetch-loop-arrays
    # not useful: -Waggregate-return

    # This creates dependency (.d) files for make.
  AREXE := gcc-ar
endif
ifeq ($(CXX),icpc)
  CXXFLAGS := -restrict c++11 -Wall -Wextra -Wshadow -g \
    -Ofast -xHost -auto-ilp32 -ipo -fPIC -ffast-math -no-fast-transcendentals \
    -no-inline-max-size -no-inline-max-total-size -qoverride-limits
  ifeq ($(ENABLE_INTEL_MKL),ON)
    FMRFLAGS += -mkl=sequential -DMKL_DIRECT_CALL_SEQ
  endif
  AREXE := xiar
endif

# Femera-specific compiler options
FMRFLAGS += \
  -DFMR_VERSION=\"$(BUILD_VERSION)\" \
  -DFMR_CORE_N=$(CPUCOUNT)           \
  -DFMR_BUILD_DIR=\"$(BUILD_DIR)\"
ifdef FMR_VERBMAX
  FMRFLAGS += -DFMR_VERBMAX=$(FMR_VERBMAX)
endif
ifdef FMR_TIMELVL
  FMRFLAGS +=-DFMR_TIMELVL=$(FMR_TIMELVL)
endif

ifeq ($(ENABLE_LIBNUMA),ON)
  FMRFLAGS += -DFMR_HAS_LIBNUMA
  LDLIBS += -lnuma
endif

ifeq ($(CXX),mpic++)# or g++
  # need to remove libfemera from this
  FMRFLAGS += -I"$(BUILD_DIR)" -isystem"$(INSTALL_DIR)/include"
else
  FMRFLAGS += -I$(BUILD_DIR) -I"$(INSTALL_DIR)/include"
endif

LDPATH:=$(INSTALL_DIR)/lib:$(INSTALL_DIR)/lib64:${LD_LIBRARY_PATH}
LDFLAGS += -L"$(BUILD_DIR)" -L"$(INSTALL_DIR)/lib" -L"$(INSTALL_DIR)/lib64"

PYEXEC := LD_LIBRARY_PATH=$(LDPATH) python

ifeq ($(ENABLE_MPI),ON)
  #TODO Find include and lib dirs automatically
  ifeq ($(CXX),mpic++)
   FMRFLAGS += -isystem"/usr/include/openmpi-x86_64"
  else
   FMRFLAGS += -I"/usr/include/openmpi-x86_64"
  endif
  LDFLAGS += -L/usr/lib64/openmpi/lib
  LDLIBS += -lmpi
#  MPIEXEC := fmrexec -np $(TDD_MPI_NP) \
#    --bind-to core -map-by node:pe=$(TDD_OMP_NP)
  MPIEXEC := fmr1node
  TEST_EXEC := LD_LIBRARY_PATH=$(LDPATH) $(MPIEXEC)
  VGMPI := mpiexec -np $(TDD_MPI_NP) \
    --bind-to core -map-by node:pe=$(TDD_OMP_NP)
else
  TEST_EXEC := LD_LIBRARY_PATH=$(LDPATH)
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
  LDLIBS += -lgtest -lgmock -lpthread
  #  -lstdc++
  #  -D_GLIBCXX_USE_CXX11_ABI=1
  #   or 0, may be needed when compiling external -lstdc++
  GTESTFLAGS := $(filter-out -Wundef,$(CXXFLAGS))
  # GTESTFLAGS := $(filter-out -Winline,$(GTESTFLAGS))
  GTESTFLAGS += $(FMRFLAGS)
  # -DFMR_HAS_GTEST
endif
ifeq ($(ENABLE_HDF5),ON)
  # LDLIBS += -l:libhdf5.a
  LDLIBS += -lhdf5
endif
ifeq ($(ENABLE_CGNS),ON)
  # LDLIBS += -l:libcgns.a
  LDLIBS += -lcgns
endif
ifeq ($(ENABLE_GMSH),ON)
  #NOTE linking gmsh static increases executable by 170 MB!
  # LDLIBS += -l:libgmsh.a -llapack -lblas
  LDLIBS += -lgmsh
endif
ifeq ($(ENABLE_PETSC),ON)
  LDLIBS += -lpetsc
endif

CXXFLAGS+= $(FMRFLAGS)

ifeq ($(ENABLE_VALGRIND),ON)
  # Use (mostly) the same flags to compile the suppression file.
  VGFLAGS:= $(filter-out -Winline,$(CXXFLAGS))
  VALGRIND_SUPP := valgrind.supp
  VALGRIND_SUPP_EXE := $(BUILD_DIR)/$(VALGRIND_SUPP).exe
  VGSUPP := valgrind --leak-check=full --show-reachable=yes --error-limit=no \
    --show-leak-kinds=all --gen-suppressions=all           \
    $(VGMPI) $(BUILD_DIR)/$(VALGRIND_SUPP).exe 3>&1 1>&2 2>&3 \
    | tools/grindmerge.pl $(VGMPISUPP)                     \
    > $(BUILD_DIR)/$(VALGRIND_SUPP) 2>/dev/null;

  VGEXEC := valgrind --track-origins=yes --leak-check=full \
    --suppressions=$(BUILD_DIR)/$(VALGRIND_SUPP)           \
    --log-file=$(BUILD_DIR)/mini.valgrind.log              \
    $(VGMPI) $(BUILD_DIR)/mini -n$(TDD_OMP_NP) -v0 $(TDD_FMRFILE); \
    sed -i '/invalid file descriptor 10[0-4][0-9] /d'      \
    $(BUILD_DIR)/mini.valgrind.log;                        \
    sed -i '/invalid file descriptor 2[56][0-9] /d'        \
    $(BUILD_DIR)/mini.valgrind.log;                        \
    sed -i '/select an alternative log fd/d'               \
    $(BUILD_DIR)/mini.valgrind.log
endif


# Define some functions to call -----------------------------------------------

# This keeps the time persistent through recursive makefile calls.
BUILD_DATE := ${BUILD_DATE}
BUILD_SECS := ${BUILD_SECS}
ifndef BUILD_DATE
  export BUILD_DATE := $(shell date -u +"%Y-%m-%dT%H:%M")
  export BUILD_SECS := $(shell date +%s)
endif

strcut = $(shell echo "$(1)" | cut -c-45 )
build_timestamp = @printf '%8s%-46s %25s\n' "  make  " \
'$(call strcut,$(1): $(2)$(SPC40)$(SPC40))' \
$(BUILD_DATE)"+"$(shell tools/elapsed_time.sh $(BUILD_SECS))

rewrap_file = printf "$(INFO_COLOR)"; cat '$(1)' | tr '\n' ' ' | tr -s " " \
  | fold -w 72 -s | sed 's/^/$(SPCS) /'; printf "$(NORM_COLOR)\n"

ifeq ($(CXX),mpic++)#TODO or g++
  # Remove warnings from system headers not recognized as such.
  ifeq (1,1)
  build_log = 2> >( \
   sed '/\/gtest-internal.* In /,+3d' \
 | sed '/\/gtest-internal/,+2d' \
 | sed '/\/external\/gmsh.*arning/,+3d' \
 | tee $(1).err >&2);
  else
  build_log = 2> >( \
   sed '/\/bits\/.* In /,+3d' \
 | sed '/\/bits\//,+2d' \
 | sed '/\/sstream.* In /,+3d' \
 | sed '/\/sstream/,+2d' \
 | sed '/In member.*DescribeNegationTo/,+4d' \
 | sed '/In member.*operator/,+4d' \
 | sed '/In function ‘Print’:/,+4d' \
 | sed '/\/ostream:.*operator/,+2d' \
 | sed '/\/ostream:.*Winline/,+3d' \
 | sed '/\/gtest-internal.* In /,+3d' \
 | sed '/\/gtest-internal/,+2d' \
 | sed '/\/external\/gmsh.*arning/,+3d' \
 | tee $(1).err >&2);
 endif
else
  build_log = 2> >(tee $(1).err >&2);
endif


# make targets ================================================================
.SILENT :

# Prevent some intermediate file removal
# .PRECIOUS : %.o
#  %.gtst
#%.a %.h %.o
# Maybe this should be used only for debbuging?

#NOTE The first two lines of .PHONY targets are intended for users.
#NOTE The rest, for developers and internal makefile use.
.PHONY: all external mini femera libfemera check test-perf \
  install unistall clean cleaner cleanest \
  libmini libfull \
  build-info pre-build-tests post-build-tests done \
  $(BUILD_DIR)/VERSION
  #$(BUILD_DIR)/femera-options.new
#TODO what other targets are needed?
#  femera plugins
#  mini-test test-build-scripts test-gmsh


# Primary named targets -------------------------------------------------------
# These are intended for users to make.
all : mini #femera plugins
	$(call build_timestamp,$@,$?)
	$(MAKE) done

libfemera : pre-build-tests
	$(call build_timestamp,$@,$?)
	$(MAKE) build/libfemera$(INSTALL_SUFFIX).a

mini : libfemera # $(BUILD_DIR)/femera-options.inc
	$(call build_timestamp,$@,$(?F))
	$(MAKE) build/femera-mini$(INSTALL_SUFFIX)

femera : mini
	$(call build_timestamp,$@,$?)

external : $(INSTALL_EXTERNAL)
	$(call build_timestamp,$@,$(BUILD_EXTERNAL_DIR)/*-ok)

# Named targets for developer and internal makefile use. ----------------------

build-info : $(BUILD_DIR)/VERSION $(BUILD_TREE)
	$(call build_timestamp,$@,Femera $(BUILD_VERSION))
ifdef BUILT_BY_INFO_3
	@head -c -1 -q data/modification.txt data/copyright.txt \
  | tr '\n' ' ' | fold -w 80 -s > $(BUILD_DIR)/build-data/copyright.txt
	@echo >> $(BUILD_DIR)/build-data/copyright.txt
	$(call rewrap_file,$(BUILD_DIR)/build-data/copyright.txt)
	#$(call rewrap_file,data/modification.txt)
	#$(call rewrap_file,data/copyright.txt)
	@printf "$(BUILT_BY_INFO_3)\n" > "$(BUILD_DIR)/build-data/built-by.txt"
else
	@head -c -1 -q data/copyright.txt \
  | tr '\n' ' ' | fold -w 80 -s > $(BUILD_DIR)/build-data/copyright.txt
	@echo >> $(BUILD_DIR)/build-data/copyright.txt
	$(call rewrap_file,data/copyright.txt)
	@printf "" > $(BUILD_DIR)/build-data/built-by.txt
endif
	@printf "$(BUILT_BY_MINI_1)\n$(BUILT_ON_INFO)\n" \
  > $(BUILD_DIR)/build-data/build-info.txt
	$(shell grep -m1 -i 'model name' /proc/cpuinfo \
  > $(BUILD_DIR)/build-data/build-detail.txt)
	$(shell printf " Compile & link : ` \
  echo $(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -lfemera \
  | tr -s '[:space:]'`\n" | fold -w 80 -s \
  >> $(BUILD_DIR)/build-data/build-detail.txt)

pre-build-tests : tools/pre_build_test.py build-info ompexec fmrexec \
  $(BUILD_DIR)/Data/cube-tet6p1n1.msh2 $(INSTALL_CINCLUDE2DOT)
	$(call build_timestamp,$@,$<)
	if [ -h "tools/testy" ]; then \
  echo "$(INFO) tools/testy is a link to external/testy."; \
  else if [ -d "tools/testy" ]; then \
    echo "$(INFO) tools/testy is a directory."; \
    else \
      ln -s "../external/testy" "tools/testy"; \
      echo "$(NOTE) tools/testy symlinked to external/testy."; fi; fi
	@external/tools/cinclude2dot --src src >docs/src.dot \
          2>build/src.dot.err #  --groups is nice, too
	$(info $(INFO) Source dependencies: docs/src.dot)
	@grep -v '\.gtst\.' docs/src.dot > docs/src-notest.dot
	$(info $(INFO) Dependencies without tests: docs/src-notest.dot)
ifeq ($(ENABLE_GRAPHIZ),ON)
	@tools/src-inherit.sh
	@dot docs/src-notest.dot -Gsize="10.0,8.0" -Teps -o build/src-notest.eps
	#  -Gratio="fill" -Gsize="11.7,8.267!" -Gmargin=0
	$(info $(INFO) Dependency graph: build/src-notest.eps)
endif
	python tools/pre_build_test.py 'src' $(BUILD_DIR)
	$(MAKE) $(WRAP_SRCS)

post-build-tests : tools/post_build_test.py $(BUILD_DIR)/mini
	$(call build_timestamp,$@,$<)
	$(info $(INFO) $(BUILD_DIR)/mini is \
  $(shell du -h "$(BUILD_DIR)/mini" | cut -f1)B)
	$(PYEXEC) tools/post_build_test.py 'src' $(BUILD_DIR)

ompexec : src/ompexec $(STAGE_DIR)/bin/
	$(call build_timestamp,$@,$?)
	@cp src/ompexec $(STAGE_DIR)/bin/

fmrexec : src/fmr1node src/fmrexec $(STAGE_DIR)/bin/
	$(call build_timestamp,$@,$?)
	@cp src/fmrexec $(STAGE_DIR)/bin/
	@cp src/fmr1node $(STAGE_DIR)/bin/

done : post-build-tests
	$(call build_timestamp,$@,$?)
	$(info $(DONE) $(BUILT_BY_INFO_1))
	$(info $(SPCS) $(BUILT_BY_INFO_2))
	$(info $(SPCS) $(BUILT_BY_INFO_3))
	$(info $(XMPL) $(MPIEXEC) "$(BUILD_DIR)/mini" -d -t -D \)# -n$(TDD_OMP_NP)
	$(info $(SPCS) $(TDD_FMRFILE))


# Installation file (exes, libs) targets --------------------------------------

# Rename files for installation.

build/femera$(INSTALL_SUFFIX) : $(BUILD_DIR)/full $(STAGE_DIR)/bin/
	@cp $< $@
	@cp $< $(STAGE_DIR)/bin/femera

build/femera-mini$(INSTALL_SUFFIX) : $(BUILD_DIR)/mini $(STAGE_DIR)/bin/
	@cp $< $@
	@cp $< $(STAGE_DIR)/bin/femera-mini

build/%$(INSTALL_SUFFIX).a : $(BUILD_DIR)/%.a $(STAGE_DIR)/lib/
	@cp $< $@
	@cp $< $(STAGE_DIR)/lib/$(<F)


# Executables

$(VALGRIND_SUPP_EXE) : src/$(VALGRIND_SUPP).cpp
	$(info $(CLAB) $(CXX) ... -o $(BUILD_DIR)/$(VALGRIND_SUPP).exe)
	$(CXX) $(VGFLAGS) src/$(VALGRIND_SUPP).cpp $(LDFLAGS) -L$(BUILD_DIR)\
  -lfemera -o $(VALGRIND_SUPP_EXE) $(LDLIBS) $(call build_log,$@)
	$(info $(GRND) suppression file: $(BUILD_DIR)/$(VALGRIND_SUPP))
	$(VGSUPP)

$(BUILD_DIR)/mini : $(BUILD_DIR)/libfemera.a
$(BUILD_DIR)/mini : src/mini.cpp src/mini.hpp $(BUILD_DIR)/libuser.a \
 src/mini.test.py $(VALGRIND_SUPP_EXE)
	$(info $(CLAB) $(CXX) ... $< -o $@)
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(CXX) $(GTESTFLAGS) $< $(LDFLAGS) -L$(BUILD_DIR) \
  -lfemera -o $@ $(LDLIBS) $(call build_log,$@)
else
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -L$(BUILD_DIR) \
  -lfemera -o $@ $(LDLIBS) $(call build_log,$@)
endif
	$(PYEXEC) src/mini.test.py $@
ifeq ($(ENABLE_VALGRIND),ON)
	$(info $(GRND) Checking mpiexec ... $(BUILD_DIR)/mini ...)
	$(VGEXEC)
	-grep -i 'lost: [1-9]' $(BUILD_DIR)/mini.valgrind.log \
  | cut -d " " -f 5- | awk '{print "$(WARN) valgrind:",$$0}'
	-grep -i '[1-9] err' $(BUILD_DIR)/mini.valgrind.log \
  | cut -d " " -f 4- | awk '{print "$(WARN) valgrind:",$$0}'
	$(info $(GRND) See: $(BUILD_DIR)/mini.valgrind.log)
endif

#NOTE mini and libfemera.a (containing libmini.a stuff)
#     will be built before this.
$(BUILD_DIR)/full: src/femera.cpp

$(BUILD_DIR)/%.gtst : src/%.gtst.cpp # $(BUILD_DIR)/libcore.a
ifeq ($(ENABLE_GOOGLETEST),ON)
	$(info $(CLAB) $(CXX) ... $< -o $@)
	$(CXX) $(GTESTFLAGS) $< $(LDFLAGS) -L$(BUILD_DIR) \
  -lfemera -o $@ $(LDLIBS) $(call build_log,$@)
	-$(TEST_EXEC) $@ -Tv0 $(TDD_FMRFILE) \
  $(call build_log,$(BUILD_DIR)/$*.gtst)
else
	$(info $(WARN) $@ not tested: Googletest disabled. )
	-echo "WARNING: $@ not tested: Googletest disabled." >> $@.err
endif

# Libraries

ifeq ($(ENABLE_GOOGLETEST),ON)
$(BUILD_DIR)/libuser.a : | $(CORE_GTST)

$(CORE_GTST) : | $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libfemera.a : | $(MINI_GTST)

$(MINI_GTST) : $(BUILD_DIR)/libuser.a
endif

# libfemera archive.
#NOTE that libfemera.a is used to make both femera AND femera-mini executables.

$(BUILD_DIR)/libfemera.a : $(BUILD_DIR)/libcore.a $(BUILD_DIR)/libuser.a

# intermediate archives, used for handling prerequisites and testing
$(BUILD_DIR)/libuser.a : $(MINI_OBJS)
	$(call build_timestamp,$(@F),"")
	$(info $(LIBS) $(AREXE) libfemera.a <-- $(?F))
	@touch $@ # @$(AREXE) -crs $@ $(MINI_OBJS) $(call build_log,$@)
	@$(AREXE) -crs $(BUILD_DIR)/libfemera.a $? $(call build_log,$@)

$(BUILD_DIR)/libuser.a : | $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a : $(CORE_OBJS)
	$(call build_timestamp,$(@F),"")
	$(info $(LIBS) $(AREXE) libfemera.a <-- $(?F))
	@touch $@ # @$(AREXE) -crs $@ $(CORE_OBJS) $(call build_log,$@)
	@$(AREXE) -crs $(BUILD_DIR)/libfemera.a $? $(call build_log,$@)

$(BUILD_DIR)/libfemera.h : $(TODO_lib_h_files) \
  $(BUILD_DIR)/phys_inline.h  #$(BUILD_DIR)/Phys/phys_stub.o
	$(info $(LIBS) cat $(?F) --> $(@F))
	@cat $? >> $@ $(call build_log,$@)


# Intermediate file target patterns -------------------------------------------

ifeq (0,1)
$(BUILD_DIR)/mini.o : src/mini.cpp src/mini.hpp $(BUILD_DIR)/libuser.a
	$(info $(CLAB) $(CXX) ... $< -o $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(call build_log,$@)
	# No test here. Executable mini is tested by src/mini.test.py.
endif

# Every other target .o needs a test.

#$(BUILD_DIR)/%.gtst.o : src/%.cpp src/%.hpp src/%.gtst.cpp
#	$(info $(CLAB) $(CXX) ... $< -o $@)
#	$(CXX) -c $(CXXFLAGS) $< -o $@ $(call build_log,$@)

$(BUILD_DIR)/%.o : src/%.cpp src/%.hpp src/%.gtst.cpp # $(BUILD_DIR)/%.gtst
	-python tools/testy/check_code_graffiti.py $^
	rm -f $*.err $*.gtst.err
	$(info $(CLAB) $(CXX) ... $< -o $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(call build_log,$@)

ifeq (0,1)
$(BUILD_DIR)/%.o $(BUILD_DIR)/%.test.o $(BUILD_DIR)/%.test : \
src/%.cpp src/%.hpp src/%.test.cpp src/%.test.py
#  $(BUILD_DIR)/<parent>.hpp
	-python tools/testy/check_code_graffiti.py $^
	rm -f $*.err $*.test.err
	$(info $(CLAB) $(CXX) ... $< -o $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(call build_log,$@)
	$(info $(CLAB) $(CXX) ... $< -o $(BUILD_DIR)/$*.test.o)
	$(CXX) -c $(CXXFLAGS) src/$*.test.cpp -o $(BUILD_DIR)/$*.test.o \
  $(call build_log,$(BUILD_DIR)/$*.test.o)
	$(info $(LINK) $(CXX) ... -o $(BUILD_DIR)/$*.test)
	$(CXX) $(CXXFLAGS) $(BUILD_DIR)/$*.test.o $< $(LDFLAGS) \
  -o $(BUILD_DIR)/$*.test $(LDLIBS) $(call build_log,$(BUILD_DIR)/$*.test)
	$(PYEXEC) src/$*.test.py "$(@D)/"
endif

# Handle dependancies generated by CXX -MMD -MP ...
#NOTE This needs to be before "no test" recipes.
 -include $(CXXSRCS : %.cpp=%.d)

$(BUILD_DIR)/Proc.o : $(BUILD_DIR)/build-data/copyright.inc

# Warn if no test.
$(BUILD_DIR)/%.o : src/%.cpp src/%.hpp
	-python tools/testy/check_code_graffiti.py $^
	$(info $(WARN) No test of $<.)
	rm -f $*.err
	$(info $(CLAB) $(CXX) ... $< -o $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(call build_log,$@)
	-echo "WARNING: No test of $<." >> $@.err

ifeq (1,0)
# Warn if no header.
$(BUILD_DIR)/%.o : src/%.cpp
	-python tools/testy/check_code_graffiti.py $^
	$(info $(WARN) No $(*).hpp for $<, needed to build $(@F).)
	-echo "WARNING: No $(*).hpp for $<, needed to build $(@F)." >> $@.err
endif

# Build info include targets ---------------------------------------------------
# These only update the target if the content has changed.
# Touches src/Work.cpp because that is where they are used.

ifeq (1,0) #DISABLED femera-options
$(BUILD_DIR)/femera-options.new :
	touch $@

DISABLED:
ifeq ($(ENABLE_LIBNUMA),ON)
	echo "#define FMR_HAS_LIBNUMA" >> $@
endif
ifeq ($(ENABLE_GOOGLETEST),ON)
	echo "#define FMR_HAS_GTEST" >> $@
endif
ifeq ($(ENABLE_MPI),ON)
	echo "#define FMR_HAS_MPI" >> $@
endif
ifeq ($(ENABLE_HDF5),ON)
	echo "#define FMR_HAS_HDF5" >> $@
endif
ifeq ($(ENABLE_CGNS),ON)
	echo "#define FMR_HAS_CGNS" >> $@
endif
ifeq ($(ENABLE_GMSH),ON)
	echo "#define FMR_HAS_GMSH" >> $@
endif
ifeq ($(ENABLE_PETSC),ON)
	echo "#define FMR_HAS_PETSC" >> $@
endif

$(BUILD_DIR)/femera-options.inc : $(BUILD_DIR)/femera-options.new | $(BUILD_TREE)
	if test -r $@; then                              \
	  cmp $@ $< >/dev/null 2>/dev/null || (          \
	    mv -f $< $@;                                 \
	    touch src/*.hpp src/*/*.hpp;                 \
	    echo "$(INFO)" $(@F): has changed. )         \
	else                                             \
	  mv -f $< $@;                                   \
	  touch src/*.hpp src/*/*.hpp;                   \
	  echo "$(INFO)" $(@F): created.;                \
	fi;                                              \
	rm -f $(BUILD_DIR)/femera-options.new
endif #DISABLED femera-options

ifeq ($(ENABLE_GOOGLETEST),ON)
$(BUILD_DIR)/build-data/googletest-version.inc :
	-echo \"$(shell external/get-googletest-version.sh)\" > $@
endif

$(BUILD_DIR)/build-data/%.inc : $(BUILD_DIR)/build-data/%.txt | $(BUILD_TREE)
	tools/wrap-txt2c.sh $< $@.new
	if test -r $@; then                              \
	  cmp $@.new $@ >/dev/null || (                  \
	    python tools/testy/check_code_graffiti.py $?;\
	    mv -f $@.new $@;                             \
	    touch src/Proc.cpp;                          \
	    echo "$(WRAP)" $< as $(@F) )                 \
	else                                             \
	  python tools/testy/check_code_graffiti.py $?;  \
	  mv $@.new $@; echo "$(WRAP)" $< as $(@F);      \
	  touch src/Proc.cpp;                            \
	fi

$(BUILD_DIR)/build-data/%.inc : data/%.txt | $(BUILD_TREE)
	tools/wrap-txt2c.sh $< $@.new
	if test -r $@; then                              \
	  cmp $@.new $@ >/dev/null || (                  \
	    python tools/testy/check_code_graffiti.py $?;\
	    mv -f $@.new $@;                             \
	    touch src/Proc.cpp;                          \
	    echo "$(WRAP)" $< as $(@) )                  \
	else                                             \
	  python tools/testy/check_code_graffiti.py $?;  \
	  mv $@.new $@; echo "$(WRAP)" $< as $(@F);      \
	  touch src/Proc.cpp;                            \
	fi

# Build directory structure targets and patterns ------------------------------

$(BUILD_DIR)/VERSION : | $(BUILD_TREE)
	echo $(BUILD_VERSION) > $(BUILD_DIR)/VERSION.new
	if test -r $@; then                                   \
	  cmp $(BUILD_DIR)/VERSION.new $@ >/dev/null ||       \
	    (mv -f $(BUILD_DIR)/VERSION.new $@;               \
	    touch src/Proc.cpp src/Work.cpp;                  \
	    echo "$(INFO)" Set version to $(BUILD_VERSION). ) \
	else                                                  \
	  mv $(BUILD_DIR)/VERSION.new $@;                     \
	  touch src/Proc.cpp src/Work.cpp;                    \
	  echo "$(INFO)" Set version to $(BUILD_VERSION).;    \
	fi
	echo $(CXXFLAGS) > $(BUILD_DIR)/CXXFLAGS


# Targets for additional tests ------------------------------------------------
# These include integration, end-to-end, and performance tests.

$(BUILD_DIR)/Data/cube-tet6p1n1.msh2 : tests/mesh-tests.sh
	$(call build_timestamp,$(@F),$?)
	$(shell $< $(BUILD_DIR)/Data)

#test-perf : test-gmsh
#	$(call build_timestamp,$@,$?)
#
#test-gmsh : $(TEST_DIR)/test-gmsh.log
#	$(call build_timestamp,$@,$?)
#
#$(TEST_DIR)/test-gmsh.log : tests/test-gmsh.sh tests/geo/uhxt-cube.geo
#	tests/test-gmsh.sh > $(TEST_DIR)/test-gmsh.log
#	tests/print-test-results.sh "" "" $(TEST_DIR)/test-gmsh.log


# Installation targets --------------------------------------------------------

install :
	$(call build_timestamp,$@,$?)
	-rsync -a $(STAGE_DIR) $(INSTALL_DIR)/

uninstall :
	$(call build_timestamp,$@,$?)
	-rm -f $(INSTALL_DIR)/bin/femera$(INSTALL_SUFFIX)
	-rm -f $(INSTALL_DIR)/bin/femera-mini$(INSTALL_SUFFIX)
	-rm -f $(INSTALL_DIR)/lib/libfemera$(INSTALL_SUFFIX).a
	-rm -r $(INSTALL_DIR)/include/femera$(INSTALL_SUFFIX).h


# External targets ------------------------------------------------------------
external/tools/cinclude2dot :
	$(call build_timestamp,cinclude2dot,$<)
ifeq ($(ENABLE_GRAPHIZ),ON)
	-mkdir -p external/tools
	-$(shell cd external/tools; \
  wget https://www.flourish.org/cinclude2dot/cinclude2dot; \
  chmod +x cinclude2dot )
endif

$(BUILD_EXTERNAL_DIR)/CGNS-ok : external/build-cgns.sh \
  $(BUILD_EXTERNAL_DIR)/hdf5-ok |$(BUILD_TREE)
	$(call build_timestamp,CGNS,$<)
	external/build-cgns.sh "`pwd`/$(BUILD_EXTERNAL_DIR)/CGNS" \
  "$(INSTALL_DIR)" && touch "$(BUILD_EXTERNAL_DIR)/CGNS-ok"

$(BUILD_EXTERNAL_DIR)/gmsh-ok : external/build-gmsh.sh $(GMSH_REQUIRES) \
  | $(BUILD_TREE)
	$(call build_timestamp,gmsh,$<)
	external/build-gmsh.sh "`pwd`/$(BUILD_EXTERNAL_DIR)/gmsh" \
  "$(INSTALL_DIR)" "$(GMSH_FLAGS)" && touch "$(BUILD_EXTERNAL_DIR)/gmsh-ok"

$(BUILD_EXTERNAL_DIR)/petsc-ok : external/build-petsc.sh $(PETSC_REQUIRES) \
  | $(BUILD_TREE)
	$(call build_timestamp,PETSc,$<)
	external/build-petsc.sh "`pwd`/$(BUILD_EXTERNAL_DIR)/petsc" \
  "$(INSTALL_DIR)" "$(PETSC_FLAGS)" && touch "$(BUILD_EXTERNAL_DIR)/petsc-ok"

$(BUILD_EXTERNAL_DIR)/%-ok : external/build-%.sh | $(BUILD_TREE)
	$(call build_timestamp,$*,$<)
	external/build-$*.sh "`pwd`/$(BUILD_EXTERNAL_DIR)/$*" \
  "$(INSTALL_DIR)" && touch "$(BUILD_EXTERNAL_DIR)/$*-ok"

$(BUILD_EXTERNAL_DIR)/%-flags : $(BUILD_EXTERNAL_DIR)/%-flags.new
	if test -r $@; then                         \
	  cmp $@.new $@ >/dev/null ||               \
	    (mv -f $@.new $@;                       \
	    echo "$(FLAG)" $< to $(@) )             \
	else                                        \
	  mv $@.new $@; echo "$(FLAG)" $< to $(@F); \
	fi

$(BUILD_EXTERNAL_DIR)/gmsh-flags.new : |$(BUILD_TREE)
	$(shell echo $(GMSH_FLAGS) > $(BUILD_EXTERNAL_DIR)/gmsh-flags.new )

$(BUILD_EXTERNAL_DIR)/petsc-flags.new : |$(BUILD_TREE)
	$(shell echo $(GMSH_FLAGS) > $(BUILD_EXTERNAL_DIR)/petsc-flags.new )

# Cleanup targets -------------------------------------------------------------

clean : $(BUILD_DIR)/
	$(call build_timestamp,$@,$<)
	-rm -rf $(BUILD_DIR)
	$(info  $(BUILD_HOST) made Femera $(BUILD_VERSION) $@ \
	  for $(shell tools/cpumodel.sh). )

cleaner : clean clean-googletest/ clean-pybind11/ clean-hdf5/ clean-CGNS/ \
  clean-gmsh/
	$(call build_timestamp,$@,$< [clean-*])
	-rm -f build/femera$(INSTALL_SUFFIX)
	-rm -f build/femera-mini$(INSTALL_SUFFIX)
	-rm -f build/libfemera$(INSTALL_SUFFIX).a
	$(info $(BUILD_DATE) $(BUILD_HOST) made $@ \
	  for $(shell tools/cpumodel.sh). )

cleanest :
	$(call build_timestamp,$@,build/ removed)
	-rm -rf build
	$(info $(BUILD_DATE) $(BUILD_HOST) made $@. )

clean-%/ :
	$(call build_timestamp,$@,$(BUILD_EXTERNAL_DIR)/$*/ removed)
	-rm -f  $(BUILD_EXTERNAL_DIR)/$*-ok
	-rm -rf $(BUILD_EXTERNAL_DIR)/$*

%/ :
	@mkdir -p $@

