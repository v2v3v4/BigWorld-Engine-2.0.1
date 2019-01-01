LIB_NAME = terrain
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =								\
	main							\
	test_resource					\

# Currently Windows only
#	test_vertex_lod_manager			\

MY_LIBS = cstdmf terrain

LDFLAGS += -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
