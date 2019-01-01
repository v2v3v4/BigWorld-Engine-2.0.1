LIB_NAME = entitydef
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =												\
	main											\
	../../pyscript/unit_test/integer_range_checker	\
	test_conversion									\

# TODO: entitydef library should not depend on network
# TODO: entitydef library should not depend on chunk
# TODO: entitydef library should not depend on physics2
# TODO: entitydef library should not depend on terrain
MY_LIBS = pyscript entitydef chunk terrain physics2 network resmgr zip math cstdmf

USE_PYTHON = 1

LDFLAGS += -rdynamic

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
