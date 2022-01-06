#!/usr/bin/make
# Labels needs extra spacing.
# Make action labels
SPCS:=$(shell echo "       ")
WRAP:=$(shell echo "  wrap ")
CXX_:=$(shell echo "  C++  ")
CC__:=$(shell echo "   C   ")
LINK:=$(shell echo "  link ")
LIBS:=$(shell echo "  lib  ")
GOGO:=$(shell echo "   GO  ")
DONE:=$(shell echo "  DONE ")
FLAG:=$(shell echo "  flag ")
TIME:=$(shell echo "  time ")
COPY:=$(shell echo "   \(C\) ")
# Standard labels
ifeq (1,1)#TODO Check for ANSI support
  NORM_COLOR:=\e[0m
  GOOD_COLOR:=\e[30m\e[42m
  HINT_COLOR:=\e[35m
  PASS_COLOR:=\e[32m
  INFO_COLOR:=\e[94m
  NOTE_COLOR:=\e[92m
  ToDo_COLOR:=\e[96m
  TEST_COLOR:=\e[35m
  # WARN_COLOR:=\e[93m\e[1m
  WARN_COLOR:=\e[07m
  # FAIL_COLOR:=\e[91m\e[1m
  # FAIL_COLOR:=\e[91m\e[1m\e[7m
  FAIL_COLOR:=\e[30m\e[41m
  DBUG_COLOR:=\e[30m\e[35m\e[07m
  STOP_COLOR:=\e[37;1m\e[41m
  #
  GOOD:=$(shell printf " $(GOOD_COLOR) GOOD $(NORM_COLOR)")
  PASS:=$(shell printf " $(PASS_COLOR)  ok  $(NORM_COLOR)")
  INFO:=$(shell printf " $(INFO_COLOR) info $(NORM_COLOR)")
  NOTE:=$(shell printf " $(NOTE_COLOR) NOTE $(NORM_COLOR)")
  HINT:=$(shell printf " $(HINT_COLOR) HINT $(NORM_COLOR)")
  MORE:=$(shell printf " $(HINT_COLOR) see: $(NORM_COLOR)")
  ToDo:=$(shell printf " $(ToDo_COLOR) ToDo $(NORM_COLOR)")
  TEST:=$(shell printf " $(TEST_COLOR) test $(NORM_COLOR)")
  WARN:=$(shell printf " $(WARN_COLOR) WARN $(NORM_COLOR)")
  FAIL:=$(shell printf " $(FAIL_COLOR) FAIL $(NORM_COLOR)")
  DBUG:=$(shell printf " $(DBUG_COLOR) DBUG $(NORM_COLOR)")
  DEBUG:=$(DBUG)
  STOP:=$(shell printf " $(STOP_COLOR) STOP $(NORM_COLOR)")
  Make:=$(shell printf " $(INFO_COLOR) make $(NORM_COLOR)")
  EXEC:=$(shell printf " $(PASS_COLOR) exec $(NORM_COLOR)")
  E_G_:=$(shell printf " $(NOTE_COLOR) e.g. $(NORM_COLOR)")
  GRND:=$(shell printf " $(INFO_COLOR)grind $(NORM_COLOR)")
else
  GOOD:=$(shell echo "  GOOD ")
  PASS:=$(shell echo "   ok  ")
  INFO:=$(shell echo "  info ")
  NOTE:=$(shell echo "  NOTE ")
  HINT:=$(shell echo "  HINT ")
  MORE:=$(shell echo "  see: ")
  ToDo:=$(shell echo "  ToDo ")
  TEST:=$(shell echo "  test ")
  WARN:=$(shell echo "  WARN ")
  FAIL:=$(shell echo "  FAIL ")
  DBUG:=$(shell echo "  DBUG ")
  STOP:=$(shell echo "  STOP ")
  Make:=$(shell echo "  make ")
  EXEC:=$(shell echo "  exec ")
  E_G_:=$(shell echo "  e.g. ")
  GRND:=$(shell echo " grind ")
endif
override ToDo := $(shell echo "$(ToDo)" | tr [a-z] [A-Z])
