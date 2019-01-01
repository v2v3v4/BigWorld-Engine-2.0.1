#
# Makefile for baseappmgr unit tests
#

BIN = baseappmgr
BIN_PATH = /bigworld/src/server/$(BIN)/unit_test

SRCS = main									\

LDLIBS += -ldl

ifndef MF_ROOT
export MF_ROOT := $(subst $(BIN_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
