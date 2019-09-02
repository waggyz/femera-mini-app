NCPU:=$(shell ./cpucount.sh)
CPUMODEL:=$(shell ./cpumodel.sh)
HOST2CHAR:=$(shell hostname | cut -c1-2 )
# Defaults: use g++
CSTR=gcc
CPPFLAGS=-std=c++11 -Wall -Wextra -g -Ofast -ftree-vectorize -march=native \
 -mtune=native -fno-builtin-sin -fno-builtin-cos
#FIXME -mtune=core-avx2 when -mtune=native doesn't work
OMPFLAGS=-fopenmp
SERFLAGS=-Wno-unknown-pragmas
# CPPLOG=-fopt-info-vec-optimized 2>>$(CPUMODEL)-$(CSTR).err
# AUTOVEC_SUMMARY=grep -i vectorized $(CPUMODEL)-$(CSTR).err | grep -v " 0 ";
NUMA=2

# Check for intel compiler
ifdef INTEL_LICENSE_FILE
CXX=icc
CSTR=icc
CPPFLAGS=-std=c++11 -Wall -Wextra -Ofast -xHost -axSKYLAKE-AVX512 \
 -ffast-math -no-fast-transcendentals \
 -no-inline-max-size -no-inline-max-total-size -g
# -march=native
SERFLAGS=-fno-alias -diag-disable 3180
NUMA=2
endif

# Check if using intel compiler on k3 or k4
ifdef INTEL_PYTHONHOME
CXX=icc
CSTR=icc
CPPFLAGS=-restrict -std=c++11 -Wall -Wextra -O2 -ansi-alias\
 -ffast-math -no-fast-transcendentals\
 -no-inline-max-size -no-inline-max-total-size -xSKYLAKE-AVX512 -g
SERFLAGS=-fno-alias -diag-disable 3180
# CPPLOG="-Wsuggest-final-types -Wsuggest-final-methods\
#  -fopt-info-vec-optimized 2>>$(CPUMODEL)-$(CSTR).err;
NUMA=6
endif

CPUMODELC:=$(CPUMODEL)-$(CSTR)

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections
# -Wsuggest-final-types and -Wsuggest-final-methods -O3 -flto


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

BBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_BASE_C:.$(CEXT)=.$(OEXT)))
OBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(OEXT)))
QBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(QEXT)))
SBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(SEXT)))
GBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_GCC_C:.$(CEXT)=.$(GEXT)))
IBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_ICC_C:.$(CEXT)=.$(IEXT)))

.SILENT :

all : gmsh2fmr-ser mini-omp mini-omq

$(ODIR)/%.$(OEXT) : %.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(OEXT) : %.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(QEXT) : %.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(QEXT) : %.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(SEXT) : %.cc
	echo $(CXX) ... -o $@
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DFETCH_JAC \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(SEXT) : %.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DFETCH_JAC \
	$< -o $@ $(CPPLOG)

mini-omp : $(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODEL)-$(CSTR)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODEL)-$(CSTR) $(CPPLOG); $(AUTOVEC_SUMMARY)\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODEL)-$(CSTR).log \
	./femera-$(CPUMODEL)-$(CSTR) -v2 -c$(NCPU) -p cube/unst19p1n16

mini-omq : $(QBJS) $(ODIR)/femera-mini.$(QEXT)
	echo $(CXX) ... -o femerq-$(CPUMODEL)-$(CSTR)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(QBJS) $(ODIR)/femera-mini.$(QEXT) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	-o femerq-$(CPUMODEL)-$(CSTR) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODEL)-$(CSTR).log \
	./femerq-$(CPUMODEL)-$(CSTR) -v1 -c$(NCPU) -p cube/unst19p1n16

base-omp : $(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femerb-$(CPUMODEL)-$(CSTR)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femerb-$(CPUMODEL)-$(CSTR) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODEL)-$(CSTR).log \
	./femerb-$(CPUMODEL)-$(CSTR) -v2 -c$(NCPU) -p cube/unst19p1n16

mini-hyb: $(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODEL)-hyb
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODEL)-hyb $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODEL)-$(CSTR).log \
	./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16

gmsh2fmr-ser : $(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT)
	echo $(CXX) ... -o gmsh2fmr-$(CPUMODEL)-$(CSTR)
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
	-rm $(CPUMODEL)-$(CSTR).err
	-rm $(CPUMODEL)-$(CSTR).log

cleaner :
	-rm $(ODIR)/*$(CPUMODEL)*;
	-rm *-$(CPUMODEL)-*;
