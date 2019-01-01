#
# Makefile for reviver unit tests
#

BIN = reviver
BIN_PATH = /bigworld/src/server/$(BIN)/unit_test

SRCS = main									\

ifndef MF_ROOT
export MF_ROOT := $(subst $(BIN_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
