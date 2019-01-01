#
# Need to define the following variables
#
# BIN = Name of the executables to make
#		(e.g. BIN = cellappmgr)
# SRCS = Name of the source files WITHOUT the extension
#		(e.g. cellappmgr main)

BIN  = reviver
SRCS = main											\
	component_reviver								\
	reviver											\
	reviver_config									\
	../baseappmgr/baseappmgr_interface				\
	../cellappmgr/cellappmgr_interface				\
	../dbmgr/db_interface							\

MY_LIBS = server
BUILD_TIME_FILE = main

# USE_PYTHON = 1

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
