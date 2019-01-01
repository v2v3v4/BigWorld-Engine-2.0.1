# Check that MySQL development libraries have been installed.
HAS_MYSQL := $(shell ../../../build/test_mysql.sh && echo $$?)
ifneq ($(HAS_MYSQL),0)

$(warning MySQL development libraries not installed. Skipping sync_db build.)

else

IS_COMMAND = 1
BIN = sync_db
SRCS = main 															\
	mysql_synchronise													\
	mysql_table_initialiser												\
	mysql_upgrade_database												\

endif
# endif MySQL development libraries installed. 

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/$(BIN),,$(CURDIR))
endif

CPPFLAGS += -I $(MF_ROOT)/bigworld/src/server

USE_PYTHON = 1

BUILD_TIME_FILE = main

MY_LIBS =  chunk dbmgr_mysql dbmgr_lib entitydef pyscript server
LDLIBS += `$(MYSQL_CONFIG_PATH) --libs_r`

BUILD_TIME_FILE = main

include $(MF_ROOT)/bigworld/src/build/common.mak
