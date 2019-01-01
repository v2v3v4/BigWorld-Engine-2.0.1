LIB = zip

CSRCS =					\
	adler32				\
	compress			\
	crc32				\
	deflate				\
	gzio				\
	infback				\
	inffast				\
	inflate				\
	inftrees			\
	trees				\
	uncompr				\
	zutil

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

