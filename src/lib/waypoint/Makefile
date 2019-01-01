LIB = waypoint

SRCS =							\
	chunk_navigator				\
	chunk_waypoint				\
	chunk_waypoint_set			\
	chunk_waypoint_set_data 	\
	chunk_waypoint_set_state_path	\
	chunk_waypoint_set_state 	\
	chunk_waypoint_state 		\
	chunk_waypoint_state_path	\
	girth_grid_list				\
	navigator					\
	navigator_cache				\
	navloc						\
	waypoint_neighbour_iterator	\
	waypoint_stats				\


# Windows only files
# waypoint_generator waypoint_flood

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak
