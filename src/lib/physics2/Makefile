LIB = physics2

SRCS =				\
	bsp				\
	hulltree		\
	material_kinds	\
	quad_tree		\
	worldpoly		\
	worldtri

LDFLAGS += -rdynamic

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
