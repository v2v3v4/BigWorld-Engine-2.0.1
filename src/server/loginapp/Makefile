# Need to define the following variables
#
# BIN = Name of the executables to make
#		(e.g. BIN = cellmgr)
# SRCS = Name of the source files WITHOUT the extension
#		(e.g. cellmgr main)

BIN  = loginapp
SRCS =								\
	main							\
	database_reply_handler 			\
	loginapp 						\
	loginapp_config 				\
	message_handlers 				\
	nat_config		 				\
	status_check_watcher			\
	../dbmgr/db_interface			\
	../../common/doc_watcher		\

MY_LIBS = server connection
BUILD_TIME_FILE = main

# USE_PYTHON = 1

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
