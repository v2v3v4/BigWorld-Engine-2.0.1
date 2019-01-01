
BIN = bwmachined2
SRCS = main linux_machine_guard cluster bwmachined listeners usermap

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/bwmachined,,$(CURDIR))
endif

INSTALL_DIR = $(MF_ROOT)/bigworld/tools/server/bin/$(MF_CONFIG)

NO_EXTRA_LIBS = 1
MY_LIBS = cstdmf server network

include $(MF_ROOT)/bigworld/src/build/common.mak
