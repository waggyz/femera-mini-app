# Defaults: use g++
CPPFLAGS=-std=c++11 -Wall -Wextra -g -Ofast -ftree-vectorize -march=native \
 -mtune=native -fno-builtin-sin -fno-builtin-cos
#FIXME -mtune=core-avx2 when -mtune=native doesn't work
OMPFLAGS=-fopenmp
SERFLAGS=-Wno-unknown-pragmas
NCPU=2
NUMA=2

CPUMODEL:=$(shell ./cpumodel.sh)

# Check for intel compiler
ifdef INTEL_LICENSE_FILE
CXX=icc
CPPFLAGS=-std=c++11 -Wall -Wextra -Ofast -xHost -axSKYLAKE-AVX512 \
 -ffast-math -no-fast-transcendentals \
 -no-inline-max-size -no-inline-max-total-size -g
# -march=native
SERFLAGS=-fno-alias -diag-disable 3180
NCPU=8
NUMA=2
endif

# Check if using intel compiler on k3 or k4
ifdef INTEL_PYTHONHOME
CXX=icc
CPPFLAGS=-restrict -std=c++11 -Wall -Wextra -O2 -ansi-alias\
 -ffast-math -no-fast-transcendentals\
 -no-inline-max-size -no-inline-max-total-size -xSKYLAKE-AVX512 -g
SERFLAGS=-fno-alias -diag-disable 3180
NCPU=40
NUMA=6
endif

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections
# -Wsuggest-final-types and -Wsuggest-final-methods -O3 -flto

# CPPLOG=-fopt-info-vec-optimized 2>a.log ; grep -i vectorized a.log ; grep -i warn a.log; grep -i error a.log

FEMERA_MINI_CC = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc \
 tens-numa-halo-pcg.cc halo-pcr-dummy.cc \
 numa-elas-iso3.cc numa-elas-ort3.cc

#OMPI_CPPFLAGS=$(CPPFLAGS) ; \
#OMPI_CXXFLAGS=$(CPPFLAGS) ; \

all: gmsh2fmr-ser mini-omp mini-ser mini-omq mini-seq

mini-ser:
	mv -f femser-$(CPUMODEL) femser.old 2>/dev/null ; \
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) -DVERB_MAX=4 -DHAS_TEST \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femser-$(CPUMODEL) $(CPPLOG);\
	./femser-$(CPUMODEL) -v2 -p cube/unit1p1n2 ;\
	./femser-$(CPUMODEL) -v3 -p cube/unit1p2n2 ;

mini-omp:
	mv -f femera-$(CPUMODEL) femera.old 2>/dev/null ; \
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) -DOMP_SCHEDULE=static -DHAS_TEST \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femera-$(CPUMODEL) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread; \
	command /usr/bin/time -v ./femera-$(CPUMODEL) -v2 -c$(NCPU) -p cube/unst19p1n16 ;

mini-seq:
	mv -f femseq-$(CPUMODEL) femseq.old 2>/dev/null ; \
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) -DVERB_MAX=1 \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femseq-$(CPUMODEL) $(CPPLOG);\
	./femseq-$(CPUMODEL) -v1 -p cube/unit1p1n2 ;\
	./femseq-$(CPUMODEL) -v1 -p cube/unit1p2n2 ;

mini-omq:
	mv -f femerq-$(CPUMODEL) femerq.old 2>/dev/null ; \
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) -DVERB_MAX=1 \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femerq-$(CPUMODEL) $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread ;\
	command /usr/bin/time -v ./femerq-$(CPUMODEL) -v1 -c$(NCPU) -p cube/unst19p1n16 ;

mini-mpi:
	mv -f femera-mpi-$(CPUMODEL) femera.old 2>/dev/null ; \
	export OMPI_CXX=$(CXX) ; \
	export OMPI_CPPFLAGS=-std=c++11 ; \
	export OMPI_CXXFLAGS=-std=c++11 ; \
	mpicc -x c++ $(LDFLAGS) $(LDLIBS) -DHAS_TEST \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femera-mpi-$(CPUMODEL) $(CPPLOG);\
	command /usr/bin/time -v ./femera-mpi-$(CPUMODEL) -v2 -c$(NCPU) -p cube/unst19p1n16$(NCPU) ;

test-asc:
	mv -f a.out a.old 2>/dev/null ;\
	mv -f cube/unit1p2_1.cpy cube/unit1p2_1.old 2>/dev/null ;\
	./gmsh2fmr  -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu0.001 -p cube/unit1p1n2;
	cp cube/unit1p2_1.fmr cube/unit1p1n2_1.cpy ;\
	$(CXX) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	elem.cc elem-tet.cc phys.cc numa-elas-ort3.cc numa-elas-iso3.cc \
	test_asc.cc $(CPPLOG) ; ./a.out ;\
	diff cube/unit1p1n2_1.fmr cube/unit1p1n2_1.cpy ;

gmsh2fmr-ser:
	mv -f gmsh2fmr gmsh2fmr.old 2>/dev/null ;\
	$(CXX) $(SERFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) gmsh2.cc gmsh2fmr.cc -o gmsh2fmr ;\
	./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu 0.001 \
	-M1 -E100e9 -N0.3 -A20e-6 -K100e-6 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-v3 -ap cube/unit1p2n2;
	./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu 0.001 -t444 666 -Tu 100 \
	-M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R \
	-v3 -ap cube/unit1p2n2;

gmsh2fmr-rve:
	mv -f gmsh2fmr gmsh2fmr.old 2>/dev/null ;\
	$(CXX) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) gmsh2.cc gmsh2fmr.cc -o gmsh2fmr ;\
	./gmsh2fmr -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
	-M0 -E100e9 -N0.3 -R \
	-v3 -a neper/n5-id1;

gmsh2fmr-omp:
	mv -f gmsh2fmr gmsh2fmr.old 2>/dev/null ;\
	$(CXX) $(OMPFLAGS) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) gmsh2.cc gmsh2fmr.cc -o gmsh2fmr ;\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread ;\
	export OMP_NUM_THREADS=$(NCPU) ;\
	./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu 0.001 \
	-M1 -E100e9 -N0.3 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
	-v2 -ap cube/unit1p2n2;

test-head: femera.h test_h.cc
	$(CXX) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	test_h.cc ;

distclean: clean
	$(RM) femera-mini femera-knl gmsh2fmr

