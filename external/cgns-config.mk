#!/usr/bin/make
#FIXME CGNS, FLTK, and FreeType build in external/*/
ifeq ($(ENABLE_CGNS),ON)
  FMRFLAGS += -DFMR_HAS_CGNS
  LDLIBS += -lcgns
  LIST_EXTERNAL += cgns
  EXTERNAL_DOT+="Femera" -> "CGNS"\n
  ifeq ($(ENABLE_HDF5),ON)
    CGNS_REQUIRES += hdf5
    EXTERNAL_DOT+="CGNS" -> "HDF5"\n
    CGNS_FLAGS += -DCGNS_ENABLE_HDF5:BOOL=ON
  else
    CGNS_FLAGS += -DCGNS_ENABLE_HDF5:BOOL=OFF
  endif
  CGNS_FLAGS += -DCMAKE_INSTALL_PREFIX=$(INSTALL_CPU)
  CGNS_FLAGS += -DCGNS_ENABLE_LFS:BOOL=ON
  CGNS_FLAGS += -DCGNS_ENABLE_64BIT:BOOL=ON
  CGNS_FLAGS += -DCGNS_ENABLE_LEGACY:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_SCOPING:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_BASE_SCOPE:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_TESTS:BOOL=OFF
  CGNS_FLAGS += -DCGNS_ENABLE_FORTRAN:BOOL=OFF
  CGNS_DEPS:=$(patsubst %,$(BUILD_CPU)/external/%-install.out,$(CGNS_REQUIRES))
  CGNS_FLAGFILE:=$(BUILD_CPU)/external/cgns-install.flags
endif
ifeq ($(ENABLE_CGNS),ON)

.PHONY: $(CGNS_FLAGFILE).new

external-flags: $(CGNS_FLAGFILE).new

$(BUILD_CPU)/external/cgns-install.out: | $(CGNS_DEPS)

$(CGNS_FLAGFILE).new: external/cgns-config.mk
	printf "%s" '$(CGNS_FLAGS)' > $(@)

endif
