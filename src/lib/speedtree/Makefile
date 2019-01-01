LIB = speedtree

SRCS =						\
	speedtree_collision		\
	speedtree_renderer		\
	speedtree_renderer_off	\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
