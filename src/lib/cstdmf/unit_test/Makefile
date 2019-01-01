LIB_NAME = cstdmf
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =										\
	main									\
	test_asserts							\
	test_bgtasks							\
	test_bw_util							\
	test_dogwatch							\
	test_static_array						\
	test_time_queue							\

MY_LIBS = cstdmf
LDFLAGS += -lpthread -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
