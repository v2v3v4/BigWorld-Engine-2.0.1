LIB_NAME = unit_test
LIB_PATH = /src/lib/$(LIB_NAME)/unit_test

SRCS =										\
	main									\
	test_multi_proc_test_case				\

MY_LIBS = cstdmf

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/src/lib/unit_test_lib/unit_test.mak
