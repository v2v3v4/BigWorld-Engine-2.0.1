BIN  = client4

SRCS =										\
	main									\
	entity									\
	entity_type								\
	py_server								\
	py_entities								\
	../../common/simple_client_entity		\


ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/examples/$(BIN),,$(CURDIR))
endif

INSTALL_DIR = $(MF_ROOT)/bigworld/src/examples/client4

ASMS =

MY_LIBS = entitydef pyscript chunk connection

USE_PYTHON = 1

USE_OPENSSL = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

