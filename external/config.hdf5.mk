#!/usr/bin/make
ifeq ($(ENABLE_HDF5),ON)
  FMRFLAGS += -DFMR_HAS_HDF5
  ifeq ($(ENABLE_MPI),ON)
    EXTERNAL_DOT+="MPI" -> "HDF5"\n
  endif
  LIST_EXTERNAL += hdf5
  HDF5_FLAGS += -DCMAKE_INSTALL_PREFIX="$(INSTALL_CPU)"
  HDF5_FLAGS += -DHDF5_BUILD_CPP_LIB:BOOL=OFF
  HDF5_FLAGS += -DHDF5_ENABLE_PARALLEL:BOOL=ON
  HDF5_FLAGS += -DMPIEXEC_MAX_NUMPROCS:STRING=4
  HDF5_FLAGS += -DBUILD_STATIC_LIBS:BOOL=ON
  HDF5_FLAGS += -DBUILD_SHARED_LIBS:BOOL=ON
  HDF5_FLAGS += -DCTEST_BUILD_CONFIGURATION=Release
  HDF5_FLAGS += -DHDF5_NO_PACKAGES:BOOL=ON
  HDF5_FLAGFILE:=$(BUILD_CPU)/external/install-hdf5.flags
endif
ifeq ($(ENABLE_HDF5),ON)

.PHONY: $(HDF5_FLAGFILE).new

external-flags: $(HDF5_FLAGFILE).new

$(HDF5_FLAGFILE).new: external/config.hdf5.mk
	printf "%s" '$(HDF5_FLAGS)' > $(@)

endif
