#!/usr/bin/make
ifeq ($(ENABLE_MKL),ON)
  FMRFLAGS += -DFMR_HAS_MKL
  LIST_EXTERNAL += mkl
  EXTERNAL_DOT+="Femera" -> "MKL" [color="green"]\n
  MKL_FLAGS += --extract-folder $(FMRDIR)/external/mkl
  MKL_FLAGS += --remove-extracted-files no
  MKL_FLAGS += -a --silent --eula accept --install-dir $(INSTALL_CPU)
  # https://www.intel.com/content/www/us/en/develop/documentation
  # /installation-guide-for-intel-oneapi-toolkits-linux/top/installation
  # /install-with-command-line.html#install-with-command-line
  # ./install.sh --silent --action remove # To remove MKL
  MKL_FLAGFILE:=$(BUILD_CPU)/external/mkl-install.flags
endif
ifeq ($(ENABLE_MKL),ON)

external-flags: $(MKL_FLAGFILE).new

.PHONY: $(MKL_FLAGFILE).new

$(MKL_FLAGFILE).new: external/mkl-config.mk
	printf "%s" '$(MKL_FLAGS)' > $(@)

endif
