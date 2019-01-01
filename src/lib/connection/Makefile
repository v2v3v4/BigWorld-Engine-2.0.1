LIB = 	connection

SRCS = 														\
	baseapp_login_request									\
	client_interface										\
	data_download											\
	log_on_params											\
	login_handler											\
	login_interface											\
	loginapp_login_request									\
	retrying_request										\
	server_connection										\

	

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

USE_OPENSSL = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

