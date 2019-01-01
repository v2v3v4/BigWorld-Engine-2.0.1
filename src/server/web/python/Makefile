# 
# Makefile for Web Integration Module.
#

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/web/python,,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/architecture.mak

MF_REAL_CONFIG := $(MF_CONFIG)

ifeq (,$(findstring SingleThreaded,$(MF_CONFIG)))
override MF_CONFIG := $(MF_CONFIG)_SingleThreaded
endif

export MF_CONFIG

SO = BigWorld
OUTPUTDIR = $(MF_ROOT)/bigworld/bin/web/$(MF_REAL_CONFIG)

BUILD_TIME_FILE = module_init

SRCS =	backup_hash_chain_request_handler			\
		blocking_db_reply_handler					\
		blocking_db_logon_handler					\
		blocking_response_handler					\
		functions									\
		mailbox										\
		retrying_remote_method						\
		web_integration								\
		../../baseapp/baseapp_int_interface			\
		../../baseappmgr/baseappmgr_interface		\
		../../dbmgr/db_interface					\


USE_PYTHON = 1
USE_OPENSSL = 1

MY_LIBS = connection chunk server entitydef pyscript resmgr network zip math cstdmf
MAKE_LIBS=1

include $(MF_ROOT)/bigworld/src/build/common.mak
