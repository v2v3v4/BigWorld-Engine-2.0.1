LIB =	entitydef

SRCS =											\
		base_user_data_object_description		\
		bit_reader								\
		bit_writer								\
		data_description						\
		data_lod_level							\
		data_type								\
		data_types								\
		entity_description						\
		entity_description_debug				\
		entity_description_map					\
		entity_member_stats						\
		entity_method_descriptions				\
		mailbox_base							\
		meta_data_type							\
		method_description						\
		method_response							\
		property_change							\
		property_change_reader					\
		property_event_stamps					\
		property_owner							\
		py_volatile_info						\
		remote_entity_method 					\
		user_data_object_description			\
		user_data_object_description_map		\
		volatile_info							\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak
