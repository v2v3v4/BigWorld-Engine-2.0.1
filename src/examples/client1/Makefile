
BIN  = client1
SRCS =										\
	main									\


ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/examples/$(BIN),,$(CURDIR))
endif

INSTALL_DIR = $(MF_ROOT)/bigworld/src/examples/$(BIN)

ASMS =

MY_LIBS = connection

USE_PYTHON =

USE_OPENSSL = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

