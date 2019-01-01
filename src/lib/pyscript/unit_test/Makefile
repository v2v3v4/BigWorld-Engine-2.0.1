LIB_NAME = pyscript
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =								\
	main							\
	integer_range_checker			\
	test_conversion					\

MY_LIBS = pyscript resmgr zip math cstdmf

USE_PYTHON = 1

LDFLAGS += -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
