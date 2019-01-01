USE_MYSQL=0
USE_XML=1
BUILD_TIME_FILE = main

USE_CUSTOM_BILLING_SYSTEM=0

BIN = dbmgr
SRCS =															\
	main 														\
	consolidator												\
	custom														\
	custom_billing_system										\
	database													\
	delete_entity_handler										\
	dbmgr_config												\
	entity_auto_loader											\
	get_entity_handler											\
	load_entity_handler											\
	login_app_check_status_reply_handler						\
	login_handler												\
	look_up_dbid_handler										\
	look_up_entity_handler										\
	message_handlers											\
	py_billing_system											\
	relogon_attempt_handler										\
	write_entity_handler										\
	../baseappmgr/baseappmgr_interface							\
	../baseapp/baseapp_int_interface							\
	../../common/py_network										\
	../../common/py_server										\
	../../common/doc_watcher									\

MY_LIBS =

ifeq ($(USE_CUSTOM_BILLING_SYSTEM), 1)
CPPFLAGS += -DUSE_CUSTOM_BILLING_SYSTEM
endif

ifeq ($(USE_MYSQL), 1)
HAS_MYSQL := $(shell ../../build/test_mysql.sh && echo $$?)
ifneq ($(HAS_MYSQL),0)
$(warning MySQL development libraries not installed. Disabling USE_MYSQL.)
USE_MYSQL=0
endif
endif

ifeq ($(USE_MYSQL), 1)
MY_LIBS += dbmgr_mysql

LDLIBS += `$(MYSQL_CONFIG_PATH) --libs_r`
CPPFLAGS += -DUSE_MYSQL


# When building with MySQL support, make sure we also attempt to build the
# sync_db and consolidate_dbs binaries.
ifeq ($(MAKELEVEL), 0)

all clean::
	$(MAKE) -C ../tools/sync_db $@
	$(MAKE) -C ../tools/consolidate_dbs $@
	$(MAKE) -C ../tools/clear_auto_load $@

endif

endif # USE_MYSQL

ifeq ($(USE_XML), 1)
MY_LIBS += dbmgr_xml
CPPFLAGS += -DUSE_XML
endif

MY_LIBS += dbmgr_lib server chunk entitydef pyscript connection

CPPFLAGS += -I $(CURDIR)

USE_PYTHON = 1

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
