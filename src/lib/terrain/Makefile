LIB =	terrain

SRCS =								\
	base_terrain_block				\
	height_map_compress				\
	manager						\
	terrain_block_cache				\
	terrain_collision_callback		\
	terrain_finder					\
	terrain_height_map				\
	terrain_settings				\
	terrain1/common_terrain_block1	\
	terrain1/terrain_block1			\
	terrain1/terrain_height_map1	\
	terrain1/terrain_hole_map1		\
	dominant_texture_map	\
	terrain1/terrain_texture_layer1	\
	terrain2/common_terrain_block2	\
	terrain2/dominant_texture_map2	\
	terrain2/terrain_quad_tree_cell	\
	terrain2/terrain_hole_map2		\
	terrain2/terrain_height_map2	\
	terrain2/terrain_block2			\
	terrain2/terrain_texture_layer2

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak
