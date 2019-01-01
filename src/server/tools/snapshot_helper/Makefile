# Check that MySQL development libraries have been installed.
HAS_MYSQL := $(shell ../../../build/test_mysql.sh && echo $$?)
ifneq ($(HAS_MYSQL),0)

$(warning MySQL development libraries not installed. Skipping snapshot_helper build.)

else

BIN  = snapshot_helper
SRCS = snapshot_helper

LDLIBS += `$(MYSQL_CONFIG_PATH) --libs_r`

endif
# endif MySQL development libraries installed. 

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/snapshot_helper,,$(CURDIR))
endif

INSTALL_DIR = $(MF_ROOT)/bigworld/tools/server/bin/$(MF_CONFIG)

MY_LIBS = server

include $(MF_ROOT)/bigworld/src/build/common.mak
