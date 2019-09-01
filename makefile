# Defaults: use g++
CPPFLAGS=-std=c++11 -Wall -Wextra -g -Ofast -ftree-vectorize -march=native \
 -mtune=native -fno-builtin-sin -fno-builtin-cos
#FIXME -mtune=core-avx2 when -mtune=native doesn't work
OMPFLAGS=-fopenmp
SERFLAGS=-Wno-unknown-pragmas
NUMA=2
CSTR=gcc

# Check for intel compiler
ifdef INTEL_LICENSE_FILE
CXX=icc
CPPFLAGS=-std=c++11 -Wall -Wextra -Ofast -xHost -axSKYLAKE-AVX512 \
 -ffast-math -no-fast-transcendentals \
 -no-inline-max-size -no-inline-max-total-size -g
# -march=native
SERFLAGS=-fno-alias -diag-disable 3180
NUMA=2
CSTR=icc
endif

# Check if using intel compiler on k3 or k4
ifdef INTEL_PYTHONHOME
CXX=icc
CPPFLAGS=-restrict -std=c++11 -Wall -Wextra -O2 -ansi-alias\
 -ffast-math -no-fast-transcendentals\
 -no-inline-max-size -no-inline-max-total-size -xSKYLAKE-AVX512 -g
SERFLAGS=-fno-alias -diag-disable 3180
NUMA=6
CSTR=icc
endif

NCPU:=$(shell ./cpucount.sh)
CPUMODEL:=$(shell ./cpumodel.sh)
CPUMODELC:=$(CPUMODEL)-$(CSTR)
HOST2CHAR:=$(shell hostname | cut -c1-2 )

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections
# -Wsuggest-final-types and -Wsuggest-final-methods -O3 -flto

# CPPLOG=-fopt-info-vec-optimized 2>a.log ; grep -i vectorized a.log ; grep -i warn a.log; grep -i error a.log

FEMERA_COMMON = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc\
 halo-pcg-omp.cc halo-ncg-omp.cc halo-pcr-dummy.cc \
 elas-iso3.cc elas-ort3.cc elas-ther-ort3.cc

ifeq ($(HOST2CHAR), k2)
FEMERA_MINI_C = $(FEMERA_COMMON) elas-iso3-base.cc elas-ort3-bas2.cc elas-ther-ort3-bas2.cc
else
FEMERA_MINI_C = $(FEMERA_COMMON) elas-iso3-vect.cc elas-ort3-vec2.cc elas-ther-ort3-vec2.cc
endif

FEMERA_BASE_C = $(FEMERA_COMMON)\
 elas-iso3-base.cc elas-ort3-bas2.cc elas-ther-ort3-bas2.cc
 
FEMERA_REF_C = $(FEMERA_COMMON)\
 elas-iso3-ref.cc elas-ort3-ref2.cc elas-ther-ort3-ref2.cc

FEMERA_NAIV_C = $(FEMERA_COMMON)\
 elas-iso3-ref.cc elas-ort3-nai2.cc elas-ther-ort3-ref2.cc


HYBRID_GCC_C = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc\
 halo-pcg-omp.cc halo-ncg-omp.cc halo-pcr-dummy.cc\
 elas-iso3.cc elas-ort3.cc elas-ther-ort3.cc

HYBRID_ICC_C = elas-iso3-vect.cc elas-ort3-vec2.cc elas-ther-ort3-vec2.cc

CEXT = cc
ODIR = mini.o

OEXT = $(CPUMODEL).$(CSTR).o
QEXT = qui.$(CPUMODEL).$(CSTR).o
SEXT = ser.$(CPUMODEL).$(CSTR).o
IEXT = $(CPUMODEL).icc.o
GEXT = $(CPUMODEL).gcc.o

OBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(OEXT)))
QBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(QEXT)))
SBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(SEXT)))
GBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_GCC_C:.$(CEXT)=.$(GEXT)))
IBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_ICC_C:.$(CEXT)=.$(IEXT)))

all : gmsh2fmr-ser mini-omp mini-omq

$(ODIR)/%.$(OEXT) : %.cc
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DHAS_TEST \
	$< -o $@

$(ODIR)/%.$(OEXT) : %.h
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DHAS_TEST \
	$< -o $@

$(ODIR)/%.$(QEXT) : %.cc
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	$< -o $@

$(ODIR)/%.$(QEXT) : %.h
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	$< -o $@

$(ODIR)/%.$(SEXT) : %.cc
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DFETCH_JAC \
	$< -o $@

$(ODIR)/%.$(SEXT) : %.h
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DFETCH_JAC \
	$< -o $@

mini-hyb: $(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODEL)-hyb $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v ./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16 ;

mini-omp : $(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODEL)-$(CSTR) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v ./femera-$(CPUMODEL)-$(CSTR) -v2 -c$(NCPU) -p cube/unst19p1n16 ;

mini-omq : $(QBJS) $(ODIR)/femera-mini.$(QEXT)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(QBJS) $(ODIR)/femera-mini.$(QEXT) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	-o femerq-$(CPUMODEL)-$(CSTR) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v ./femerq-$(CPUMODEL)-$(CSTR) -v1 -c$(NCPU) -p cube/unst19p1n16 ;

gmsh2fmr-ser : $(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT)
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT) \
	-o gmsh2fmr-$(CPUMODEL)-$(CSTR) ;
	./gmsh2fmr-$(CPUMODEL)-$(CSTR) -t666 -x0 -t111 -z0 -t333 -y0 -t444 -xu 0.001 \
	-M1 -E100e9 -N0.3 -A20e-6 -K100e-6 -Z1 -X0 -Z0 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-v3 -ap cube/unit1p2n2;
	./gmsh2fmr-$(CPUMODEL)-$(CSTR) -t666 -x0 -t333 -y0 -t111 -z0 -t444 -xu 0.001 -t444 -Tu 10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R \
	-v3 -ap cube/unit1p2n2;

clean :
	-rm $(ODIR)/*$(CPUMODEL)*;
	-rm *-$(CPUMODEL)-*;