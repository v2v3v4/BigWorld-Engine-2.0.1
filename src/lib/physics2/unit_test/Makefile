LIB_NAME = physics2
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =								\
	main							\
	pch								\
	test_bsptree					\
	test_worldtri

MY_LIBS = physics2 cstdmf math resmgr

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
