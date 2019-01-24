# Defaults: use g++
CPPFLAGS=-std=c++11 -Wall -Wextra -g -Ofast -ftree-vectorize -march=native \
 -mtune=native -fno-builtin-sin -fno-builtin-cos -Wno-unknown-pragmas
#FIXME -mtune=core-avx2 when -mtune=native doesn't work
NCPU=2
NUMA=2

CPUMODEL:=$(shell ./cpumodel.sh)

# Check for intel compiler
ifdef INTEL_LICENSE_FILE
CXX=icc
CPPFLAGS=-std=c++11 -Wall -Wextra -Ofast -xHost -axSKYLAKE-AVX512 \
 -ffast-math -no-fast-transcendentals -fno-alias -diag-disable 3180 \
 -qopt-zmm-usage=high -no-inline-max-size -no-inline-max-total-size -g
# -march=native
NCPU=8
NUMA=2
endif

# Check if using intel compiler on k3 or k4
ifdef INTEL_PYTHONHOME
CXX=icc
CPPFLAGS=-std=c++11 -Wall -Wextra -Ofast -xHost -axSKYLAKE-AVX512 \
 -ffast-math -no-fast-transcendentals -fno-alias -diag-disable 3180 \
 -qopt-zmm-usage=high -no-inline-max-size -no-inline-max-total-size -g
NCPU=40
NUMA=6
endif

#-O3 -ftree-vectorize -ffast-math -march=native
# -funsafe-loop-optimizations
# -fopt-info-vec-optimized -missed -all
# -ffunction-sections -fdata-sections -W1, --gc-sections
# -Wsuggest-final-types and -Wsuggest-final-methods -O3 -flto

# CPPLOG=-fopt-info-vec-optimized 2>a.log ; grep -i vectorized a.log ; grep -i warn a.log; grep -i error a.log

# FEMERA_MINI_CC = mesh.cc elem.cc elem-tet.cc \
# phys.cc phys-elastortho3d.cc solv-pcg-halo.cc

FEMERA_MINI_CC = mesh.cc elem.cc phys.cc solv.cc elem-tet.cc \
 tens-numa-halo-pcg.cc tens-numa-halo-pcr.cc numa-elas-iso3.cc numa-elas-ort3.cc

#OMPI_CPPFLAGS=$(CPPFLAGS) ; \
#OMPI_CXXFLAGS=$(CPPFLAGS) ; \

all: test-all

mini-ser:
	mv -f femser-$(CPUMODEL) femser.old 2>/dev/null ; \
	$(CXX) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femser-$(CPUMODEL) $(CPPLOG) ;\
	./femser-$(CPUMODEL) -v 2 -p cube/unit1p1n2 ;

mini-omp:
	mv -f femera-$(CPUMODEL) femera.old 2>/dev/null ; \
	$(CXX) -fopenmp $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femera-$(CPUMODEL) $(CPPLOG) ;\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread ;\
	command /usr/bin/time -v ./femera-$(CPUMODEL) -v2 -c$(NCPU) -p cube/unst19p1n16 ;

mini-mpi:
	mv -f femera-mpi-$(CPUMODEL) femera.old 2>/dev/null ; \
	export OMPI_CXX=$(CXX) ; \
	export OMPI_CPPFLAGS=-std=c++11 ; \
	export OMPI_CXXFLAGS=-std=c++11 ; \
	mpicc -x c++ $(LDFLAGS) $(LDLIBS) \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femera-mpi-$(CPUMODEL) $(CPPLOG) ;\
	command /usr/bin/time -v ./femera-mpi-$(CPUMODEL) -v2 -c$(NCPU) -p cube/unst19p1n16$(NCPU) ;

knl-mini:
	mv -f femera-knl femera.old 2>/dev/null ; \
	$(CXX) -fopenmp $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) test.cc femera-mini.cc -o femera-knl $(CPPLOG);\
	export OMP_PLACES=cores; export OMP_PROC_BIND=spread ;\
	command /usr/bin/time -v ./femera-knl -v2 -c$(NCPU) -p cube/unst19p1n16 ;

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
	$(CXX) $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
	$(FEMERA_MINI_CC) gmsh2.cc gmsh2fmr.cc -o gmsh2fmr ;\
	./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu 0.001 \
	-M1 -E100e9 -N0.3 \
	-M2 -E100e9 -N0.3 -Z1 -X0 -Z0 \
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
	$(CXX) -fopenmp $(LDFLAGS) $(LDLIBS) $(CPPFLAGS) \
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

