LIB =	dbmgr_lib

SRCS += 							\
			billing_system			\
			db_entitydefs			\
			db_status				\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

