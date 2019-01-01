SO  = example
COMPONENT = cellapp

SRCS =						\
	debug_extension			\
	egextra					\
	egcontroller			\
	speed_extra				\
	speed_controller		\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/examples/cellapp_extension,,$(CURDIR))
endif


CPPFLAGS += -I../../server

USE_PYTHON=1

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

