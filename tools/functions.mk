#!/usr/bin/make
SHELL := bash
-include tools/labels.mk

SPC40:=$(shell echo "                                        ")

# These keep the time persistent through recursive makefile calls.
BUILD_DATE := ${BUILD_DATE}
BUILD_SECS := ${BUILD_SECS}
ifndef BUILD_DATE
  export BUILD_DATE := $(shell date -u +"%Y-%m-%dT%H:%M")
  export BUILD_SECS := $(shell date +%s)
endif

parse_email = $(shell echo '$(1)' | sed 's/ *(.*)//; s/>.*//; s/.*[:<] *//')

strcut = $(shell echo "$(1)" | cut -c-$(2) )

make_jobs_str = $(shell if [[ "$(MAKEFLAGS)" == *"j"* ]]; \
  then printf 'p';else printf 's'; fi)
#  then printf '\u2225';else printf '\u2226'; fi)

timestamp = @printf '%8s%-45s %1s%25s\n' "$(Make) " \
'$(call strcut,$(1): $(2)$(SPC40)$(SPC40),44)' $(call make_jobs_str) \
$(BUILD_DATE)"+"$(shell tools/elapsed-time.sh $(BUILD_SECS))

elapstamp = @printf '%8s%-62s %9s\n' "$(Make) " \
'$(call strcut,$(1): $(2)$(SPC40)$(SPC40),61)' \
"+"$(shell tools/elapsed-time.sh $(BUILD_SECS))

# 80-8 = 72, (80-8)/2 =41
col2cxx = printf "%s %-47s%25s\n" "$(1)" "$(2)" "$(3)";
col2lib = printf "%s %-30s%42s\n" "$(1)" "$(2)" "$(3)";

ifeq (0,1)
label_test = touch $(4).out;    \
 $(3) >$(4).out 2>$(4).err;     \
  if [[ "$$?" -eq 0 ]];         \
    then printf "$(1) %-.70s\n" "$(3)"; \
    else printf "$(2) $(3)\n";  \
      if [ -f $(4).out ]; then  \
        tail $(4).out >&2;      \
        mv -f $(4).out $(4).bad;\
    fi;                         \
  fi;                           \
  if [ -f $(4).err ]; then      \
    tail $(4).err >&2;          \
  fi;
else
label_test = -tools/label-test.sh "$(1)" "$(2)" "$(3)" "$(4)"
endif



# label_test = touch $(strip $(4)).out; \
# $(strip $(3)) >>$(strip $(4)).out 2>>$(strip $(4)).err;   \
#  if [[ "$$?" -eq 0 ]];                              \
#    then printf "$(1) $(strip $(3))\n";           \
#    else printf "$(2) $(strip $(3))\n";                       \
#      if [ -f $(4).out ]; then cat $(4).out >&2; fi; \
#  fi;                                                \
#  if [ -f $(4).err ]; then cat $(4).err >&2; fi;

#NOTE redirect filename cannot match target?
#NOTE tee does not seem to work with Bats

# log_test = "$(1)" 1>> "$(2).out" 2>> "$(2).err"; cat "$(2).err" >&2
# log_test = "$(1)" 1>> "$(2).out" 2> >(tee -a "$(2).err" >&2)
