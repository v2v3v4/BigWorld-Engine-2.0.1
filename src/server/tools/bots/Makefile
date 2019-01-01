BIN  = bots

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/$(BIN),,$(CURDIR))
endif

SRCS =														\
	bots_config												\
	beeline_controller										\
	client_app												\
	entity													\
	entity_type												\
	main													\
	main_app												\
	movement_controller										\
	patrol_graph											\
	py_bots													\
	py_entities												\
	py_server												\
	space_data_manager										\
	zigzag_patrol_graph										\
	../../../common/simple_client_entity					\


ASMS =

MY_LIBS = entitydef pyscript server chunk connection

USE_PYTHON = 1

USE_OPENSSL = 1

include $(MF_ROOT)/bigworld/src/build/common.mak

all::
