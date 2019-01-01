LIB = 	chunk

SRCS =								\
	base_chunk_space				\
	base_chunk_tree					\
	chunk 							\
	chunk_boundary					\
	chunk_exit_portal				\
	chunk_item						\
	chunk_link						\
	chunk_loader					\
	chunk_model_obstacle			\
	chunk_obstacle					\
	chunk_overlapper				\
	chunk_space						\
	chunk_stationnode				\
	chunk_terrain_obstacle			\
	chunk_user_data_object			\
	geometry_mapping				\
	portal_chunk_item				\
	server_chunk_manager			\
	server_chunk_model				\
	server_chunk_space				\
	server_chunk_terrain			\
	server_chunk_tree				\
	server_super_model				\
	station_graph					\
	user_data_object				\
	user_data_object_type			\
	user_data_object_link_data_type	\
	

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

