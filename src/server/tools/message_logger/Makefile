# This Makefile is a meta makefile for the message_logger directory. It
# calls two other Makefiles to do the actual work:
#
# Makefile.message_logger - builds the message_logger binary for handling
#                           BigWorld server process logs.
# Makefile.message_reader - builds the bwlog.so Python module

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools/message_logger,,$(CURDIR))
endif

all clean realclean install::
	@$(MAKE) -f Makefile.message_logger $@
	@$(MAKE) -f Makefile.message_reader $@

include $(MF_ROOT)/bigworld/src/build/common.mak
