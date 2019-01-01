LIB =	dbmgr_xml

SRCS +=							\
	xml_billing_system			\
	xml_database

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

