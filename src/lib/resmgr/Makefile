LIB = resmgr

SRCS =					\
	access_monitor		\
	auto_config			\
	bdiff				\
	binary_block		\
	bin_section			\
	bundiff				\
	bwresource			\
	dataresource		\
	datasection			\
	data_section_cache	\
	data_section_census	\
	dir_section			\
	multi_file_system	\
	packed_section		\
	primitive_file		\
	sanitise_helper		\
	string_provider		\
	unix_file_system	\
	xml_section			\
	xml_special_chars	\
	zip_file_system		\
	zip_section			\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

CPPFLAGS += -DHAVE_ZIP
