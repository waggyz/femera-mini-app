#!/usr/bin/make
ifeq($(ENABLE_NVIDIA),ON)
  FMRFLAGS += -DFMR_HAS_NVIDIA
  LDLIBS += -lcuda -lcudart
  ifeq ("$(findstring $(NVIDIA_LIBDIR):,$(LDPATH):)","")
    ADD_TO_LDPATH:= $(ADD_TO_LDPATH):$(NVIDIA_LIBDIR)
  endif
  # CCUFLAGS+= -I$(NVIDIA_INCDIR)
  # LDFLAGS+= -L$(NVIDIA_LIBDIR)
  # LDLIBS+= -lcublas
  CCU:=nvcc
endif
