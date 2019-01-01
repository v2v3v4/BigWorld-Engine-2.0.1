LIB = server

SRCS =									\
	add_to_manager_helper				\
	anonymous_channel_client			\
	auto_backup_and_archive				\
	app_script_timers					\
	backup_hash							\
	backup_hash_chain					\
	balance_config						\
	base_backup_switch_mailbox_visitor 	\
	bwconfig							\
	bwservice							\
	config_reader						\
	entity_app							\
	entity_app_config					\
	event_history_stats					\
	external_app_config					\
	id_client							\
	manager_app							\
	manager_app_config					\
	manager_app_gateway					\
	plugin_library						\
	python_server						\
	reviver_subject						\
	script_timers						\
	signal_processor					\
	server_app							\
	server_app_config					\
	server_app_option					\
	server_info							\
	shared_data							\
	time_keeper							\
	watcher_forwarding					\
	watcher_forwarding_collector		\
	watcher_protocol

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak
