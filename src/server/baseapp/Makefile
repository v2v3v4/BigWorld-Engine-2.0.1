#
# Makefile for baseapp
#

BIN  = baseapp
SRCS = main								\
	archiver							\
	backed_up_base_app					\
	backed_up_base_apps					\
	backup_sender						\
	base								\
	base_message_forwarder				\
	bases								\
	baseapp								\
	baseapp_config						\
	baseappmgr_gateway					\
	bwtracer							\
	client_entity_mailbox				\
	controlled_shutdown_handler			\
	create_cell_entity_handler			\
	data_downloads						\
	dead_cell_apps						\
	download_streamer					\
	download_streamer_config			\
	eg_tcpecho							\
	entity_channel_finder				\
	entity_creator						\
	entity_type							\
	external_interfaces					\
	global_bases						\
	id_config							\
	load_entity_handler					\
	loading_thread						\
	login_handler						\
	mailbox								\
	message_handlers					\
	offloaded_backups					\
	pending_logins						\
	ping_manager						\
	proxy								\
	proxy_buffered_message				\
	proxy_rate_limit_callback			\
	py_bases							\
	py_cell_data						\
	rate_limit_config					\
	rate_limit_message_filter			\
	script_bigworld						\
	secondary_db_config					\
	shared_data_manager					\
	sqlite_database						\
	worker_thread						\
	write_to_db_reply					\
	../../common/db_interface_utils		\
	../../common/doc_watcher			\
	../../common/py_network				\
	../../common/py_server				\

# chunk is needed by entitydef because of UDOs
MY_LIBS = server chunk entitydef pyscript sqlite

USE_PYTHON = 1

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

BUILD_TIME_FILE = main

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
