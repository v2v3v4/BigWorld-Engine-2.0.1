# Check that MySQL development libraries have been installed.
HAS_MYSQL := $(shell ../../../build/test_mysql.sh && echo $$?)
ifneq ($(HAS_MYSQL),0)

$(warning MySQL development libraries not installed. Skipping autoload_clear build.)

else

IS_COMMAND = 1

BIN = clear_auto_load

SRCS = 	clear_auto_load											\
		main 													\

endif
# endif MySQL development libraries installed. 

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/$(BIN),,$(CURDIR))
endif

BUILD_TIME_FILE = main

CPPFLAGS += -I $(MF_ROOT)/bigworld/src/server

USE_PYTHON = 1

MY_LIBS = chunk dbmgr_mysql dbmgr_lib entitydef pyscript server
LDLIBS += `$(MYSQL_CONFIG_PATH) --libs_r`

include $(MF_ROOT)/bigworld/src/build/common.mak
