# Thread Local Storage is broken in redhat8 (with a non-tls ld-linux.so) and
# Debian (up to and including Sarge).
# LDLINUX_TLS_IS_BROKEN = 0

ifndef MF_CONFIG
	MF_CONFIG=Hybrid
	ifeq ($(shell uname -m),x86_64)
		 MF_CONFIG=Hybrid64
	endif
endif

# As of BigWorld 2.0 32 bit builds are no longer supported. Enabling this
# option is an acknowledgement that you are using unsupported code.
ALLOW_32BIT_BUILD=0

ifeq (,$(findstring 64,$(MF_CONFIG)))
 ifeq (0,$(ALLOW_32BIT_BUILD))

all::
	@echo "ERROR: 32 bit builds are not supported as of BigWorld 2.0 ($(MF_CONFIG))"
	@false

 endif
endif

# This variable is used by src/lib/python/configure to determine whether to
# print out nasty error messages.
export BUILDING_BIGWORLD=1


ifeq (,$(findstring $(MF_CONFIG), Release Hybrid Debug Evaluation \
	Debug_SingleThreaded \
	Hybrid_SingleThreaded \
	Hybrid64 Hybrid64_SingleThreaded \
	Hybrid_SystemPython Hybrid64_SystemPython \
	Debug_SystemPython Debug64_SystemPython \
	Release_SingleThreaded  \
	Debug64 Debug64_SingleThreaded \
	Debug64_GCOV Debug64_GCOV_SingleThreaded Debug64_GCOV_SystemPython \
	Debug_GCOV Debug_GCOV_SingleThreaded Debug_GCOV_SystemPython ))
all:: 
	@echo Error - Unknown configuration type $(MF_CONFIG)
	@false
endif

LIBDIR = $(MF_ROOT)/bigworld/src/lib/bin/$(MF_CONFIG)

ifneq (,$(findstring s, $(MAKEFLAGS)))
QUIET_BUILD=1
endif

# Script to help discover system python settings if necessary
PYTHON_DISCOVERY := $(MF_ROOT)/bigworld/src/build/discover_python.sh

# In order to build src/lib/python, which includes this file, we need to define
# this even when not explicitly requiring Python. This assists in setting up
# the target for libpython<version>.a when common.mak is re-included.
PYTHONLIB = python2.6

# Set to 1 if the Python redist should be built as a standard (no modification)
# build. There are still minor changes from BigWorld, but will be closer enough
# for most purposes.
BW_STANDARD_PYTHON = 0



# If SEPARATE_DEBUG_INFO is defined, the debug information for an executable
# will be placed in a separate file. For example, cellapp and cellapp.dbg. The
# majority of the executable's size is debug information.
# SEPARATE_DEBUG_INFO=1

# This file is used for somewhat of a hack. We want to display a line of info
# the first time a .o is made for a component (and not display if no .o files
# are made.
MSG_FILE := make$(MAKELEVEL)_$(shell echo $$RANDOM).tmp

ifdef BIN
MAKE_LIBS=1
ifndef INSTALL_DIR
ifeq ($(IS_COMMAND),1)
	OUTPUTDIR = $(MF_ROOT)/bigworld/bin/$(MF_CONFIG)/commands
else
	OUTPUTDIR = $(MF_ROOT)/bigworld/bin/$(MF_CONFIG)
endif # IS_COMMAND == 1
else # INSTALL_DIR

# INSTALL_ALL_CONFIGS has been put in to be used by unit_tests so the Debug
# and Hybrid binaries are both placed in MF_ROOT/tests/MF_CONFIG not just
# the Hybrid builds.
ifdef INSTALL_ALL_CONFIGS
	OUTPUTDIR = $(INSTALL_DIR)/$(MF_CONFIG)
else
# For the tools, the Hybrid configuration is automatically made into the install
# directory. Other configurations are made locally.
ifeq ($(MF_CONFIG), Hybrid) 
	OUTPUTDIR = $(INSTALL_DIR)
else # MF_CONFIG == Hybrid

ifeq ($(MF_CONFIG), Hybrid64) 
	OUTPUTDIR = $(INSTALL_DIR)
else # MF_CONFIG == Hybrid64
	OUTPUTDIR = $(MF_CONFIG)
endif # MF_CONFIG == Hybrid64

endif # MF_CONFIG == Hybrid
endif # INSTALL_DIR
endif # INSTALL_ALL_CONFIGS

	OUTPUTFILE = $(OUTPUTDIR)/$(BIN)
endif # BIN

ifdef SO
MAKE_LIBS=1
ifndef OUTPUTDIR
	OUTPUTDIR = $(MF_ROOT)/bigworld/bin/$(MF_CONFIG)/$(COMPONENT)-extensions
endif # OUTPUTDIR
	OUTPUTFILE = $(OUTPUTDIR)/$(SO).so
endif # SO

ifdef LIB
	OUTPUTDIR = $(LIBDIR)
	OUTPUTFILE = $(OUTPUTDIR)/lib$(LIB).a
endif

#----------------------------------------------------------------------------
# Macros
#----------------------------------------------------------------------------

# Our source files
OUR_C = $(addsuffix .c, $(CSRCS))
OUR_CPP = $(addsuffix .cpp, $(SRCS))
OUR_ASMS = $(addsuffix .s, $(ASMS))
ALL_SRC = $(SRCS) $(CSRCS) $(ASMS)

# All .o files that need to be linked
OBJS = $(addsuffix .o, $(ALL_SRC))

# Standard libs that everyone gets
# don't want these for a shared object - we'll use the exe's instead
ifndef SO
ifndef NO_EXTRA_LIBS
MY_LIBS += network resmgr zip math cstdmf
endif
endif

# Include and lib paths
LDFLAGS += -L$(LIBDIR)
BW_INCLUDES += -I $(MF_ROOT)/bigworld/src/lib
BW_INCLUDES += -I $(MF_ROOT)/bigworld/src
BW_INCLUDES += -I $(MF_ROOT)/bigworld/src/server

# Preprocessor output only (useful when debugging macros)
# CPPFLAGS += -E
# CPPFLAGS += -save-temps

LDLIBS += $(addprefix -l, $(MY_LIBS))
LDLIBS += -lm

ifdef USE_PYTHON

 USE_BW_PYTHON = 1

 # If empty string != SystemPython
 ifneq (,$(findstring SystemPython, $(MF_CONFIG)))
	USE_BW_PYTHON = 0
 endif

 # If empty string != SingleThreaded
 ifneq (,$(findstring SingleThreaded, $(MF_CONFIG)))
	USE_BW_PYTHON = 1
	BW_STANDARD_PYTHON = 1
	# These flags are defined so that any BigWorld library that includes
	# the Python headers will work correctly. The src/lib/python Makefile
	# has its own definition of these if it needs to build the same way.
	CPPFLAGS+=-DBW_DONT_WRAP_MALLOC -DBW_PY_NO_RES_FS
 endif
 
 # Now set the environment for the version we require
 ifeq ($(USE_BW_PYTHON),0)

   BW_INCLUDES += `$(PYTHON_DISCOVERY) --includes`
   LDFLAGS += `$(PYTHON_DISCOVERY) --ldflags`
   CPPFLAGS+=-fpermissive -Wno-error

 # Otherwise we are trying to link against the system libraries
 else

   USE_BW_PYTHON = 1

   # This is the version of python BigWorld is redistributing
   BW_INCLUDES += -I $(MF_ROOT)/bigworld/src/lib/python/Include
   LDLIBS += -l$(PYTHONLIB) -lpthread -lutil -ldl

 endif 

endif # USE_PYTHON

# everyone needs pthread if LDLINUX_TLS_IS_BROKEN
ifdef LDLINUX_TLS_IS_BROKEN
CPPFLAGS += -DLDLINUX_TLS_IS_BROKEN
LDLIBS += -lpthread
endif

LDFLAGS += -export-dynamic

# The OpenSSL redist is used for all builds as cstdmf/md5.[ch]pp depends
# on the OpenSSL MD5 implementation.
OPENSSL_DIR = $(MF_ROOT)/bigworld/src/lib/third_party/openssl
BW_INCLUDES += -I$(OPENSSL_DIR)/include
ifeq ($(USE_OPENSSL),1)
LDLIBS += -lssl -lcrypto -ldl
CPPFLAGS += -DUSE_OPENSSL
endif

ifneq (,$(findstring 64,$(MF_CONFIG)))
	x86_64=1
	OPENSSL_CONFIG="x86_64=1"
	PYTHON_EXTRA_CFLAGS="EXTRA_CFLAGS=-m64 -fPIC"
	ARCHFLAGS=-m64 -fPIC
	MYSQL_CONFIG_PATH=/usr/lib64/mysql/mysql_config
else
	OPENSSL_CONFIG=
	PYTHON_EXTRA_CFLAGS="EXTRA_CFLAGS=-m32"
	ARCHFLAGS=-m32
	MYSQL_CONFIG_PATH=/usr/lib/mysql/mysql_config
endif

ifdef USE_CPPUNITLITE2
CPPUNITLITE2_DIR = $(MF_ROOT)/bigworld/src/lib/third_party/CppUnitLite2/src/
BW_INCLUDES += -I$(CPPUNITLITE2_DIR)
LDLIBS += -lCppUnitLite2
endif

# Use backwards compatible hash table style. This is because Fedora Core 6
# defaults to using "gnu" style hash tables which produces incompatible
# binaries with FC5 and before.
#
# By setting it to "both", we can have advantages of the faster hash style
# on FC6 systems and backwards compatibilities with older systems, at a
# small size penalty which is < 0.1% of file size.
ifneq ($(shell gcc -dumpspecs|grep "hash-style"),)
LDFLAGS += -Wl,--hash-style=both
endif

#----------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------

ifndef CC
CC = gcc
endif

ifndef CXX
CXX = g++
endif

ifdef QUIET_BUILD
ARFLAGS = rsu
else
ARFLAGS = rsuv
endif
# CXXFLAGS = -W -Wall -pipe -Wno-uninitialized -Wno-deprecated
CXXFLAGS = $(ARCHFLAGS) -pipe
CXXFLAGS += -Wall -Wno-deprecated
CXXFLAGS += -Wno-uninitialized -Wno-char-subscripts
CXXFLAGS += -fno-strict-aliasing -Wno-non-virtual-dtor
CXXFLAGS += -Werror

CPPFLAGS += -DMF_SERVER -MMD -DMF_CONFIG=\"${MF_CONFIG}\"

ifeq (,$(findstring SingleThreaded,$(MF_CONFIG)))
LDLIBS += -lpthread
endif

# CPPFLAGS += -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT
# CPPFLAGS += -DINSTRUMENTATION
# CPPFLAGS += -DUDP_PROXIES

ifeq ($(MF_CONFIG), Release)
	CXXFLAGS += -O3
	CPPFLAGS += -DCODE_INLINE -D_RELEASE
endif

ifneq (,$(findstring Hybrid,$(MF_CONFIG)))
	CXXFLAGS += -O3 -g
	CPPFLAGS += -DCODE_INLINE -DMF_USE_ASSERTS -D_HYBRID
endif

ifeq ($(MF_CONFIG), Evaluation)
	CXXFLAGS += -O3 -g
	CPPFLAGS += -DCODE_INLINE -DMF_USE_ASSERTS -D_HYBRID -DBW_EVALUATION
endif

ifneq (,$(findstring Debug,$(MF_CONFIG)))
	CXXFLAGS += -g
	CPPFLAGS += -DMF_USE_ASSERTS -D_DEBUG
endif

ifeq ($(MF_CONFIG), Release_SingleThreaded)
	CXXFLAGS += -O3
	CPPFLAGS += -DCODE_INLINE -D_RELEASE -DMF_SINGLE_THREADED
endif

ifneq (,$(findstring SingleThreaded,$(MF_CONFIG)))
	CPPFLAGS += -DMF_SINGLE_THREADED

endif

ifneq (,$(findstring Evaluation,$(MF_CONFIG)))
	CPPFLAGS += -DBW_EVALUATION

endif

ifneq (,$(findstring GCOV, $(MF_CONFIG)))
	CXXFLAGS += -fprofile-arcs -ftest-coverage 
endif


CCFLAGS += $(MY_DEFINES) $(MY_CPPFLAGS)
LDFLAGS += $(MY_LDFLAGS)

CFLAGS += $(ARCHFLAGS)

#----------------------------------------------------------------------------
# Build variables
#----------------------------------------------------------------------------

# These variables are defined by make (see 'make -p').

# Add BW_INCLUDES to the compilation variables. By not including them in
# the CFLAGS / CXXFLAGS it helps tidy up the link step.
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(BW_INCLUDES) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(BW_INCLUDES) $(TARGET_ARCH) -c

# Removed CPPFLAGS from the linker to remove all our -DDEFINES during linking
LINK.cc = $(CXX) $(CXXFLAGS) $(LDFLAGS) $(TARGET_ARCH)



#----------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------

all:: $(OUTPUTDIR) $(MF_CONFIG) $(OUTPUTFILE) done

all_config:
	$(MAKE) MF_CONFIG=Debug
	$(MAKE) MF_CONFIG=Hybrid
	$(MAKE) MF_CONFIG=Release

done:
ifdef DO_NOT_BELL
else
ifeq (0, $(MAKELEVEL))
	@echo -n 
endif
endif


ifdef QUIET_BUILD
RM_FLAGS = "-f"
else
RM_FLAGS = "-fv"
endif

ifeq ($(wildcard *.cpp *.c $(MF_CONFIG)/*.o), )	# only if it has some cpps/c or object files!
SHOULD_NOT_LINK=1
endif

clean::
	@filemissing=0;  					\
	 for i in $(SRCS); do 				\
		if [ -e $$i.cpp ]; then		 	\
			rm $(RM_FLAGS) $(MF_CONFIG)*/`basename $$i`.[do]; \
		else 							\
			filemissing=1; 				\
		fi; 							\
	 done; 								\
	 if [ $$filemissing -ne 1 ]; then	\
		rm $(RM_FLAGS) $(MF_CONFIG)*/* ;\
	 fi
ifdef SHOULD_NOT_LINK
	@echo Not removing $(OUTPUTFILE) since no source to remake
else
ifdef LIB
	@rm $(RM_FLAGS) $(OUTPUTDIR)*/lib$(LIB).a
else
	@rm $(RM_FLAGS) $(OUTPUTFILE)
endif
endif

ifneq ($(OUTPUTDIR), $(MF_CONFIG))
$(OUTPUTDIR):
	@mkdir -p $(OUTPUTDIR)
endif

$(MF_CONFIG):
	@mkdir -p $(MF_CONFIG)

ifdef INSTALL_DIR
install::
	@mkdir -p $(INSTALL_DIR)
	@cp $(OUTPUTFILE) $(INSTALL_DIR)
else
install::
endif

ifneq ($(wildcard unit_test), )	# only if it has some cpps/c or object files!
HAS_UNIT_TEST=1
endif

unit_tests:: unit_tests_build unit_tests_run

unit_tests_build::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test
endif

unit_tests_run::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test run
endif

unit_tests_clean::
ifdef HAS_UNIT_TEST
	$(MAKE) -C unit_test clean
endif

#----------------------------------------------------------------------------
# Library dependencies
#----------------------------------------------------------------------------

# Get the full path for all non-system libraries, so we can use them
# as dependencies on the main target. We need to do a recursive make
# to work out the dependencies of each lib, and a phony target is
# necessary so the libs still get checked after they are built the
# first time.

ifdef MAKE_LIBS
MY_LIBNAMES = $(foreach L, $(MY_LIBS), $(LIBDIR)/lib$(L).a)

.PHONY: always

BW_PYTHONLIB=$(LIBDIR)/lib$(PYTHONLIB).a

ifdef USE_BW_PYTHON
$(BW_PYTHONLIB): always
	@$(MAKE) -C $(MF_ROOT)/bigworld/src/lib/python $(LIBDIR)/lib$(PYTHONLIB).a \
		"BW_STANDARD_PYTHON=$(BW_STANDARD_PYTHON)" \
		"MF_CONFIG=$(MF_CONFIG)" \
		$(PYTHON_EXTRA_CFLAGS) 
endif

ifeq ($(USE_OPENSSL),1)
$(LIBDIR)/libcrypto.a: always
	@$(MAKE) -C $(OPENSSL_DIR) $(OPENSSL_CONFIG) build_crypto

$(LIBDIR)/libssl.a: always
	@$(MAKE) -C $(OPENSSL_DIR) $(OPENSSL_CONFIG) build_ssl
endif

ifdef USE_CPPUNITLITE2
$(LIBDIR)/libCppUnitLite2.a: always
	@$(MAKE) -C $(CPPUNITLITE2_DIR)
endif

# Strip the prefixed "lib" string. Be careful not to strip any _lib
$(MY_LIBNAMES): always
	$(MAKE) -C $(MF_ROOT)/bigworld/src/lib/$(subst XXXXX,_lib,$(subst lib,,$(subst _lib,XXXXX,$(*F)))) \
		"MF_CONFIG=$(MF_CONFIG)"

endif # MAKE_LIBS

#----------------------------------------------------------------------------
# File dependencies
#----------------------------------------------------------------------------

# If the dependency file doesn't exist, neither does the .o
# The .d will be created the first time the .o is built, so this is fine.

ifneq ($(OUR_CPP),)
-include $(addprefix $(MF_CONFIG)/, $(notdir $(OUR_CPP:.cpp=.d)))
endif

# About the notdir: For some annoying reason, and despite the information
# in the gcc man page, %.d's are always written into the current directory.
# There's no way I'm redefining the default %.cpp rule to later move this
# (or to cd first, even 'tho that could be quite useful), so each binary
# has its own version of the %.d's. I think Murph would like this
# This does however raise one minor requirement: the binary cannot use two
# sources with the same name even if they are in different directories.
DIRLESS_OBJS = $(notdir $(OBJS))
CONFIG_OBJS = $(addprefix $(MF_CONFIG)/, $(DIRLESS_OBJS))

# Macro that will return any string in the second arg that matches the string in
# the first argument
grep = $(foreach a,$(2),$(if $(findstring $(1),$(a)),$(a)))

# Rules to set up vpaths for sources outside the current directory.
$(foreach dd,$(call grep,/,$(CSRCS)),$(eval vpath $(notdir $(dd)).c $(dir $(dd))))
$(foreach dd,$(call grep,/,$(SRCS)),$(eval vpath $(notdir $(dd)).cpp $(dir $(dd))))
$(foreach dd,$(call grep,/,$(ASMS)),$(eval vpath $(notdir $(dd)).s $(dir $(dd))))

#----------------------------------------------------------------------------
# Precompiled headers
#----------------------------------------------------------------------------

ifdef HAS_PCH
$(MF_CONFIG)/pch.hpp:
	echo '#include "../pch.hpp"' > $(MF_CONFIG)/pch.hpp

$(MF_CONFIG)/pch.hpp.gch: $(MF_CONFIG)/pch.hpp pch.hpp
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo pch.hpp
endif
	rm -f $(MF_CONFIG)/pch.hpp.gch
	$(COMPILE.cc) -x c++-header $(MF_CONFIG)/pch.hpp $(OUTPUT_OPTION)

-include $(MF_CONFIG)/pch.hpp.d

PCH_DEP = $(MF_CONFIG)/pch.hpp.gch
CPPFLAGS += -include $(MF_CONFIG)/pch.hpp
else
PCH_DEP =
endif


#----------------------------------------------------------------------------
# Implicit rules
#----------------------------------------------------------------------------

# This implicit rule is needed for three reasons.
# 1. To place .o files in the $(MF_CONFIG) directory.
# 2. To move the .d file into the $(MF_CONFIG) directory
# 3. To change the target in the .d file to include the $(MF_CONFIG) directory.

# Note there is a bug in gcc 2.91, where the dependency file is always
# placed in the current directory regardless of the path. If we find it
# in the current directory, we move it into the MF_CONFIG directory.
# So this should work for both old and new versions of gcc.

$(MF_CONFIG)/%.o: %.cpp $(PCH_DEP)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
	@if test -e $*.d; then echo -n $(MF_CONFIG)/ > $(MF_CONFIG)/$*.d; \
		cat $*.d >> $(MF_CONFIG)/$*.d; rm $*.d; fi

$(MF_CONFIG)/%.o: %.c $(PCH_DEP)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.c) $< $(OUTPUT_OPTION)
	@if test -e $*.d; then echo -n $(MF_CONFIG)/ > $(MF_CONFIG)/$*.d; \
		cat $*.d >> $(MF_CONFIG)/$*.d; rm $*.d; fi

#----------------------------------------------------------------------------
# Local targets
#----------------------------------------------------------------------------

ifeq ($(USE_OPENSSL),1)
OPENSSL_DEP = $(LIBDIR)/libssl.a $(LIBDIR)/libcrypto.a
else
OPENSSL_DEP =
endif


# For executables

ifdef BIN

# This target is an additional one for executables to create the file containing
# the "first line" info. This is the info that is displayed if any .o are made.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(BIN)::
	@echo -e \\n------ Configuration $(@F) - $(MF_CONFIG) ------ > $(MSG_FILE)
endif

ifdef USE_BW_PYTHON
PYTHON_DEP = $(BW_PYTHONLIB)
else
PYTHON_DEP =
endif

ifdef USE_CPPUNITLITE2
CPPUNITLITE2_DEP = $(LIBDIR)/libCppUnitLite2.a
else
CPPUNITLITE2_DEP =
endif


$(OUTPUTDIR)/$(BIN):: $(CONFIG_OBJS) $(MY_LIBNAMES) $(PYTHON_DEP) $(OPENSSL_DEP) $(CPPUNITLITE2_DEP)

ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
endif
ifdef BUILD_TIME_FILE
	@echo Updating Compile Time String
	@if test -e $(BUILD_TIME_FILE).cpp; then touch $(BUILD_TIME_FILE).cpp; $(MAKE) $(MF_CONFIG)/$(BUILD_TIME_FILE).o; fi
endif
ifdef QUIET_BUILD
	@echo Linking...
endif
	$(LINK.cc) -o $@ $(CONFIG_OBJS) $(LDLIBS) $(POSTLINK)
ifdef SEPARATE_DEBUG_INFO
	@objcopy --only-keep-debug $@ $@.dbg
	@objcopy --strip-debug $@
	@objcopy --add-gnu-debuglink=$@.dbg $@
endif
ifdef QUIET_BUILD
	@echo $@
endif

# This target is an additional one to clean up "first line" info.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(BIN)::
	@rm -f $(MSG_FILE)
endif

endif # BIN



# for shared objects
ifdef SO

ifdef USE_BW_PYTHON
PYTHON_DEP = $(BW_PYTHONLIB)
else
PYTHON_DEP =
endif

ifdef QUIET_BUILD
$(OUTPUTDIR)/$(SO).so::
	@echo -e \\n------ Configuration $(@F) - $(MF_CONFIG) ------ > $(MSG_FILE)
endif

ifdef BUILD_TIME_FILE
BUILD_TIME_FILE_OBJ= $(MF_CONFIG)/$(BUILD_TIME_FILE).o
endif

$(OUTPUTDIR)/$(SO).so:: $(CONFIG_OBJS) $(MY_LIBNAMES) $(PYTHON_DEP) $(BUILD_TIME_FILE_OBJ) $(OPENSSL_DEP)
ifdef BUILD_TIME_FILE
	@echo Updating Compile Time String
	@if test -e $(BUILD_TIME_FILE).cpp; then \
		touch -m $(BUILD_TIME_FILE).cpp; \
		$(MAKE) $(BUILD_TIME_FILE_OBJ); \
	fi
endif # BUILD_TIME_FILE

ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo Linking...
endif
	$(LINK.cc) -shared -o $@ $(CONFIG_OBJS) $(BUILD_TIME_FILE_OBJ) $(LDLIBS) $(POSTLINK) 
ifdef QUIET_BUILD
	@echo $@
endif

# This target is an additional one to clean up "first line" info.
ifdef QUIET_BUILD
$(OUTPUTDIR)/$(SO).so::
	@rm -f $(MSG_FILE)
endif

endif # SO



# For libraries

ifdef LIB

ifndef SHOULD_NOT_LINK
ifdef QUIET_BUILD
$(OUTPUTDIR)/lib$(LIB).a::
	@echo -e \\n------ Configuration $(@F) - $(MF_CONFIG) ------ > $(MSG_FILE)
endif

$(OUTPUTDIR)/lib$(LIB).a:: $(CONFIG_OBJS)
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo Archiving to $(@F)
endif
	@$(AR) $(ARFLAGS) $@ $(CONFIG_OBJS)
ifdef QUIET_BUILD
	@echo $@
endif

ifdef QUIET_BUILD
$(OUTPUTDIR)/lib$(LIB).a::
	@rm -f $(MSG_FILE)
endif

else	# wildcard
#do nothing if no cpps
$(OUTPUTDIR)/lib$(LIB).a::
	@echo Not building library \'$(LIB)\' since source not present.
endif

endif	# LIB
