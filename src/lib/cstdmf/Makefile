LIB =	cstdmf

SRCS =					\
	base64				\
	bgtask_manager		\
	blob_or_null		\
	binary_stream		\
	bwrandom			\
	bwversion			\
	bw_util				\
	bw_util_linux		\
	concurrency			\
	cstdmf				\
	debug				\
	diary				\
	dogwatch			\
	dprintf				\
	fini_job			\
	intrusive_object	\
	locale				\
	md5					\
	memory_tracker		\
	profile				\
	profiler			\
	string_utils		\
	time_queue			\
	timestamp			\
	unique_id			\
	watcher				\
	watcher_path_request\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

