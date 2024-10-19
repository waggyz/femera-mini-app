#!/usr/bin/make

ifeq ("$(BUILT_BY)","")
  BUILT_BY := Anonymous User (Example University)<anon@example.edu>
  #(info $(INFO) set BUILT_BY to $(BUILT_BY))
endif
ifeq ("$(PREFIX)","")
  PREFIX := /usr/local
  #(info $(INFO) set PREFIX to $(PREFIX))
endif
ifeq ("$(HOSTNAME)","")
  HOSTNAME := $(shell hostname)
  #(info $(INFO) set HOSTNAME to $(HOSTNAME))
endif
ifeq ("$(CPUMODEL)","")
  CPUMODEL := $(shell tools/fmrmodel.sh)
  #(info $(INFO) set CPUMODEL to $(CPUMODEL))
endif
ifeq ("$(CPUCOUNT)","")
  CPUCOUNT := $(shell tools/fmrcores.sh)
  #(info $(INFO) set CPUCOUNT to $(CPUCOUNT))
endif
ifeq ("$(CPUNUMAS)","")
  CPUNUMAS := $(shell tools/fmrnumas.sh 2>/dev/null)
  #(info $(INFO) set CPUNUMAS to $(CPUNUMAS))
endif
ifeq ("$(CPUSIMDS)","")
  CPUSIMDS := $(shell tools/fmrsimds.sh)
  #(info $(INFO) set CPUSIMDS to $(CPUSIMDS))
endif
ifneq ("$(MAKEJOBS)","")
  ENABLE_MAKEJOBS:=ON
endif
ifeq ($(ENABLE_MAKEJOBS),ON)
  JSER:= -j1
  JPAR:= -j
  ifeq ("$(MAKEJOBS)","")
    ifeq ($(shell expr $(CPUCOUNT) \<= 8), 1)
      JLIM:= -j`echo "2*$(CPUCOUNT)" | bc`
      JEXT:= -j$(CPUCOUNT)
    else
      JLIM:= -j$(CPUCOUNT)
      JEXT:= -j8
    endif
  else
    JLIM:= -j$(MAKEJOBS)
    JEXT:= -j$(MAKEJOBS)
  endif
endif
