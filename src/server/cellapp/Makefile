BIN  = cellapp
SRCS =										\
	ack_cell_app_death_helper				\
	aoi_update_schemes						\
	buffered_entity_messages				\
	buffered_ghost_message					\
	buffered_ghost_message_factory			\
	buffered_ghost_message_queue			\
	buffered_ghost_messages					\
	buffered_ghost_messages_for_entity		\
	cell									\
	cell_info								\
	cells									\
	cellapp									\
	cellapp_config							\
	cellappmgr_gateway						\
	cellapp_death_listener					\
	cell_app_channel						\
	cell_app_channels						\
	cell_chunk								\
	cell_range_list							\
	cell_viewer_connection					\
	cell_viewer_server						\
	controllers								\
	emergency_throttle						\
	entity									\
	entity_cache							\
	entity_extra							\
	entity_navigate							\
	entity_population						\
	entity_range_list_node					\
	entity_type								\
	entity_vision							\
	external_interfaces						\
	history_event							\
	id_config								\
	loading_column							\
	loading_edge							\
	mailbox									\
	main									\
	message_handlers						\
	noise_config							\
	passenger_extra							\
	passengers								\
	profile									\
	py_client								\
	py_entities								\
	range_list_node							\
	real_caller								\
	real_entity								\
	server_geometry_mapping					\
	server_geometry_mappings				\
	space									\
	space_branch							\
	spaces									\
	throttle_config							\
	updatables								\
	witness									\
	controller								\
	accelerate_along_path_controller		\
	accelerate_to_entity_controller			\
	accelerate_to_point_controller			\
	base_acceleration_controller			\
	face_entity_controller					\
	move_controller							\
	navigation_controller					\
	passenger_controller					\
	portal_config_controller				\
	proximity_controller					\
	scan_vision_controller					\
	timer_controller						\
	turn_controller							\
	visibility_controller					\
	vision_controller						\
	../../common/py_chunk					\
	../../common/db_interface_utils			\
	../../common/doc_watcher				\
	../../common/py_network					\
	../../common/py_physics2				\
	../../common/py_server					\

ASMS =

MY_LIBS =		\
	pyscript	\
	entitydef	\
	server		\
	waypoint	\
	chunk		\
	terrain		\
	physics2	\
	moo			\
	speedtree	\
	png

BUILD_TIME_FILE = main
USE_PYTHON = 1

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

#HAS_PCH = 1

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/$(BIN),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

ifeq ($(wildcard cell_range_list.cp*),cell_range_list.cpp)
# this file doesn't want to be optimised ... for some reason
$(MF_CONFIG)/cell_range_list.o: cell_range_list.cpp
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.cc) -O0 $< $(OUTPUT_OPTION)
	@if test -e cell_range_list.d; then 						\
		echo -n $(MF_CONFIG)/ > $(MF_CONFIG)/cell_range_list.d;	\
		cat cell_range_list.d >> $(MF_CONFIG)/cell_range_list.d;\
		rm cell_range_list.d;									\
	fi
endif

