#
# Makefile for baseappmgr
#

BIN = baseappmgr
SRCS = main								\
	baseapp								\
	baseappmgr							\
	baseappmgr_config					\
	message_handlers					\
	reply_handlers						\
	watcher_forwarding_baseapp			\
	../baseapp/baseapp_int_interface	\
	../loginapp/login_int_interface		\
	../dbmgr/db_interface				\
	../../common/doc_watcher			\

MY_LIBS = server
BUILD_TIME_FILE = main

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
