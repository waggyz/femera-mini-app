NCPU:=$(shell ./cpucount.sh)
CPUMODEL:=$(shell ./cpumodel.sh)
CPUSIMD:=$(shell ./cpusimd.sh)
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

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections
# -Wsuggest-final-types -Wsuggest-final-methods -O3 -flto
NUMA=6
endif

CPUMODELC:=$(CPUMODEL)-$(CSTR)

FEMERA_COMMON = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc\
 halo-pcg-omp.cc halo-ncg-omp.cc halo-pcr-dummy.cc \
 elas-iso3.cc elas-ort3.cc elas-ther-ort3.cc


FEMERA_BASE_C = $(FEMERA_COMMON)\
 elas-iso3-base.cc elas-ort3-bas2.cc elas-ther-ort3-bas2.cc

# ifeq ($(HOST2CHAR), k2)
ifneq (,$(findstring AVX,$(CPUSIMD)))
FEMERA_MINI_C = $(FEMERA_COMMON)\
 elas-iso3-vect.cc elas-ort3-vec2.cc elas-ther-ort3-vec2.cc
else
FEMERA_MINI_C = $(FEMERA_BASE_C)
endif
 
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
BBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_BASE_C:.$(CEXT)=.$(OEXT)))
QBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(QEXT)))
SBJS:= $(patsubst %,$(ODIR)/%,$(FEMERA_MINI_C:.$(CEXT)=.$(SEXT)))
GBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_GCC_C:.$(CEXT)=.$(GEXT)))
IBJS:= $(patsubst %,$(ODIR)/%,$(HYBRID_ICC_C:.$(CEXT)=.$(IEXT)))

TESTDIR = test/$(CPUMODEL)
PERFDIR = perf

_dummy := $(shell mkdir -p mini.o test $(TESTDIR) $(PERFDIR))

.SILENT :

all : gmsh2fmr-ser mini-omp mini-omq

$(ODIR)/%.$(OEXT) : %.cc *.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DHAS_TEST \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(QEXT) : %.cc *.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	$< -o $@ $(CPPLOG)

$(ODIR)/%.$(SEXT) : %.cc *.h
	echo $(CXX) ... -o $@
	$(CXX) -c $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	-DFETCH_JAC \
	$< -o $@ $(CPPLOG)

mini-omp : test-scripts femera-$(CPUMODELC)

mini-omq : test-scripts femerq-$(CPUMODELC)

base-omp : test-scripts femerb-$(CPUMODELC)

mini-hyb : test-scripts femera-$(CPUMODEL)-hyb

gmsh2fmr-ser : gmsh2fmr-$(CPUMODELC) test-scripts

femera-$(CPUMODELC) : $(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(OBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODELC) $(CPPLOG); $(AUTOVEC_SUMMARY)
	echo ./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

femerq-$(CPUMODELC) : $(QBJS) $(ODIR)/femera-mini.$(QEXT)
	echo $(CXX) ... -o femerq-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(QBJS) $(ODIR)/femera-mini.$(QEXT) \
	-DOMP_SCHEDULE=static -DFETCH_JAC -DVERB_MAX=1 \
	-o femerq-$(CPUMODELC) $(CPPLOG);
	echo ./femerq-$(CPUMODELC) -v1 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femerq-$(CPUMODELC) -v1 -c$(NCPU) -p cube/unst19p1n16

femerb-$(CPUMODELC) : $(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femerb-$(CPUMODELC)
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(BBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femerb-$(CPUMODELC) $(CPPLOG);
	echo ./femerb-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femerb-$(CPUMODELC) -v2 -c$(NCPU) -p cube/unst19p1n16

femera-$(CPUMODEL)-hyb : $(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT)
	echo $(CXX) ... -o femera-$(CPUMODEL)-hyb
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(GBJS) $(IBJS) $(ODIR)/test.$(OEXT) $(ODIR)/femera-mini.$(OEXT) \
	-DOMP_SCHEDULE=static -DHAS_TEST -DFETCH_JAC \
	-o femera-$(CPUMODEL)-hyb $(CPPLOG);
	echo ./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v --append -o $(CPUMODELC).log \
	./femera-$(CPUMODEL)-hyb -v2 -c$(NCPU) -p cube/unst19p1n16

gmsh2fmr-$(CPUMODELC) : $(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT)
	echo $(CXX) ... -o gmsh2fmr-$(CPUMODELC)
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(SBJS) $(ODIR)/gmsh2.$(SEXT) $(ODIR)/gmsh2fmr.$(SEXT) \
	-o gmsh2fmr-$(CPUMODELC) ;
	./gmsh2fmr-$(CPUMODELC) -t666 -x0 -t111 -z0 -t333 -y0 -t444 -xu 0.001 \
	-M1 -E100e9 -N0.3 -A20e-6 -K100e-6 -Z1 -X0 -Z0 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-v3 -ap cube/unit1p2n2;
	./gmsh2fmr-$(CPUMODELC) -t666 -x0 -t333 -y0 -t111 -z0 -t444 -xu 0.001 -t444 -Tu 10 \
	-M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R \
	-v3 -ap cube/unit1p2n2;

profile : profile-basic profile-small profile-large

profile-basic : mini-omq gmsh2fmr-ser
	$(PERFDIR)/profile-basic.sh > $(PERFDIR)/profile-basic-$(CPUMODELC).log

profile-small : mini-omq gmsh2fmr-ser
	$(PERFDIR)/profile-small.sh > $(PERFDIR)/profile-small-$(CPUMODELC).log

profile-large : mini-omq gmsh2fmr-ser
	$(PERFDIR)/profile-large.sh > $(PERFDIR)/profile-large-$(CPUMODELC).log

unit-test : test-scripts test-gmsh

test-scripts : $(TESTDIR)/cpucount.sh.err $(TESTDIR)/cpumodel.sh.err \
$(TESTDIR)/cpusimd.sh.err

$(TESTDIR)/%.sh.err : %.sh unit-test/%.sh.chk
	unit-test/$<.chk > $(TESTDIR)/$<.err ;
	unit-test/print-test-results.sh "$<" "$(TESTDIR)/$<.err"

test-gmsh : $(TESTDIR)/test-gmsh.err

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

cleaner : clean clean-test

cleanest : cleaner clean-perf
	-rm -r $(PERFDIR)/*-$(CPUMODELC)*
