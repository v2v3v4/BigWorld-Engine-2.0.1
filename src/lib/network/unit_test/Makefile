LIB_NAME = network
LIB_PATH = /bigworld/src/lib/$(LIB_NAME)/unit_test

SRCS =								\
	main							\
	common_interface				\
	packet_generator				\
	test_auto_switch				\
	test_baseapp_death				\
	test_channel					\
	test_channel_version			\
	test_compresslength				\
	test_config						\
	test_flood						\
	test_fragment					\
	test_interface					\
	test_mangle						\
	test_netmask					\
	test_overflow					\
	test_receive_window				\
	test_stream						\
	test_threadsafety				\


MY_LIBS = network math cstdmf
LDLIBS += -lpthread -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst $(LIB_PATH),,$(PWD))
endif

include $(MF_ROOT)/bigworld/src/lib/unit_test_lib/unit_test.mak
