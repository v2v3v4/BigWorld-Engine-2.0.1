BIN  = client3

SRCS =										\
	../client2/main							\
	../client2/entity						\
	../client2/entity_type					\
	main_loop_action						\
	../../common/simple_client_entity		\


ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/examples/$(BIN),,$(CURDIR))
endif

INSTALL_DIR = $(MF_ROOT)/bigworld/src/examples/$(BIN)

ASMS =

MY_LIBS = entitydef pyscript chunk connection

USE_PYTHON = 1

USE_OPENSSL = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

