# Check that MySQL development libraries have been installed.
HAS_MYSQL := $(shell ../../../build/test_mysql.sh && echo $$?)
ifneq ($(HAS_MYSQL),0)

$(warning MySQL development libraries not installed. Skipping consolidate_dbs build.)

else

IS_COMMAND = 1
BIN = consolidate_dbs
SRCS =												\
	command_line_parser								\
	consolidate_dbs_app								\
	consolidate_entity_task							\
	consolidation_progress_reporter					\
	dbmgr											\
	db_file_transfer_error_monitor					\
	file_receiver									\
	file_receiver_mgr								\
	file_transfer_progress_reporter					\
	msg_receiver									\
	main											\
	primary_database_update_queue					\
	secondary_database								\
	secondary_database_table						\
	transfer_db_process								\

endif
# endif MySQL development libraries installed. 

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/$(BIN),,$(CURDIR))
endif

CPPFLAGS += -I $(MF_ROOT)/bigworld/src/server

USE_PYTHON = 1
USE_MYSQL_PREPARED_STATEMENTS=0

BUILD_TIME_FILE = main

ifeq ($(USE_MYSQL_PREPARED_STATEMENTS), 1)
	CPPFLAGS += -DUSE_MYSQL_PREPARED_STATEMENTS
endif

MY_LIBS = chunk dbmgr_mysql dbmgr_lib entitydef pyscript sqlite server 

LDLIBS += `$(MYSQL_CONFIG_PATH) --libs_r`

include $(MF_ROOT)/bigworld/src/build/common.mak
