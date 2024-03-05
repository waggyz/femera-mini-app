NCPU:=$(shell ./cpucount.sh)
CPUMODEL:=$(shell ./cpumodel.sh)
CPUSIMD:=$(shell ./cpusimd.sh)
HOST2CHAR:=$(shell hostname | cut -c1-2 )

# Defaults use g++
CSTR=gcc
CPPFLAGS=-std=c++11 -Wall -Wextra -g -Ofast -ftree-vectorize -march=native\
 -mtune=native -flto -fearly-inlining -funroll-loops\
 -fno-builtin-sin -fno-builtin-cos
#FIXME -mtune=core-avx2 when -mtune=native doesn't work
OMPFLAGS=-fopenmp -D_GLIBCXX_PARALLEL
# -DOMP_PLACES=cores -DOMP_PROC_BIND=spread -DOMP_NUM_THREADS=$(NCPU)
SERFLAGS=-Wno-unknown-pragmas
# CPPLOG=-fopt-info-vec-optimized 2>>$(CPUMODEL)-$(CSTR).err
# AUTOVEC_SUMMARY=grep -i vectorized $(CPUMODEL)-$(CSTR).err | grep -v " 0 ";

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections

# Check for intel compiler
ifdef INTEL_LICENSE_FILE
CXX=icc
CSTR=icc
CPPFLAGS=-std=-restrict c++11 -mkl=sequential -Wall -Wextra -Ofast -xHost\
 -ffast-math -no-fast-transcendentals\
 -no-inline-max-size -no-inline-max-total-size -qoverride-limits -g\
 -DMKL_DIRECT_CALL_SEQ
SERFLAGS=-fno-alias -diag-disable 3180 -g
endif

# Check if using intel compiler on k3 or k4
ifdef INTEL_PYTHONHOME
CXX=icc
CSTR=icc
SERFLAGS=-fno-alias -diag-disable 3180 -g
CPPFLAGS=-restrict -std=c++11 -mkl=sequential -Wall -Wextra -O2 -ansi-alias\
 -ffast-math -no-fast-transcendentals\
 -no-inline-max-size -no-inline-max-total-size -qoverride-limits -g\
 -DMKL_DIRECT_CALL_SEQ

#ifneq (,$(findstring 512,$(CPUSIMD)))
# CPPFLAGS:=$(CPPFLAGS) -xSKYLAKE-AVX512
# Does not work for KNL
# # -qopt-zmm-usage=high degrades performance 20%
#else
 CPPFLAGS:=$(CPPFLAGS) -xHost
#endif

# Activate with: make COLLECT_VTUNE_DATA=1 mini-all
ifdef COLLECT_VTUNE_DATA
CPPFLAGS:=$(CPPFLAGS) -DCOLLECT_VTUNE_DATA
LDFLAGS:=$(LDFLAGS) -I$(VTUNE_AMPLIFIER_XE_2019_DIR)/include
LDFLAGS:=$(LDFLAGS) -L$(VTUNE_AMPLIFIER_XE_2019_DIR)/lib64 -littnotify
$(warning **WARNING** Compiling to collect vector advisor data.)
endif

#  -axSKYLAKE-AVX512 can run out of memory
# CPPLOG="-Wsuggest-final-types -Wsuggest-final-methods\
#  -fopt-info-vec-optimized 2>>$(CPUMODEL)-$(CSTR).err;
# -Wsuggest-final-types -Wsuggest-final-methods -O3 -flto
endif

# Check if pragma omp simd is supported.
HAS_PRAGMA_SIMD:=$(shell $(CXX) $(OMPFLAGS)\
 -Wunknown-pragmas unit-test/test-pragma-simd.c\
 -o /dev/null >& /dev/stdout | grep -i ignoring )
ifeq ($(strip $(HAS_PRAGMA_SIMD)),)
  OMPFLAGS:=$(OMPFLAGS) -DHAS_PRAGMA_SIMD
endif

# Check if AVX2 is supported.
ifneq (,$(findstring AVX2,$(CPUSIMD)))
  CPPFLAGS:=$(CPPFLAGS) -DHAS_AVX2
endif

OMPFLAGS:=$(OMPFLAGS) -DFETCH_JAC

MMPFLAGS = -DOMP_NESTED=true -DOMP_PROC_BIND=spread,close

# Saves u and f info for nodes with applied boundary conditions
CPPFLAGS+= -DSAVE_BCS_INFO

CPUMODELC:=$(CPUMODEL)-$(CSTR)

FEMERA_COMMON = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc\
 halo-pcg-omp.cc halo-ncg-omp.cc halo-pcr-dummy.cc\
 elas-iso3.cc elas-ort3.cc elas-dmv3.cc\
 elas-plkh-iso3.cc elas-ther-iso3.cc elas-ther-ort3.cc\
 ther-iso3.cc

FEMERA_BASE_C = $(FEMERA_COMMON)\
 elas-iso3-base.cc elas-ort3-bas2.cc elas-dmv3-base.cc\
 elas-plkh-iso3-dum.cc elas-ther-iso3-base.cc elas-ther-ort3-bas2.cc\
 ther-iso3-base.cc

ifneq (,$(findstring AVX,$(CPUSIMD)))
 CPPFLAGS:=$(CPPFLAGS) -DHAS_AVX
FEMERA_MINI_C = $(FEMERA_COMMON)\
 elas-iso3-vect.cc elas-ort3-vec2.cc elas-dmv3-vect.cc\
 elas-plkh-iso3-vect.cc elas-ther-iso3-base.cc elas-ther-ort3-bas2.cc\
 ther-iso3-vect.cc
else
FEMERA_MINI_C = $(FEMERA_BASE_C)
endif
#FIXME Intel does not compile DMAT driver?

FEMERA_REF_C = $(FEMERA_COMMON)\
 elas-iso3-ref.cc elas-ort3-ref2.cc elas-dmv3-base.cc\
 elas-plkh-iso3-ref.cc elas-ther-iso3-base.cc elas-ther-ort3-ref2.cc\
 ther-iso3-base.cc

FEMERA_NAIV_C = $(FEMERA_COMMON)\
 elas-iso3-ref.cc elas-ort3-nai2.cc elas-dmv3-base.cc\
 elas-plkh-iso3-ref.cc elas-ther-iso3-base.cc elas-ther-ort3-ref2.cc\
 ther-iso3-base.cc

FEMERA_STIF_C = $(FEMERA_COMMON)\
 elas-lms3-base.cc elas-ort3-bas2.cc elas-dmv3-base.cc\
 elas-plkh-iso3-dum.cc elas-ther-iso3-base.cc elas-ther-ort3-bas2.cc\
 ther-lms3-base.cc

HYBRID_GCC_C = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc\
 halo-pcg-omp.cc halo-ncg-omp.cc halo-pcr-dummy.cc\
 elas-iso3.cc elas-ort3.cc elas-ther-iso3.cc elas-ther-ort3.cc

HYBRID_ICC_C = elas-iso3-vect.cc elas-ort3-vec2.cc\
 elas-ther-iso3-base.cc elas-ther-ort3-vec2.cc\
 elas-plkh-iso3-ref.cc ther-iso3-vect.cc

CEXT = cc
ODIR = mini.o

OEXT = $(CPUMODEL).$(CSTR).o
QEXT = qui.$(CPUMODEL).$(CSTR).o
SEXT = ser.$(CPUMODEL).$(CSTR).o
REXT = ref.$(CPUMODEL).$(CSTR).o
IEXT = $(CPUMODEL).icc.o
GEXT = $(CPUMODEL).gcc.o
MEXT = mmp.$(CPUMODEL).$(CSTR).o
NEXT = mmq.$(CPUMODEL).$(CSTR).o
KEXT = lms.$(CPUMODEL).$(CSTR).o
LEXT = lmq.$(CPUMODEL).$(CSTR).o

OBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(OEXT)))
QBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(QEXT)))
MBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(MEXT)))
NBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(NEXT)))
BBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_BASE_C:.$(CEXT)=.$(OEXT)))
SBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(SEXT)))
RBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_REF_C:.$(CEXT)=.$(REXT)))
GBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_GCC_C:.$(CEXT)=.$(GEXT)))
IBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_ICC_C:.$(CEXT)=.$(IEXT)))
KBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_STIF_C:.$(CEXT)=.$(KEXT)))
LBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_STIF_C:.$(CEXT)=.$(LEXT)))

TESTDIR = test/$(CPUMODEL)
PERFDIR = perf

_dummy := $(shell mkdir -p mini.o test $(TESTDIR) $(PERFDIR))

.SILENT :

all : mini-all gmsh2fmr

mini-all : mini-omp mini-omq mini-mmp mini-mmq mini-lms mini-lmq

test : all
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M1 -E100e9 -N0.3 -A20e-6 -K100e-6 -Z1 -X0 -Z0 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-ap cube/unit1p2n2;
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R \
	-ap cube/unit1p2n2;
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -R \
	-ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	echo ./femerq-$(CPUMODELC) -v1 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femerq-$(CPUMODELC) -v1 -c$(NCPU) -p cube/unst19p1n16

test-iso : mini-omp gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-ort : mini-omp gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -R -ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-dmv : mini-omp gmsh2fmr
	./gmsh2fmr-$(CPUMODEL)-gcc -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -D -ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-cmv : mini-omp gmsh2fmr
	$(warning * WARNING * gmsh2fmr does not work for elas-cmv.)
	./gmsh2fmr-$(CPUMODEL)-gcc -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -D -R -ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-mmp : mini-mmp
	echo ./femera-mmp-$(CPUMODELC) -v2 -m8 -n2 -c2 -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread,close; \
	export OMP_NESTED=true; export OMP_MAX_ACTIVE_LEVELS=2; \
	./femera-mmp-$(CPUMODELC) -v2 -m8 -n2 -c2 \
	-p cube/unst19p1n16

test-lms : mini-lms gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -ap cube/unst19p1n16;
	echo ./femerk-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-thermal : mini-omp gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v3 -x@0.0 -x0 -x@1.0 -xu10 -M0 -K100e-6 \
	-ap cube/unit1p2n2;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -d0 -p cube/unit1p2n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	export OMP_NUM_THREADS=1;
	./femera-$(CPUMODELC) -v3 -c1 -d0 -p cube/unit1p2n2
	./gmsh2fmr-$(CPUMODEL) -v1 -x@0.0 -x0 -x@1.0 -xu10 -M0 -K100e-6 \
	-ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

test-thermelas : mini-omp gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K10e3 \
	-ap cube/unit1p1n2;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -d0 -p cube/unit1p1n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -d1 -p cube/unit1p1n2
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K10e3 -R \
	-ap cube/unit1p2n2;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -d0 -p cube/unit1p2n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -d1 -p cube/unit1p2n2

placeholder :
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R \
	-ap cube/unit1p2n2;
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -d0 -p cube/unit1p2n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -d1 -p cube/unit1p2n2

test-plastic :  gmsh2fmr mini-omp
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.005 \
	-M0 -E66.2e9 -N0.33 -J305e6 -J100e6 \
	-ap cube/unit1p1n2;
	echo ./femera-$(CPUMODELC) -v3 -s2 -c1 -p cube/unit1p1n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODELC) -v3 -s2 -c1 -p cube/unit1p1n2

test-plastic-20 :  gmsh2fmr mini-omp
	./gmsh2fmr-$(CPUMODEL) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.02 \
	-M0 -E66.2e9 -N0.33 -J305e6 -J100e6 \
	-ap cube/unst19p1n16;
	echo ./femera-$(CPUMODELC) -v2 -s2 -d2 -I20 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODELC) -v2 -s2 -d2 -I20 -c$(NCPU) -p cube/unst19p1n16

ref-plastic : gmsh2fmr mini-ref
	./gmsh2fmr-$(CPUMODEC) -v1 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.005 \
	-M0 -E66.2e9 -N0.33 -J305e6 -J100e6 \
	-ap cube/unit1p1n2;
	echo ./refera-$(CPUMODELC) -v3 -s2 -c$(NCPU) -p cube/unit1p1n2
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./refera-$(CPUMODELC) -v3 -s2 -c$(NCPU) -p cube/unit1p1n2

test-slice : gmsh2fmr mini-omp
	gmsh -v 3 -nt $(NCPU) -part 8 \
	-format msh2 -o cube/uhxt10p2/uhxt10p2n8.msh \
	cube/uhxt10p2/uhxt10p2n.msh -
	./gmsh2fmr-$(CPUMODEL) -v4 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -a cube/uhxt10p2/uhxt10p2n8;
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/uhxt10p2/uhxt10p2n8
	export OMP_SCHEDULE=static;\
	export OMP_PLACES=cores;\
	export OMP_PROC_BIND=spread;\
	export OMP_NUM_THREADS=$(NCPU);\
	./gmsh2fmr-$(CPUMODEL) -v4 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -xyzS2 -a cube/uhxt10p2/uhxt10p2n;
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/uhxt10p2/uhxt10p2n8

test-gmsh2fmr : gmsh2fmr
	./gmsh2fmr-$(CPUMODEL) -v3 \
	-x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M1 -E100e9 -N0.3 -A20e-6 -K100e-6 -Z1 -X0 -Z0 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-ap cube/unit1p2n2;

$(ODIR)/%.$(OEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(KEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(QEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(LEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(MEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(MMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(NEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(MMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(DEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(EEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(SEXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(REXT) : %.cc *.h  phys-inline.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	$< -o $@ $(CPPLOG)

mini-omp : test-scripts femera-$(CPUMODELC)

mini-omq : test-scripts femerq-$(CPUMODELC)

mini-mmp : test-scripts femera-mmp-$(CPUMODELC)

mini-mmq : test-scripts femera-mmq-$(CPUMODELC)

mini-ref : test-scripts refera-$(CPUMODELC)

base-omp : test-scripts femerb-$(CPUMODELC)

mini-lms : test-scripts femerk-$(CPUMODELC)

mini-lmq : test-scripts femeqk-$(CPUMODELC)

mini-dmq : test-scripts femeqd-$(CPUMODELC)

mini-hyb : test-scripts femera-$(CPUMODEL)-hyb

ifeq ($(CSTR),gcc)
gmsh2fmr : gmsh2fmr-$(CPUMODEL)
	echo ok.

gmsh2fmr-$(CPUMODEL) : gmsh2fmr-omp-$(CPUMODELC)
	rm -f gmsh2fmr-$(CPUMODELC) gmsh2fmr-$(CPUMODEL)
	cp gmsh2fmr-omp-$(CPUMODELC) gmsh2fmr-$(CPUMODELC)
	cp gmsh2fmr-omp-$(CPUMODELC) gmsh2fmr-$(CPUMODEL)

else
gmsh2fmr : gmsh2fmr-$(CPUMODEL)
	echo ok. > /dev/null

gmsh2fmr-$(CPUMODEL) :
	echo Please use GNU gcc to compile gmsh2fmr.

endif

femera-$(CPUMODELC) : $(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femera-$(CPUMODELC) $(CPPLOG); $(AUTOVEC_SUMMARY)

femerq-$(CPUMODELC) : $(QBJS) $(ODIR)/femera-mini.$(QEXT)
	echo $(CXX) ... -o femerq-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(QBJS) $(ODIR)/femera-mini.$(QEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	-o femerq-$(CPUMODELC) $(CPPLOG);

femerk-$(CPUMODELC) : $(KBJS) $(ODIR)/test.$(KEXT) $(ODIR)/femera-mini.$(KEXT)
	echo $(CXX) ... -o femerk-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(KBJS) $(ODIR)/test.$(KEXT) $(ODIR)/femera-mini.$(KEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femerk-$(CPUMODELC) $(CPPLOG);

femeqk-$(CPUMODELC) : $(LBJS) $(ODIR)/femera-mini.$(LEXT)
	echo $(CXX) ... -o femeqk-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(LBJS) $(ODIR)/femera-mini.$(LEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DVERB_MAX=1 \
	-o femeqk-$(CPUMODELC) $(CPPLOG);

femera-mmp-$(CPUMODELC) : $(MBJS) $(ODIR)/test.$(MEXT) $(ODIR)/femera-mini.$(MEXT)
	echo $(CXX) ... -o femera-mmp-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) -DOMP_NESTED=true $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(MBJS) $(ODIR)/test.$(MEXT) $(ODIR)/femera-mini.$(MEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femera-mmp-$(CPUMODELC) $(CPPLOG); $(AUTOVEC_SUMMARY)

femera-mmq-$(CPUMODELC) : $(NBJS) $(ODIR)/test.$(NEXT) $(ODIR)/femera-mini.$(NEXT)
	echo $(CXX) ... -o femera-mmq-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) -DOMP_NESTED=true $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(NBJS) $(ODIR)/test.$(NEXT) $(ODIR)/femera-mini.$(NEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femera-mmq-$(CPUMODELC) $(CPPLOG); $(AUTOVEC_SUMMARY)

femerb-$(CPUMODELC) : $(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femerb-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femerb-$(CPUMODELC) $(CPPLOG);
	echo ./femerb-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femerb-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

refera-$(CPUMODELC) : $(RBJS) $(ODIR)/test.$(REXT) $(ODIR)/femera-mini.$(REXT)
	echo $(CXX) ... -o femera-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(RBJS) $(ODIR)/test.$(REXT) $(ODIR)/femera-mini.$(REXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o refera-$(CPUMODELC) $(CPPLOG); $(AUTOVEC_SUMMARY)

femera-$(CPUMODEL)-hyb : $(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODEL)-hyb
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) $(LDFLAGS) \
	-DOMP_SCHEDULE=static -DHAS_TEST \
	-o femera-$(CPUMODEL)-hyb $(CPPLOG);
	echo ./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16

gmsh2fmr-ser : test-scripts gmsh2fmr-ser-$(CPUMODELC)

gmsh2fmr-omp : test-scripts gmsh2fmr-omp-$(CPUMODELC)

gmsh2fmr-omp-$(CPUMODELC) : $(OBJS) $(ODIR)/gmsh2.$(OEXT) $(ODIR)/gmsh2fmr.$(OEXT)
	echo $(CXX) ... -o gmsh2fmr-omp-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(OBJS) $(ODIR)/gmsh2.$(OEXT) $(ODIR)/gmsh2fmr.$(OEXT) \
	-o gmsh2fmr-omp-$(CPUMODELC) ;

gmsh2fmr-ser-$(CPUMODELC) : $(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT)
	echo $(CXX) ... -o gmsh2fmr-ser-$(CPUMODELC)
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT) \
	-o gmsh2fmr-ser-$(CPUMODELC) ;

profile : profile-basic profile-small profile-large

profile-basic : mini-omq gmsh2fmr
	$(PERFDIR)/profile-basic.sh > $(PERFDIR)/profile-basic-$(CPUMODELC).log

profile-small : mini-omq gmsh2fmr
	$(PERFDIR)/profile-small.sh > $(PERFDIR)/profile-small-$(CPUMODELC).log

profile-large : mini-omq gmsh2fmr
	$(PERFDIR)/profile-large.sh > $(PERFDIR)/profile-large-$(CPUMODELC).log

unit-test : test-scripts test-gmsh

test-scripts : $(TESTDIR)/cpucount.sh.err $(TESTDIR)/cpumodel.sh.err $(TESTDIR)/cpusimd.sh.err

$(TESTDIR)/%.sh.err : %.sh unit-test/%.sh.chk
	unit-test/$<.chk > $(TESTDIR)/$<.err ;
	unit-test/print-test-results.sh "$<" "$(TESTDIR)/$<.err"

test-gmsh : $(TESTDIR)/test-gmsh.err
	echo ok.

$(TESTDIR)/test-gmsh.err : unit-test/test-gmsh.sh geo/unst-cube.geo
	unit-test/test-gmsh.sh > $(TESTDIR)/test-gmsh.err
	unit-test/print-test-results.sh "" $(TESTDIR)/test-gmsh.err

clean-test :
	-rm -f $(TESTDIR)/*.err
	-rm -f $(TESTDIR)/*.log

clean-perf :
	-rm -r $(PERFDIR)/*-$(CPUMODELC).log
	-rm -r $(PERFDIR)/*-$(CPUMODELC).csv
	-rm -r $(PERFDIR)/*-$(CPUMODELC).pro

clean :
	-rm -f *-$(CPUMODEL)-*;
	-rm -f $(ODIR)/*.$(CPUMODEL)*;
	-rm -f $(CPUMODELC).err
	-rm -f $(CPUMODELC).log

cleaner : clean clean-test clean-gmsh2fmr

clean-gmsh2fmr :
	-rm -f gmsh2fmr-$(CPUMODEL)*
	-rm -f gmsh2fmr*-$(CPUMODEL)-*;

cleanest : cleaner clean-perf
	-rm -r $(PERFDIR)/*-$(CPUMODELC)*
