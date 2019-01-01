LIB_NAME = math
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =								\
	main							\
	test_boundingbox				\
	test_ema						\
	test_matrix						\
	test_quaternion					\
	test_vector3					\
	test_math_extra

# Currently Windows only
#	test_blend_transform			\

MY_LIBS = math cstdmf

LDFLAGS += -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
