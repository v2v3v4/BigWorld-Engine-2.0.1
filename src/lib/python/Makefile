# Generated automatically from Makefile.pre by makesetup.
# Top-level Makefile for Python
#
# As distributed, this file is called Makefile.pre.in; it is processed
# into the real Makefile by running the script ./configure, which
# replaces things like @spam@ with values appropriate for your system.
# This means that if you edit Makefile, your changes get lost the next
# time you run the configure script.  Ideally, you can do:
#
#	./configure
#	make
#	make test
#	make install
#
# If you have a previous version of Python installed that you don't
# want to overwrite, you can use "make altinstall" instead of "make
# install".  Refer to the "Installing" section in the README file for
# additional details.
#
# See also the section "Build instructions" in the README file.

# Bigworld: include our common makefile here to get access to the MF_CONFIG 
ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/python,,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

# The LIBDIR used by BigWorld will be overridden in this Makefile,
# so lets call it something else.
BWLIBDIR = $(MF_ROOT)/bigworld/src/lib/bin/$(MF_CONFIG)
BW_PYTHONLIB = $(BWLIBDIR)/lib$(PYTHONLIB).a

#BW_STANDARD_PYTHON = 1

BW_PY_CFLAGS=
ifeq ($(BW_STANDARD_PYTHON),1)
 MF_CONFIG := Vanilla
 BW_PY_CFLAGS=-DBW_DONT_WRAP_MALLOC -DBW_PY_NO_RES_FS
endif

# === Variables set by makesetup ===

MODOBJS=          Modules/$(MF_CONFIG)/threadmodule.o  Modules/$(MF_CONFIG)/signalmodule.o  Modules/$(MF_CONFIG)/posixmodule.o  Modules/$(MF_CONFIG)/errnomodule.o  Modules/$(MF_CONFIG)/pwdmodule.o  Modules/$(MF_CONFIG)/_sre.o  Modules/$(MF_CONFIG)/_codecsmodule.o  Modules/$(MF_CONFIG)/zipimport.o  Modules/$(MF_CONFIG)/symtablemodule.o  Modules/$(MF_CONFIG)/xxsubtype.o
MODLIBS=        $(LOCALMODLIBS) $(BASEMODLIBS)

# === Variables set by configure
VERSION=	2.6
srcdir=		.


CC=		gcc -pthread
CXX=		g++ -pthread
MAINCC=		$(CC)
LINKCC=		$(PURIFY) $(MAINCC)
AR=		ar
RANLIB=		ranlib
SVNVERSION=	svnversion $(srcdir)

# Shell used by make (some versions default to the login shell, which is bad)
SHELL=		/bin/sh

# Use this to make a link between python$(VERSION) and python in $(BINDIR)
LN=		ln

# Portable install script (configure doesn't always guess right)
INSTALL=	/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_SCRIPT= ${INSTALL}
INSTALL_DATA=	${INSTALL} -m 644
# Shared libraries must be installed with executable mode on some systems;
# rather than figuring out exactly which, we always give them executable mode.
# Also, making them read-only seems to be a good idea...
INSTALL_SHARED= ${INSTALL} -m 555

MAKESETUP=      $(srcdir)/Modules/makesetup

# Compiler options
OPT:=		-DNDEBUG $(BW_PY_CFLAGS) -g -O3 -Wall -Wstrict-prototypes
BASECFLAGS=	 -fno-strict-aliasing

CFLAGS=		$(BASECFLAGS) $(OPT) $(EXTRA_CFLAGS)
# Both CPPFLAGS and LDFLAGS need to contain the shell's value for setup.py to
# be able to build extension modules using the directories specified in the
# environment variables
CPPFLAGS=	-I. -IInclude -I$(srcdir)/Include
LDFLAGS= $(EXTRA_CFLAGS) $(BW_PY_CFLAGS)
LDLAST=		
SGI_ABI=	
CCSHARED=	-fPIC
LINKFORSHARED=	-Xlinker -export-dynamic
# Extra C flags added for building the interpreter object files.
CFLAGSFORSHARED=
# C flags used for building the interpreter object files
PY_CFLAGS=	$(CFLAGS) $(CPPFLAGS) $(CFLAGSFORSHARED) -DPy_BUILD_CORE


# Machine-dependent subdirectories
MACHDEP=	linux2

# Install prefix for architecture-independent files
prefix=		/usr/local

# Install prefix for architecture-dependent files
exec_prefix=	${prefix}

# Install prefix for data files
datarootdir=    ${prefix}/share

# Expanded directories
BINDIR=		$(exec_prefix)/bin
LIBDIR=		$(exec_prefix)/lib
MANDIR=		${datarootdir}/man
INCLUDEDIR=	${prefix}/include
CONFINCLUDEDIR=	$(exec_prefix)/include
SCRIPTDIR=	$(prefix)/lib

# Detailed destination directories
BINLIBDEST=	$(LIBDIR)/python$(VERSION)
LIBDEST=	$(SCRIPTDIR)/python$(VERSION)
INCLUDEPY=	$(INCLUDEDIR)/python$(VERSION)
CONFINCLUDEPY=	$(CONFINCLUDEDIR)/python$(VERSION)
LIBP=		$(LIBDIR)/python$(VERSION)

# Symbols used for using shared libraries
SO=		.so
LDSHARED=	$(CC) -shared
# BigWorld modified '-fPIC -Wl,-Bsymbolic'
BLDSHARED=	$(CC) -shared -fPIC -Wl,-Bsymbolic
DESTSHARED=	$(BINLIBDEST)/lib-dynload

# Executable suffix (.exe on Windows and Mac OS X)
EXE=		
BUILDEXE=	

# Short name and location for Mac OS X Python framework
UNIVERSALSDK=
PYTHONFRAMEWORK=	
PYTHONFRAMEWORKDIR=	no-framework
PYTHONFRAMEWORKPREFIX=	
PYTHONFRAMEWORKINSTALLDIR= 
# Deployment target selected during configure, to be checked
# by distutils. The export statement is needed to ensure that the
# deployment target is active during build.
MACOSX_DEPLOYMENT_TARGET=
#export MACOSX_DEPLOYMENT_TARGET

# Options to enable prebinding (for fast startup prior to Mac OS X 10.3)
OTHER_LIBTOOL_OPT=

# Environment to run shared python without installed libraries
RUNSHARED=       

# Modes for directories, executables and data files created by the
# install process.  Default to user-only-writable for all file types.
DIRMODE=	755
EXEMODE=	755
FILEMODE=	644

# configure script arguments
# BigWorld specific encoding
CONFIG_ARGS=	'--enable-unicode=ucs4'


# Subdirectories with code
SRCDIRS= 	Parser Grammar Objects Python Modules Mac

# Other subdirectories
SUBDIRSTOO=	Include Lib Misc Demo

# Files and directories to be distributed
CONFIGFILES=	configure configure.in acconfig.h pyconfig.h.in Makefile.pre.in
DISTFILES=	README ChangeLog $(CONFIGFILES)
DISTDIRS=	$(SUBDIRS) $(SUBDIRSTOO) Ext-dummy
DIST=		$(DISTFILES) $(DISTDIRS)


LIBRARY=	libpython$(VERSION).a
LDLIBRARY=      libpython$(VERSION).a
BLDLIBRARY=     $(LDLIBRARY)
DLLLIBRARY=	
LDLIBRARYDIR=   
INSTSONAME=	$(LDLIBRARY)


LIBS=		-lpthread -ldl  -lutil
LIBM=		-lm
LIBC=		
SYSLIBS=	$(LIBM) $(LIBC)
SHLIBS=		$(LIBS)

THREADOBJ=	Python/$(MF_CONFIG)/thread.o
DLINCLDIR=	.
DYNLOADFILE=	dynload_shlib.o
MACHDEP_OBJS=	
LIBOBJDIR=	Python/
LIBOBJS=	
UNICODE_OBJS=   Objects/$(MF_CONFIG)/unicodeobject.o Objects/$(MF_CONFIG)/unicodectype.o

PYTHON=		python$(EXE)
BUILDPYTHON=	python$(BUILDEXE)

# The task to run while instrument when building the profile-opt target
PROFILE_TASK=	$(srcdir)/Tools/pybench/pybench.py -n 2 --with-gc --with-syscheck
#PROFILE_TASK=	$(srcdir)/Lib/test/regrtest.py

# === Definitions added by makesetup ===

LOCALMODLIBS=          
BASEMODLIBS=
GLHACK=-Dclear=__GLclear
PYTHONPATH=$(COREPYTHONPATH)
COREPYTHONPATH=$(DESTPATH)$(SITEPATH)$(TESTPATH)$(MACHDEPPATH)$(EXTRAMACHDEPPATH)$(TKPATH)
TKPATH=:lib-tk
EXTRAMACHDEPPATH=
MACHDEPPATH=:plat-$(MACHDEP)
TESTPATH=
SITEPATH=
DESTPATH=
MACHDESTLIB=$(BINLIBDEST)
DESTLIB=$(LIBDEST)



##########################################################################
# Modules
MODULE_OBJS=	\
		Modules/$(MF_CONFIG)/config.o \
		Modules/$(MF_CONFIG)/getpath.o \
		Modules/$(MF_CONFIG)/main.o \
		Modules/$(MF_CONFIG)/gcmodule.o

# Used of signalmodule.o is not available
SIGNAL_OBJS=	


##########################################################################
# Grammar
GRAMMAR_H=	$(srcdir)/Include/graminit.h
GRAMMAR_C=	$(srcdir)/Python/graminit.c
GRAMMAR_INPUT=	$(srcdir)/Grammar/Grammar


##########################################################################
# Parser
# BigWorld: removing PGEN so it isn't built un-necessarily
#PGEN=		Parser/pgen$(EXE)

# Modified by PaulH. Removed Parser/parser.o from POBJS, added explicitly
# to PARSER_OBJS and a renamed version in PGOBJS to add BW_DONT_WRAP_MALLOC
POBJS=		\
		Parser/$(MF_CONFIG)/acceler.o \
		Parser/$(MF_CONFIG)/grammar1.o \
		Parser/$(MF_CONFIG)/listnode.o \
		Parser/$(MF_CONFIG)/node.o \
		Parser/$(MF_CONFIG)/parsetok.o \
		Parser/$(MF_CONFIG)/bitset.o \
		Parser/$(MF_CONFIG)/metagrammar.o \
		Parser/$(MF_CONFIG)/firstsets.o \
		Parser/$(MF_CONFIG)/grammar.o \
		Parser/$(MF_CONFIG)/pgen.o

PARSER_OBJS=	$(POBJS) Parser/$(MF_CONFIG)/parser.o Parser/$(MF_CONFIG)/myreadline.o Parser/$(MF_CONFIG)/tokenizer.o

# BigWorld modifications:
# This is because we want the un-wrapped malloc etc. for pgen, but wrapped
# malloc for libpython.2.6.a
#
# Original rule: 
# PGOBJS=		\
#		Objects/obmalloc.o \
#		Python/mysnprintf.o \
#		Parser/tokenizer_pgen.o \
#		Parser/printgrammar.o \
#		Parser/pgenmain.o
#
# BigWorld addition:
# separate obmalloc.o -> obmalloc_pgen.o 

PGOBJS=		\
		Objects/$(MF_CONFIG)/obmalloc_pgen.o \
		Python/$(MF_CONFIG)/mysnprintf.o \
		Parser/$(MF_CONFIG)/tokenizer_pgen.o \
		Parser/$(MF_CONFIG)/parser_pgen.o \
		Parser/$(MF_CONFIG)/printgrammar.o \
		Parser/$(MF_CONFIG)/pgenmain.o

Parser/$(MF_CONFIG)/pgenmain.o: Parser/pgenmain.c
	$(CC) -c $(CPPFLAGS) -DBW_DONT_WRAP_MALLOC $(CFLAGS) $^ -o $@

Parser/$(MF_CONFIG)/parser_pgen.o: Parser/parser.c
	$(CC) -c $(CPPFLAGS) -DBW_DONT_WRAP_MALLOC $(CFLAGS) $^ -o $@

Objects/$(MF_CONFIG)/obmalloc_pgen.o: Objects/obmalloc.c
	$(CC) -c $(CPPFLAGS) -DBW_DONT_WRAP_MALLOC $(CFLAGS) $^ -o $@

# End of BigWorld addition

PARSER_HEADERS= \
		Parser/parser.h \
		Parser/tokenizer.h

PGENOBJS=	$(PGENMAIN) $(POBJS) $(PGOBJS)

##########################################################################
# AST
AST_H_DIR=	$(srcdir)/Include
AST_H=		$(AST_H_DIR)/Python-ast.h
AST_C_DIR=	$(srcdir)/Python
AST_C=		$(AST_C_DIR)/Python-ast.c
AST_ASDL=	$(srcdir)/Parser/Python.asdl

ASDLGEN_FILES=	$(srcdir)/Parser/asdl.py $(srcdir)/Parser/asdl_c.py
# XXX Note that a build now requires Python exist before the build starts
ASDLGEN=	$(srcdir)/Parser/asdl_c.py

##########################################################################
# Python
PYTHON_OBJS=	\
		Python/$(MF_CONFIG)/_warnings.o \
		Python/$(MF_CONFIG)/Python-ast.o \
		Python/$(MF_CONFIG)/asdl.o \
		Python/$(MF_CONFIG)/ast.o \
		Python/$(MF_CONFIG)/bltinmodule.o \
		Python/$(MF_CONFIG)/ceval.o \
		Python/$(MF_CONFIG)/compile.o \
		Python/$(MF_CONFIG)/codecs.o \
		Python/$(MF_CONFIG)/errors.o \
		Python/$(MF_CONFIG)/frozen.o \
		Python/$(MF_CONFIG)/frozenmain.o \
		Python/$(MF_CONFIG)/future.o \
		Python/$(MF_CONFIG)/getargs.o \
		Python/$(MF_CONFIG)/getcompiler.o \
		Python/$(MF_CONFIG)/getcopyright.o \
		Python/$(MF_CONFIG)/getmtime.o \
		Python/$(MF_CONFIG)/getplatform.o \
		Python/$(MF_CONFIG)/getversion.o \
		Python/$(MF_CONFIG)/graminit.o \
		Python/$(MF_CONFIG)/import.o \
		Python/$(MF_CONFIG)/importdl.o \
		Python/$(MF_CONFIG)/marshal.o \
		Python/$(MF_CONFIG)/modsupport.o \
		Python/$(MF_CONFIG)/mystrtoul.o \
		Python/$(MF_CONFIG)/mysnprintf.o \
		Python/$(MF_CONFIG)/peephole.o \
		Python/$(MF_CONFIG)/pyarena.o \
		Python/$(MF_CONFIG)/pyfpe.o \
		Python/$(MF_CONFIG)/pymath.o \
		Python/$(MF_CONFIG)/pystate.o \
		Python/$(MF_CONFIG)/pythonrun.o \
		Python/$(MF_CONFIG)/structmember.o \
		Python/$(MF_CONFIG)/symtable.o \
		Python/$(MF_CONFIG)/sysmodule.o \
		Python/$(MF_CONFIG)/traceback.o \
		Python/$(MF_CONFIG)/getopt.o \
		Python/$(MF_CONFIG)/pystrcmp.o \
		Python/$(MF_CONFIG)/pystrtod.o \
		Python/$(MF_CONFIG)/formatter_unicode.o \
		Python/$(MF_CONFIG)/formatter_string.o \
		Python/$(MF_CONFIG)/$(DYNLOADFILE) \
		$(LIBOBJS) \
		$(MACHDEP_OBJS) \
		$(THREADOBJ)


##########################################################################
# Objects
OBJECT_OBJS=	\
		Objects/$(MF_CONFIG)/abstract.o \
		Objects/$(MF_CONFIG)/boolobject.o \
		Objects/$(MF_CONFIG)/bufferobject.o \
		Objects/$(MF_CONFIG)/bytes_methods.o \
		Objects/$(MF_CONFIG)/bytearrayobject.o \
		Objects/$(MF_CONFIG)/cellobject.o \
		Objects/$(MF_CONFIG)/classobject.o \
		Objects/$(MF_CONFIG)/cobject.o \
		Objects/$(MF_CONFIG)/codeobject.o \
		Objects/$(MF_CONFIG)/complexobject.o \
		Objects/$(MF_CONFIG)/descrobject.o \
		Objects/$(MF_CONFIG)/enumobject.o \
		Objects/$(MF_CONFIG)/exceptions.o \
		Objects/$(MF_CONFIG)/genobject.o \
		Objects/$(MF_CONFIG)/fileobject.o \
		Objects/$(MF_CONFIG)/floatobject.o \
		Objects/$(MF_CONFIG)/frameobject.o \
		Objects/$(MF_CONFIG)/funcobject.o \
		Objects/$(MF_CONFIG)/intobject.o \
		Objects/$(MF_CONFIG)/iterobject.o \
		Objects/$(MF_CONFIG)/listobject.o \
		Objects/$(MF_CONFIG)/longobject.o \
		Objects/$(MF_CONFIG)/dictobject.o \
		Objects/$(MF_CONFIG)/methodobject.o \
		Objects/$(MF_CONFIG)/moduleobject.o \
		Objects/$(MF_CONFIG)/object.o \
		Objects/$(MF_CONFIG)/obmalloc.o \
		Objects/$(MF_CONFIG)/rangeobject.o \
		Objects/$(MF_CONFIG)/setobject.o \
		Objects/$(MF_CONFIG)/sliceobject.o \
		Objects/$(MF_CONFIG)/stringobject.o \
		Objects/$(MF_CONFIG)/structseq.o \
		Objects/$(MF_CONFIG)/tupleobject.o \
		Objects/$(MF_CONFIG)/typeobject.o \
		Objects/$(MF_CONFIG)/weakrefobject.o \
		$(UNICODE_OBJS)


##########################################################################
# objects that get linked into the Python library
LIBRARY_OBJS=	\
		Modules/$(MF_CONFIG)/getbuildinfo.o \
		$(PARSER_OBJS) \
		$(OBJECT_OBJS) \
		$(PYTHON_OBJS) \
		$(MODULE_OBJS) \
		$(SIGNAL_OBJS) \
		$(MODOBJS)

BW_OBJDIRS = Python/$(MF_CONFIG) Parser/$(MF_CONFIG) Objects/$(MF_CONFIG) Modules/$(MF_CONFIG)

#########################################################################
# Rules

# Default target
all::		$(BUILDPYTHON)
build_all:	$(BUILDPYTHON) oldsharedmods sharedmods

# Compile a binary with gcc profile guided optimization.
profile-opt:
	@echo "Building with support for profile generation:"
	$(MAKE) clean
	$(MAKE) build_all_generate_profile
	@echo "Running benchmark to generate profile data:"
	$(MAKE) profile-removal
	$(MAKE) run_profile_task
	@echo "Rebuilding with profile guided optimizations:"
	$(MAKE) clean
	$(MAKE) build_all_use_profile

build_all_generate_profile:
	$(MAKE) all CFLAGS="$(CFLAGS) -fprofile-generate" LIBS="$(LIBS) -lgcov"

run_profile_task:
	./$(BUILDPYTHON) $(PROFILE_TASK)

build_all_use_profile:
	$(MAKE) all CFLAGS="$(CFLAGS) -fprofile-use"

coverage:
	@echo "Building with support for coverage checking:"
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) -O0 -pg -fprofile-arcs -ftest-coverage" LIBS="$(LIBS) -lgcov"


# Build the interpreter
# BigWorld: Replaced original rule with a warning from BigWorld

$(BW_OBJDIRS):
	@mkdir $@

ifeq ($(BW_STANDARD_PYTHON),1)
$(BUILDPYTHON):	$(BW_OBJDIRS) Modules/$(MF_CONFIG)/python.o $(LIBRARY) $(LDLIBRARY)
		$(LINKCC) $(LDFLAGS) $(LINKFORSHARED) -o $@ \
			Modules/$(MF_CONFIG)/python.o \
			$(BLDLIBRARY) $(LIBS) $(MODLIBS) $(SYSLIBS) $(LDLAST)

else

.PHONY: $(BUILDPYTHON)

$(BUILDPYTHON):
	$(SHELL) $(srcdir)/bigworld_warning.sh

endif

platform: $(BUILDPYTHON)
	$(RUNSHARED) ./$(BUILDPYTHON) -E -c 'import sys ; from distutils.util import get_platform ; print get_platform()+"-"+sys.version[0:3]' >platform


# Build the shared modules
sharedmods: $(BUILDPYTHON)
	@case $$MAKEFLAGS in \
	*s*) $(RUNSHARED) CC='$(CC)' LDSHARED='$(BLDSHARED)' OPT='$(OPT)' ./$(BUILDPYTHON) -E $(srcdir)/setup.py -q build;; \
	*) $(RUNSHARED) CC='$(CC)' LDSHARED='$(BLDSHARED)' OPT='$(OPT)' ./$(BUILDPYTHON) -E $(srcdir)/setup.py build;; \
	esac

$(BW_PYTHONLIB): $(BW_OBJDIRS) $(LIBRARY_OBJS)
	-rm -f $@
	$(AR) cr $@ Modules/$(MF_CONFIG)/getbuildinfo.o
	$(AR) cr $@ $(PARSER_OBJS)
	$(AR) cr $@ $(OBJECT_OBJS)
	$(AR) cr $@ $(PYTHON_OBJS)
	$(AR) cr $@ $(MODULE_OBJS) $(SIGNAL_OBJS)
	$(AR) cr $@ $(MODOBJS)
	$(RANLIB) $@

# Build static library
# avoid long command lines, same as LIBRARY_OBJS
$(LIBRARY): $(LIBRARY_OBJS)
	-rm -f $@
	$(AR) cr $@ Modules/$(MF_CONFIG)/getbuildinfo.o
	$(AR) cr $@ $(PARSER_OBJS)
	$(AR) cr $@ $(OBJECT_OBJS)
	$(AR) cr $@ $(PYTHON_OBJS)
	$(AR) cr $@ $(MODULE_OBJS) $(SIGNAL_OBJS)
	$(AR) cr $@ $(MODOBJS)
	$(RANLIB) $@

libpython$(VERSION).so: $(LIBRARY_OBJS)
	if test $(INSTSONAME) != $(LDLIBRARY); then \
		$(LDSHARED) $(LDFLAGS) -Wl,-h$(INSTSONAME) -o $(INSTSONAME) $(LIBRARY_OBJS) $(MODLIBS) $(SHLIBS) $(LIBC) $(LIBM) $(LDLAST); \
		$(LN) -f $(INSTSONAME) $@; \
	else \
		$(LDSHARED) $(LDFLAGS) -o $@ $(LIBRARY_OBJS) $(MODLIBS) $(SHLIBS) $(LIBC) $(LIBM) $(LDLAST); \
	fi

libpython$(VERSION).dylib: $(LIBRARY_OBJS)
	 $(CC) -dynamiclib -Wl,-single_module $(LDFLAGS) -undefined dynamic_lookup -Wl,-install_name,$(prefix)/lib/libpython$(VERSION).dylib -Wl,-compatibility_version,$(VERSION) -Wl,-current_version,$(VERSION) -o $@ $(LIBRARY_OBJS) $(SHLIBS) $(LIBC) $(LIBM) $(LDLAST); \
		 

libpython$(VERSION).sl: $(LIBRARY_OBJS)
	$(LDSHARED) $(LDFLAGS) -o $@ $(LIBRARY_OBJS) $(MODLIBS) $(SHLIBS) $(LIBC) $(LIBM) $(LDLAST)

# This rule is here for OPENSTEP/Rhapsody/MacOSX. It builds a temporary
# minimal framework (not including the Lib directory and such) in the current
# directory.
RESSRCDIR=Mac/Resources/framework
$(PYTHONFRAMEWORKDIR)/Versions/$(VERSION)/$(PYTHONFRAMEWORK): \
		$(LIBRARY) \
		$(RESSRCDIR)/Info.plist
	$(INSTALL) -d -m $(DIRMODE) $(PYTHONFRAMEWORKDIR)/Versions/$(VERSION)
	if test "${UNIVERSALSDK}"; then \
		$(CC) -o $(LDLIBRARY)  -dynamiclib \
			-isysroot "${UNIVERSALSDK}" \
			-all_load $(LIBRARY) -Wl,-single_module \
			-install_name $(DESTDIR)$(PYTHONFRAMEWORKINSTALLDIR)/Versions/$(VERSION)/$(PYTHONFRAMEWORK) \
			-compatibility_version $(VERSION) \
			-current_version $(VERSION); \
        else \
		/usr/bin/libtool -o $(LDLIBRARY) -dynamic $(OTHER_LIBTOOL_OPT) $(LIBRARY) \
			 ;\
	fi
	$(INSTALL) -d -m $(DIRMODE)  \
		$(PYTHONFRAMEWORKDIR)/Versions/$(VERSION)/Resources/English.lproj
	$(INSTALL_DATA) $(RESSRCDIR)/Info.plist \
		$(PYTHONFRAMEWORKDIR)/Versions/$(VERSION)/Resources/Info.plist
	$(LN) -fsn $(VERSION) $(PYTHONFRAMEWORKDIR)/Versions/Current
	$(LN) -fsn Versions/Current/$(PYTHONFRAMEWORK) $(PYTHONFRAMEWORKDIR)/$(PYTHONFRAMEWORK)
	$(LN) -fsn Versions/Current/Headers $(PYTHONFRAMEWORKDIR)/Headers
	$(LN) -fsn Versions/Current/Resources $(PYTHONFRAMEWORKDIR)/Resources

# This rule builds the Cygwin Python DLL and import library if configured
# for a shared core library; otherwise, this rule is a noop.
$(DLLLIBRARY) libpython$(VERSION).dll.a: $(LIBRARY_OBJS)
	if test -n "$(DLLLIBRARY)"; then \
		$(LDSHARED) $(LDFLAGS) -Wl,--out-implib=$@ -o $(DLLLIBRARY) $^ \
			$(LIBS) $(MODLIBS) $(SYSLIBS) $(LDLAST); \
	else true; \
	fi


oldsharedmods: $(SHAREDMODS)


###########################################################################
# BigWorld: Do not convert template into Makefile, use Makefile already
# in svn

#Makefile Modules/config.c: Makefile.pre \
#				$(srcdir)/Modules/config.c.in \
#				$(MAKESETUP) \
#				Modules/Setup.config \
#				Modules/Setup \
#				Modules/Setup.local
#	$(SHELL) $(MAKESETUP) -c $(srcdir)/Modules/config.c.in \
#				-s Modules \
#				Modules/Setup.config \
#				Modules/Setup.local \
#				Modules/Setup
#	@mv config.c Modules
#	@echo "The Makefile was updated, you may need to re-run make."


Modules/Setup: $(srcdir)/Modules/Setup.dist
	@if test -f Modules/Setup; then \
		echo "-----------------------------------------------"; \
		echo "Modules/Setup.dist is newer than Modules/Setup;"; \
		echo "check to make sure you have all the updates you"; \
		echo "need in your Modules/Setup file."; \
		echo "Usually, copying Modules/Setup.dist to Modules/Setup will work."; \
		echo "-----------------------------------------------"; \
	fi

############################################################################
# Special rules for object files

Modules/$(MF_CONFIG)/getbuildinfo.o: $(PARSER_OBJS) \
		$(OBJECT_OBJS) \
		$(PYTHON_OBJS) \
		$(MODULE_OBJS) \
		$(SIGNAL_OBJS) \
		$(MODOBJS) \
		$(srcdir)/Modules/getbuildinfo.c
	$(CC) -c $(PY_CFLAGS) -DSVNVERSION="\"`LC_ALL=C $(SVNVERSION)`\"" -o $@ $(srcdir)/Modules/getbuildinfo.c

Modules/$(MF_CONFIG)/getpath.o: $(srcdir)/Modules/getpath.c Makefile
	$(CC) -c $(PY_CFLAGS) -DPYTHONPATH='"$(PYTHONPATH)"' \
		-DPREFIX='"$(prefix)"' \
		-DEXEC_PREFIX='"$(exec_prefix)"' \
		-DVERSION='"$(VERSION)"' \
		-DVPATH='"$(VPATH)"' \
		-o $@ $(srcdir)/Modules/getpath.c

Modules/$(MF_CONFIG)/python.o: $(srcdir)/Modules/python.c
	$(MAINCC) -c $(PY_CFLAGS) -o $@ $(srcdir)/Modules/python.c

# BigWorld: generation not required, this is distributed.
#$(GRAMMAR_H) $(GRAMMAR_C): $(PGEN) $(GRAMMAR_INPUT)
#		-@$(INSTALL) -d Include
#		-$(PGEN) $(GRAMMAR_INPUT) $(GRAMMAR_H) $(GRAMMAR_C)

$(PGEN):	$(PGENOBJS)
		$(CC) $(OPT) $(LDFLAGS) $(PGENOBJS) $(LIBS) -o $(PGEN)

Parser/$(MF_CONFIG)/grammar.o:	$(srcdir)/Parser/grammar.c \
				$(srcdir)/Include/token.h \
				$(srcdir)/Include/grammar.h
Parser/$(MF_CONFIG)/metagrammar.o:	$(srcdir)/Parser/metagrammar.c

Parser/$(MF_CONFIG)/tokenizer_pgen.o:	$(srcdir)/Parser/tokenizer.c

Parser/$(MF_CONFIG)/pgenmain.o:	$(srcdir)/Include/parsetok.h

############################################################################
# BigWorld: Do not generate C code from ASDL description, use C code
# already in svn

#$(AST_H): $(AST_ASDL) $(ASDLGEN_FILES)
#	$(ASDLGEN) -h $(AST_H_DIR) $(AST_ASDL)

#$(AST_C): $(AST_ASDL) $(ASDLGEN_FILES)
#	$(ASDLGEN) -c $(AST_C_DIR) $(AST_ASDL)

Python/$(MF_CONFIG)/compile.o Python/$(MF_CONFIG)/symtable.o: $(GRAMMAR_H) $(AST_H)

Python/$(MF_CONFIG)/getplatform.o: $(srcdir)/Python/getplatform.c
		$(CC) -c $(PY_CFLAGS) -DPLATFORM='"$(MACHDEP)"' -o $@ $(srcdir)/Python/getplatform.c

Python/$(MF_CONFIG)/importdl.o: $(srcdir)/Python/importdl.c
		$(CC) -c $(PY_CFLAGS) -I$(DLINCLDIR) -o $@ $(srcdir)/Python/importdl.c

Objects/$(MF_CONFIG)/unicodectype.o:	$(srcdir)/Objects/unicodectype.c \
				$(srcdir)/Objects/unicodetype_db.h

STRINGLIB_HEADERS= \
		$(srcdir)/Include/bytes_methods.h \
		$(srcdir)/Objects/stringlib/count.h \
		$(srcdir)/Objects/stringlib/ctype.h \
		$(srcdir)/Objects/stringlib/fastsearch.h \
		$(srcdir)/Objects/stringlib/find.h \
		$(srcdir)/Objects/stringlib/formatter.h \
		$(srcdir)/Objects/stringlib/partition.h \
		$(srcdir)/Objects/stringlib/stringdefs.h \
		$(srcdir)/Objects/stringlib/string_format.h \
		$(srcdir)/Objects/stringlib/transmogrify.h \
		$(srcdir)/Objects/stringlib/unicodedefs.h \
		$(srcdir)/Objects/stringlib/localeutil.h

Objects/$(MF_CONFIG)/unicodeobject.o: $(srcdir)/Objects/unicodeobject.c \
				$(STRINGLIB_HEADERS)

Objects/$(MF_CONFIG)/bytearrayobject.o: $(srcdir)/Objects/bytearrayobject.c \
				$(STRINGLIB_HEADERS)

Objects/$(MF_CONFIG)/stringobject.o: $(srcdir)/Objects/stringobject.c \
				$(STRINGLIB_HEADERS)

Python/$(MF_CONFIG)/formatter_unicode.o: $(srcdir)/Python/formatter_unicode.c \
				$(STRINGLIB_HEADERS)

Python/$(MF_CONFIG)/formatter_string.o: $(srcdir)/Python/formatter_string.c \
				$(STRINGLIB_HEADERS)

############################################################################
# Header files

PYTHON_HEADERS= \
		Include/Python-ast.h \
		Include/Python.h \
		Include/abstract.h \
		Include/asdl.h \
		Include/ast.h \
		Include/bitset.h \
		Include/boolobject.h \
		Include/bytearrayobject.h \
		Include/bytes_methods.h \
		Include/bytesobject.h \
		Include/bufferobject.h \
		Include/cellobject.h \
		Include/ceval.h \
		Include/classobject.h \
		Include/cobject.h \
		Include/code.h \
		Include/codecs.h \
		Include/compile.h \
		Include/complexobject.h \
		Include/descrobject.h \
		Include/dictobject.h \
		Include/enumobject.h \
		Include/errcode.h \
		Include/eval.h \
		Include/fileobject.h \
		Include/floatobject.h \
		Include/frameobject.h \
		Include/funcobject.h \
		Include/genobject.h \
		Include/import.h \
		Include/intobject.h \
		Include/intrcheck.h \
		Include/iterobject.h \
		Include/listobject.h \
		Include/longintrepr.h \
		Include/longobject.h \
		Include/marshal.h \
		Include/metagrammar.h \
		Include/methodobject.h \
		Include/modsupport.h \
		Include/moduleobject.h \
		Include/node.h \
		Include/object.h \
		Include/objimpl.h \
		Include/opcode.h \
		Include/osdefs.h \
		Include/parsetok.h \
		Include/patchlevel.h \
		Include/pgen.h \
		Include/pgenheaders.h \
		Include/pyarena.h \
		Include/pydebug.h \
		Include/pyerrors.h \
		Include/pyfpe.h \
		Include/pymath.h \
		Include/pygetopt.h \
		Include/pymem.h \
		Include/pyport.h \
		Include/pystate.h \
		Include/pystrcmp.h \
		Include/pystrtod.h \
		Include/pythonrun.h \
		Include/pythread.h \
		Include/rangeobject.h \
		Include/setobject.h \
		Include/sliceobject.h \
		Include/stringobject.h \
		Include/structmember.h \
		Include/structseq.h \
		Include/symtable.h \
		Include/sysmodule.h \
		Include/traceback.h \
		Include/tupleobject.h \
		Include/ucnhash.h \
		Include/unicodeobject.h \
		Include/warnings.h \
		Include/weakrefobject.h \
		pyconfig.h \
		$(PARSER_HEADERS)

$(LIBRARY_OBJS) $(MODOBJS) Modules/$(MF_CONFIG)/python.o: $(PYTHON_HEADERS)


######################################################################

# Test the interpreter (twice, once without .pyc files, once with)
# In the past, we've had problems where bugs in the marshalling or
# elsewhere caused bytecode read from .pyc files to behave differently
# than bytecode generated directly from a .py source file.  Sometimes
# the bytecode read from a .pyc file had the bug, somtimes the directly
# generated bytecode.  This is sometimes a very shy bug needing a lot of
# sample data.

TESTOPTS=	-l $(EXTRATESTOPTS)
TESTPROG=	$(srcdir)/Lib/test/regrtest.py
TESTPYTHON=	$(RUNSHARED) ./$(BUILDPYTHON) -E -tt
test:		all platform
		-find $(srcdir)/Lib -name '*.py[co]' -print | xargs rm -f
		-$(TESTPYTHON) $(TESTPROG) $(TESTOPTS)
		$(TESTPYTHON) $(TESTPROG) $(TESTOPTS)

testall:	all platform
		-find $(srcdir)/Lib -name '*.py[co]' -print | xargs rm -f
		$(TESTPYTHON) $(srcdir)/Lib/compileall.py
		-find $(srcdir)/Lib -name '*.py[co]' -print | xargs rm -f
		-$(TESTPYTHON) $(TESTPROG) $(TESTOPTS) -uall
		$(TESTPYTHON) $(TESTPROG) $(TESTOPTS) -uall

#  Run the unitests for both architectures in a Universal build on OSX
#  Must be run on an Intel box.
testuniversal:	all platform
		if [ `arch` != 'i386' ];then \
			echo "This can only be used on OSX/i386" ;\
			exit 1 ;\
		fi
		-find $(srcdir)/Lib -name '*.py[co]' -print | xargs rm -f
		-$(TESTPYTHON) $(TESTPROG) $(TESTOPTS) -uall
		$(TESTPYTHON) $(TESTPROG) $(TESTOPTS) -uall
		$(RUNSHARED) /usr/libexec/oah/translate ./$(BUILDPYTHON) -E -tt $(TESTPROG) $(TESTOPTS) -uall


# Like testall, but with a single pass only
# run an optional script to include some information about the build environment
buildbottest:	all platform
		-@if which pybuildbot.identify >/dev/null 2>&1; then \
			pybuildbot.identify "CC='$(CC)'" "CXX='$(CXX)'"; \
		fi
		$(TESTPYTHON) $(TESTPROG) $(TESTOPTS) -uall -rw

QUICKTESTOPTS=	$(TESTOPTS) -x test_thread test_signal test_strftime \
		test_unicodedata test_re test_sre test_select test_poll \
		test_linuxaudiodev test_struct test_sunaudiodev test_zlib
quicktest:	all platform
		-find $(srcdir)/Lib -name '*.py[co]' -print | xargs rm -f
		-$(TESTPYTHON) $(TESTPROG) $(QUICKTESTOPTS)
		$(TESTPYTHON) $(TESTPROG) $(QUICKTESTOPTS)

MEMTESTOPTS=    $(QUICKTESTOPTS) -x test_dl test___all__ test_fork1 \
		test_longexp
memtest:	all platform
		-rm -f $(srcdir)/Lib/test/*.py[co]
		-$(TESTPYTHON) $(TESTPROG) $(MEMTESTOPTS)
		$(TESTPYTHON) $(TESTPROG) $(MEMTESTOPTS)

# Install everything
install::	 altinstall bininstall maninstall 

# Install almost everything without disturbing previous versions
altinstall:	 altbininstall libinstall inclinstall libainstall \
                sharedinstall oldsharedinstall 

# Install shared libraries enabled by Setup
DESTDIRS=	$(exec_prefix) $(LIBDIR) $(BINLIBDEST) $(DESTSHARED)

oldsharedinstall: $(DESTSHARED) $(SHAREDMODS)
		@for i in X $(SHAREDMODS); do \
		  if test $$i != X; then \
		    echo $(INSTALL_SHARED) $$i $(DESTSHARED)/`basename $$i`; \
		    $(INSTALL_SHARED) $$i $(DESTDIR)$(DESTSHARED)/`basename $$i`; \
		  fi; \
		done

$(DESTSHARED):
		@for i in $(DESTDIRS); \
		do \
			if test ! -d $(DESTDIR)$$i; then \
				echo "Creating directory $$i"; \
				$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
			else    true; \
			fi; \
		done


# Install the interpreter (by creating a hard link to python$(VERSION))
bininstall:	altbininstall
	-if test -f $(DESTDIR)$(BINDIR)/$(PYTHON) -o -h $(DESTDIR)$(BINDIR)/$(PYTHON); \
	then rm -f $(DESTDIR)$(BINDIR)/$(PYTHON); \
	else true; \
	fi
	(cd $(DESTDIR)$(BINDIR); $(LN) python$(VERSION)$(EXE) $(PYTHON))
	-rm -f $(DESTDIR)$(BINDIR)/python-config
	(cd $(DESTDIR)$(BINDIR); $(LN) -s python$(VERSION)-config python-config)

# Install the interpreter with $(VERSION) affixed
# This goes into $(exec_prefix)
altbininstall:	$(BUILDPYTHON)
	@for i in $(BINDIR) $(LIBDIR); \
	do \
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	$(INSTALL_PROGRAM) $(BUILDPYTHON) $(DESTDIR)$(BINDIR)/python$(VERSION)$(EXE)
	if test -f $(LDLIBRARY); then \
		if test -n "$(DLLLIBRARY)" ; then \
			$(INSTALL_SHARED) $(DLLLIBRARY) $(DESTDIR)$(BINDIR); \
		else \
			$(INSTALL_SHARED) $(LDLIBRARY) $(DESTDIR)$(LIBDIR)/$(INSTSONAME); \
			if test $(LDLIBRARY) != $(INSTSONAME); then \
				(cd $(DESTDIR)$(LIBDIR); $(LN) -sf $(INSTSONAME) $(LDLIBRARY)) \
			fi \
		fi; \
	else	true; \
	fi

# Install the manual page
maninstall:
	@for i in $(MANDIR) $(MANDIR)/man1; \
	do \
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	$(INSTALL_DATA) $(srcdir)/Misc/python.man \
		$(DESTDIR)$(MANDIR)/man1/python.1

# Install the library
PLATDIR=	plat-$(MACHDEP)
EXTRAPLATDIR= 
EXTRAMACHDEPPATH=
MACHDEPS=	$(PLATDIR) $(EXTRAPLATDIR)
XMLLIBSUBDIRS=  xml xml/dom xml/etree xml/parsers xml/sax
PLATMACDIRS= plat-mac plat-mac/Carbon plat-mac/lib-scriptpackages \
	plat-mac/lib-scriptpackages/_builtinSuites \
	plat-mac/lib-scriptpackages/CodeWarrior \
	plat-mac/lib-scriptpackages/Explorer \
	plat-mac/lib-scriptpackages/Finder \
	plat-mac/lib-scriptpackages/Netscape \
	plat-mac/lib-scriptpackages/StdSuites \
	plat-mac/lib-scriptpackages/SystemEvents \
	plat-mac/lib-scriptpackages/Terminal
PLATMACPATH=:plat-mac:plat-mac/lib-scriptpackages
LIBSUBDIRS=	lib-tk site-packages test test/output test/data \
		test/decimaltestdata \
		encodings compiler hotshot \
		email email/mime email/test email/test/data \
		json json/tests \
		sqlite3 sqlite3/test \
		logging bsddb bsddb/test csv wsgiref \
		lib2to3 lib2to3/fixes lib2to3/pgen2 lib2to3/tests \
		ctypes ctypes/test ctypes/macholib idlelib idlelib/Icons \
		distutils distutils/command distutils/tests $(XMLLIBSUBDIRS) \
		multiprocessing multiprocessing/dummy \
		lib-old \
		curses $(MACHDEPS)
libinstall:	build_all $(srcdir)/Lib/$(PLATDIR)
	@for i in $(SCRIPTDIR) $(LIBDEST); \
	do \
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	@for d in $(LIBSUBDIRS); \
	do \
		a=$(srcdir)/Lib/$$d; \
		if test ! -d $$a; then continue; else true; fi; \
		b=$(LIBDEST)/$$d; \
		if test ! -d $(DESTDIR)$$b; then \
			echo "Creating directory $$b"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$b; \
		else	true; \
		fi; \
	done
	@for i in $(srcdir)/Lib/*.py $(srcdir)/Lib/*.doc $(srcdir)/Lib/*.egg-info ; \
	do \
		if test -x $$i; then \
			$(INSTALL_SCRIPT) $$i $(DESTDIR)$(LIBDEST); \
			echo $(INSTALL_SCRIPT) $$i $(LIBDEST); \
		else \
			$(INSTALL_DATA) $$i $(DESTDIR)$(LIBDEST); \
			echo $(INSTALL_DATA) $$i $(LIBDEST); \
		fi; \
	done
	@for d in $(LIBSUBDIRS); \
	do \
		a=$(srcdir)/Lib/$$d; \
		if test ! -d $$a; then continue; else true; fi; \
		if test `ls $$a | wc -l` -lt 1; then continue; fi; \
		b=$(LIBDEST)/$$d; \
		for i in $$a/*; \
		do \
			case $$i in \
			*CVS) ;; \
			*.py[co]) ;; \
			*.orig) ;; \
			*~) ;; \
			*) \
				if test -d $$i; then continue; fi; \
				if test -x $$i; then \
				    echo $(INSTALL_SCRIPT) $$i $$b; \
				    $(INSTALL_SCRIPT) $$i $(DESTDIR)$$b; \
				else \
				    echo $(INSTALL_DATA) $$i $$b; \
				    $(INSTALL_DATA) $$i $(DESTDIR)$$b; \
				fi;; \
			esac; \
		done; \
	done
	$(INSTALL_DATA) $(srcdir)/LICENSE $(DESTDIR)$(LIBDEST)/LICENSE.txt
	PYTHONPATH=$(DESTDIR)$(LIBDEST)  $(RUNSHARED) \
		./$(BUILDPYTHON) -Wi -tt $(DESTDIR)$(LIBDEST)/compileall.py \
		-d $(LIBDEST) -f \
		-x 'bad_coding|badsyntax|site-packages' $(DESTDIR)$(LIBDEST)
	PYTHONPATH=$(DESTDIR)$(LIBDEST) $(RUNSHARED) \
		./$(BUILDPYTHON) -Wi -tt -O $(DESTDIR)$(LIBDEST)/compileall.py \
		-d $(LIBDEST) -f \
		-x 'bad_coding|badsyntax|site-packages' $(DESTDIR)$(LIBDEST)
	-PYTHONPATH=$(DESTDIR)$(LIBDEST)  $(RUNSHARED) \
		./$(BUILDPYTHON) -Wi -t $(DESTDIR)$(LIBDEST)/compileall.py \
		-d $(LIBDEST)/site-packages -f \
		-x badsyntax $(DESTDIR)$(LIBDEST)/site-packages
	-PYTHONPATH=$(DESTDIR)$(LIBDEST) $(RUNSHARED) \
		./$(BUILDPYTHON) -Wi -t -O $(DESTDIR)$(LIBDEST)/compileall.py \
		-d $(LIBDEST)/site-packages -f \
		-x badsyntax $(DESTDIR)$(LIBDEST)/site-packages
	-PYTHONPATH=$(DESTDIR)$(LIBDEST) $(RUNSHARED) \
		./$(BUILDPYTHON) -Wi -t -c "import lib2to3.pygram, lib2to3.patcomp;lib2to3.patcomp.PatternCompiler()"

# Create the PLATDIR source directory, if one wasn't distributed..
$(srcdir)/Lib/$(PLATDIR):
	mkdir $(srcdir)/Lib/$(PLATDIR)
	cp $(srcdir)/Lib/plat-generic/regen $(srcdir)/Lib/$(PLATDIR)/regen
	export PATH; PATH="`pwd`:$$PATH"; \
	export PYTHONPATH; PYTHONPATH="`pwd`/Lib"; \
	export DYLD_FRAMEWORK_PATH; DYLD_FRAMEWORK_PATH="`pwd`"; \
	export EXE; EXE="$(BUILDEXE)"; \
	cd $(srcdir)/Lib/$(PLATDIR); $(RUNSHARED) ./regen

# Install the include files
INCLDIRSTOMAKE=$(INCLUDEDIR) $(CONFINCLUDEDIR) $(INCLUDEPY) $(CONFINCLUDEPY)
inclinstall:
	@for i in $(INCLDIRSTOMAKE); \
	do \
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	@for i in $(srcdir)/Include/*.h; \
	do \
		echo $(INSTALL_DATA) $$i $(INCLUDEPY); \
		$(INSTALL_DATA) $$i $(DESTDIR)$(INCLUDEPY); \
	done
	$(INSTALL_DATA) pyconfig.h $(DESTDIR)$(CONFINCLUDEPY)/pyconfig.h

# Install the library and miscellaneous stuff needed for extending/embedding
# This goes into $(exec_prefix)
LIBPL=		$(LIBP)/config
libainstall:	all
	@for i in $(LIBDIR) $(LIBP) $(LIBPL); \
	do \
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	@if test -d $(LIBRARY); then :; else \
		if test "$(PYTHONFRAMEWORKDIR)" = no-framework; then \
			if test "$(SO)" = .dll; then \
				$(INSTALL_DATA) $(LDLIBRARY) $(DESTDIR)$(LIBPL) ; \
			else \
				$(INSTALL_DATA) $(LIBRARY) $(DESTDIR)$(LIBPL)/$(LIBRARY) ; \
				$(RANLIB) $(DESTDIR)$(LIBPL)/$(LIBRARY) ; \
			fi; \
		else \
			echo Skip install of $(LIBRARY) - use make frameworkinstall; \
		fi; \
	fi
	$(INSTALL_DATA) Modules/config.c $(DESTDIR)$(LIBPL)/config.c
	$(INSTALL_DATA) Modules/python.o $(DESTDIR)$(LIBPL)/python.o
	$(INSTALL_DATA) $(srcdir)/Modules/config.c.in $(DESTDIR)$(LIBPL)/config.c.in
	$(INSTALL_DATA) Makefile $(DESTDIR)$(LIBPL)/Makefile
	$(INSTALL_DATA) Modules/Setup $(DESTDIR)$(LIBPL)/Setup
	$(INSTALL_DATA) Modules/Setup.local $(DESTDIR)$(LIBPL)/Setup.local
	$(INSTALL_DATA) Modules/Setup.config $(DESTDIR)$(LIBPL)/Setup.config
	$(INSTALL_SCRIPT) $(srcdir)/Modules/makesetup $(DESTDIR)$(LIBPL)/makesetup
	$(INSTALL_SCRIPT) $(srcdir)/install-sh $(DESTDIR)$(LIBPL)/install-sh
	# Substitution happens here, as the completely-expanded BINDIR
	# is not available in configure
	sed -e "s,@EXENAME@,$(BINDIR)/python$(VERSION)$(EXE)," < $(srcdir)/Misc/python-config.in >python-config
	$(INSTALL_SCRIPT) python-config $(DESTDIR)$(BINDIR)/python$(VERSION)-config
	rm python-config
	@if [ -s Modules/python.exp -a \
		"`echo $(MACHDEP) | sed 's/^\(...\).*/\1/'`" = "aix" ]; then \
		echo; echo "Installing support files for building shared extension modules on AIX:"; \
		$(INSTALL_DATA) Modules/python.exp		\
				$(DESTDIR)$(LIBPL)/python.exp;		\
		echo; echo "$(LIBPL)/python.exp";		\
		$(INSTALL_SCRIPT) $(srcdir)/Modules/makexp_aix	\
				$(DESTDIR)$(LIBPL)/makexp_aix;		\
		echo "$(LIBPL)/makexp_aix";			\
		$(INSTALL_SCRIPT) $(srcdir)/Modules/ld_so_aix	\
				$(DESTDIR)$(LIBPL)/ld_so_aix;		\
		echo "$(LIBPL)/ld_so_aix";			\
		echo; echo "See Misc/AIX-NOTES for details.";	\
	else true; \
	fi
	@case "$(MACHDEP)" in beos*) \
		echo; echo "Installing support files for building shared extension modules on BeOS:"; \
		$(INSTALL_DATA) Misc/BeOS-NOTES $(DESTDIR)$(LIBPL)/README;	\
		echo; echo "$(LIBPL)/README";			\
		$(INSTALL_SCRIPT) Modules/ar_beos $(DESTDIR)$(LIBPL)/ar_beos; \
		echo "$(LIBPL)/ar_beos";			\
		$(INSTALL_SCRIPT) Modules/ld_so_beos $(DESTDIR)$(LIBPL)/ld_so_beos; \
		echo "$(LIBPL)/ld_so_beos";			\
		echo; echo "See Misc/BeOS-NOTES for details.";	\
		;; \
	esac

# Install the dynamically loadable modules
# This goes into $(exec_prefix)
sharedinstall:
	$(RUNSHARED) ./$(BUILDPYTHON) -E $(srcdir)/setup.py install \
	   	--prefix=$(prefix) \
		--install-scripts=$(BINDIR) \
		--install-platlib=$(DESTSHARED) \
		--root=/$(DESTDIR)

# Here are a couple of targets for MacOSX again, to install a full
# framework-based Python. frameworkinstall installs everything, the
# subtargets install specific parts. Much of the actual work is offloaded to
# the Makefile in Mac
#
#
# This target is here for backward compatiblity, previous versions of Python
# hadn't integrated framework installation in the normal install process.
frameworkinstall: install

# On install, we re-make the framework
# structure in the install location, /Library/Frameworks/ or the argument to
# --enable-framework. If --enable-framework has been specified then we have
# automatically set prefix to the location deep down in the framework, so we
# only have to cater for the structural bits of the framework.

frameworkinstallframework: frameworkinstallstructure install frameworkinstallmaclib

frameworkinstallstructure:	$(LDLIBRARY)
	@if test "$(PYTHONFRAMEWORKDIR)" = no-framework; then \
		echo Not configured with --enable-framework; \
		exit 1; \
	else true; \
	fi
	@for i in $(prefix)/Resources/English.lproj $(prefix)/lib; do\
		if test ! -d $(DESTDIR)$$i; then \
			echo "Creating directory $(DESTDIR)$$i"; \
			$(INSTALL) -d -m $(DIRMODE) $(DESTDIR)$$i; \
		else	true; \
		fi; \
	done
	$(LN) -fsn include/python$(VERSION) $(DESTDIR)$(prefix)/Headers
	sed 's/%VERSION%/'"`$(RUNSHARED) ./$(BUILDPYTHON) -c 'import platform; print platform.python_version()'`"'/g' < $(RESSRCDIR)/Info.plist > $(DESTDIR)$(prefix)/Resources/Info.plist
	$(LN) -fsn $(VERSION) $(DESTDIR)$(PYTHONFRAMEWORKINSTALLDIR)/Versions/Current
	$(LN) -fsn Versions/Current/$(PYTHONFRAMEWORK) $(DESTDIR)$(PYTHONFRAMEWORKINSTALLDIR)/$(PYTHONFRAMEWORK)
	$(LN) -fsn Versions/Current/Headers $(DESTDIR)$(PYTHONFRAMEWORKINSTALLDIR)/Headers
	$(LN) -fsn Versions/Current/Resources $(DESTDIR)$(PYTHONFRAMEWORKINSTALLDIR)/Resources
	$(INSTALL_SHARED) $(LDLIBRARY) $(DESTDIR)$(PYTHONFRAMEWORKPREFIX)/$(LDLIBRARY)

# This installs Mac/Lib into the framework
# Install a number of symlinks to keep software that expects a normal unix
# install (which includes python-config) happy.
frameworkinstallmaclib:
	ln -fs "../../../$(PYTHONFRAMEWORK)" "$(DESTDIR)$(prefix)/lib/python$(VERSION)/config/libpython$(VERSION).a"
	cd Mac && $(MAKE) installmacsubtree DESTDIR="$(DESTDIR)"

# This installs the IDE, the Launcher and other apps into /Applications
frameworkinstallapps:
	cd Mac && $(MAKE) installapps DESTDIR="$(DESTDIR)"

frameworkinstallapps4way:
	cd Mac && $(MAKE) installapps4way DESTDIR="$(DESTDIR)"

# This install the unix python and pythonw tools in /usr/local/bin
frameworkinstallunixtools:
	cd Mac && $(MAKE) installunixtools DESTDIR="$(DESTDIR)"

frameworkinstallunixtools4way:
	cd Mac && $(MAKE) installunixtools4way DESTDIR="$(DESTDIR)"

frameworkaltinstallunixtools:
	cd Mac && $(MAKE) altinstallunixtools DESTDIR="$(DESTDIR)"

frameworkaltinstallunixtools4way:
	cd Mac && $(MAKE) altinstallunixtools4way DESTDIR="$(DESTDIR)"

# This installs the Demos and Tools into the applications directory.
# It is not part of a normal frameworkinstall
frameworkinstallextras:
	cd Mac && $(MAKE) installextras DESTDIR="$(DESTDIR)"

# This installs a few of the useful scripts in Tools/scripts
scriptsinstall:
	SRCDIR=$(srcdir) $(RUNSHARED) \
	./$(BUILDPYTHON) $(srcdir)/Tools/scripts/setup.py install \
	--prefix=$(prefix) \
	--install-scripts=$(BINDIR) \
	--root=/$(DESTDIR)

# Build the toplevel Makefile
Makefile.pre: Makefile.pre.in config.status
	CONFIG_FILES=Makefile.pre CONFIG_HEADERS= $(SHELL) config.status
	$(MAKE) -f Makefile.pre Makefile

# Run the configure script.
config.status:	$(srcdir)/configure
	$(SHELL) $(srcdir)/configure $(CONFIG_ARGS)

.PRECIOUS: config.status $(BUILDPYTHON) Makefile Makefile.pre

### BigWorld: Added a rule to place the .o files in an isolated directory
Parser/$(MF_CONFIG)/%.o: Parser/%.c
	$(CC) -c $(PY_CFLAGS) -o $@ $<

Objects/$(MF_CONFIG)/%.o: Objects/%.c
	$(CC) -c $(PY_CFLAGS) -o $@ $<

Modules/$(MF_CONFIG)/%.o: Modules/%.c
	$(CC) -c $(PY_CFLAGS) -o $@ $<

Python/$(MF_CONFIG)/%.o: Python/%.c
	$(CC) -c $(PY_CFLAGS) -o $@ $<

# BigWorld: commented out original object file rule to implement Hybrid* dirs
# Some make's put the object file in the current directory
#.c.o:
#	$(CC) -c $(PY_CFLAGS) -o $@ $<

# Run reindent on the library
reindent:
	./$(BUILDPYTHON) $(srcdir)/Tools/scripts/reindent.py -r $(srcdir)/Lib

# Rerun configure with the same options as it was run last time,
# provided the config.status script exists
recheck:
	$(SHELL) config.status --recheck
	$(SHELL) config.status

# Rebuild the configure script from configure.in; also rebuild pyconfig.h.in
autoconf:
	(cd $(srcdir); autoconf)
	(cd $(srcdir); autoheader)

# Create a tags file for vi
tags::
	cd $(srcdir); \
	ctags -w -t Include/*.h; \
	for i in $(SRCDIRS); do ctags -w -t -a $$i/*.[ch]; \
	done; \
	sort -o tags tags

# Create a tags file for GNU Emacs
TAGS::
	cd $(srcdir); \
	etags Include/*.h; \
	for i in $(SRCDIRS); do etags -a $$i/*.[ch]; done

# Sanitation targets -- clean leaves libraries, executables and tags
# files, which clobber removes as well
pycremoval:
	find $(srcdir) -name '*.py[co]' -exec rm -f {} ';'

# BigWorld: Modified .o finding for new locations
clean:: pycremoval
	find . -wholename '*/$(MF_CONFIG)*/*.o' -exec rm -f {} ';'
	find . -wholename '*/$(MF_CONFIG)*/*.s[ol]' -exec rm -f {} ';'
	find $(srcdir)/build -name 'fficonfig.h' -exec rm -f {} ';' || true
	find $(srcdir)/build -name 'fficonfig.py' -exec rm -f {} ';' || true
	-rm -f Lib/lib2to3/*Grammar*.pickle
	-rm -f $(BWLIBDIR)*/lib$(PYTHONLIB).a
	-rm -f $(LIBRARY)
	-rm -f $(BUILDPYTHON)
	-rm -rf build

profile-removal:
	find . -name '*.gc??' -exec rm -f {} ';'

clobber: clean profile-removal
	-rm -f $(BUILDPYTHON) $(PGEN) $(LIBRARY) $(LDLIBRARY) $(DLLLIBRARY) \
		tags TAGS \
		config.cache config.log pyconfig.h Modules/config.c
	-rm -rf build platform
	-rm -rf $(PYTHONFRAMEWORKDIR)

# Make things extra clean, before making a distribution:
# remove all generated files, even Makefile[.pre]
# Keep configure and Python-ast.[ch], it's possible they can't be generated
distclean: clobber
	-rm -f core Makefile Makefile.pre config.status \
		Modules/Setup Modules/Setup.local Modules/Setup.config
	find $(srcdir) '(' -name '*.fdc' -o -name '*~' \
			   -o -name '[@,#]*' -o -name '*.old' \
			   -o -name '*.orig' -o -name '*.rej' \
			   -o -name '*.bak' ')' \
			   -exec rm -f {} ';'

# Check for smelly exported symbols (not starting with Py/_Py)
smelly: all
	nm -p $(LIBRARY) | \
		sed -n "/ [TDB] /s/.* //p" | grep -v "^_*Py" | sort -u; \

# Find files with funny names
funny:
	find $(DISTDIRS) -type d \
		-o -name '*.[chs]' \
		-o -name '*.py' \
		-o -name '*.doc' \
		-o -name '*.sty' \
		-o -name '*.bib' \
		-o -name '*.dat' \
		-o -name '*.el' \
		-o -name '*.fd' \
		-o -name '*.in' \
		-o -name '*.tex' \
		-o -name '*,[vpt]' \
		-o -name 'Setup' \
		-o -name 'Setup.*' \
		-o -name README \
		-o -name Makefile \
		-o -name ChangeLog \
		-o -name Repository \
		-o -name Root \
		-o -name Entries \
		-o -name Tag \
		-o -name tags \
		-o -name TAGS \
		-o -name .cvsignore \
		-o -name MANIFEST \
		-o -print

# Perform some verification checks on any modified files.
patchcheck:
	$(RUNSHARED) ./$(BUILDPYTHON) $(srcdir)/Tools/scripts/patchcheck.py

# Dependencies

Python/$(MF_CONFIG)/thread.o:  $(srcdir)/Python/thread_atheos.h $(srcdir)/Python/thread_beos.h $(srcdir)/Python/thread_cthread.h $(srcdir)/Python/thread_foobar.h $(srcdir)/Python/thread_lwp.h $(srcdir)/Python/thread_nt.h $(srcdir)/Python/thread_os2.h $(srcdir)/Python/thread_pth.h $(srcdir)/Python/thread_pthread.h $(srcdir)/Python/thread_sgi.h $(srcdir)/Python/thread_solaris.h $(srcdir)/Python/thread_wince.h

# Declare targets that aren't real files
.PHONY: all build_all sharedmods oldsharedmods test quicktest memtest
.PHONY: install altinstall oldsharedinstall bininstall altbininstall
.PHONY: maninstall libinstall inclinstall libainstall sharedinstall
.PHONY: frameworkinstall frameworkinstallframework frameworkinstallstructure
.PHONY: frameworkinstallmaclib frameworkinstallapps frameworkinstallunixtools
.PHONY: frameworkaltinstallunixtools recheck autoconf clean clobber distclean
.PHONY: smelly funny patchcheck

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

# Rules appended by makedepend

Modules/$(MF_CONFIG)/threadmodule.o: $(srcdir)/Modules/threadmodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/threadmodule.c -o Modules/$(MF_CONFIG)/threadmodule.o
Modules/$(MF_CONFIG)/threadmodule$(SO):  Modules/$(MF_CONFIG)/threadmodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/$(MF_CONFIG)/threadmodule.o   -o Modules/$(MF_CONFIG)/threadmodule$(SO)
Modules/$(MF_CONFIG)/signalmodule.o: $(srcdir)/Modules/signalmodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/signalmodule.c -o Modules/$(MF_CONFIG)/signalmodule.o
Modules/$(MF_CONFIG)/signalmodule$(SO):  Modules/$(MF_CONFIG)/signalmodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/$(MF_CONFIG)/signalmodule.o   -o Modules/$(MF_CONFIG)/signalmodule$(SO)
Modules/$(MF_CONFIG)/posixmodule.o: $(srcdir)/Modules/posixmodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/posixmodule.c -o Modules/$(MF_CONFIG)/posixmodule.o
Modules/$(MF_CONFIG)/posixmodule$(SO):  Modules/$(MF_CONFIG)/posixmodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/posixmodule.o   -o Modules/$(MF_CONFIG)/posixmodule$(SO)
Modules/$(MF_CONFIG)/errnomodule.o: $(srcdir)/Modules/errnomodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/errnomodule.c -o Modules/$(MF_CONFIG)/errnomodule.o
Modules/$(MF_CONFIG)/errnomodule$(SO):  Modules/$(MF_CONFIG)/errnomodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/errnomodule.o   -o Modules/$(MF_CONFIG)/errnomodule$(SO)
Modules/$(MF_CONFIG)/pwdmodule.o: $(srcdir)/Modules/pwdmodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/pwdmodule.c -o Modules/$(MF_CONFIG)/pwdmodule.o
Modules/$(MF_CONFIG)/pwdmodule$(SO):  Modules/$(MF_CONFIG)/pwdmodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/pwdmodule.o   -o Modules/$(MF_CONFIG)/pwdmodule$(SO)
Modules/$(MF_CONFIG)/_sre.o: $(srcdir)/Modules/_sre.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/_sre.c -o Modules/$(MF_CONFIG)/_sre.o
Modules/$(MF_CONFIG)/_sre$(SO):  Modules/$(MF_CONFIG)/_sre.o; $(BLDSHARED)  Modules/_sre.o   -o Modules/_sre$(SO)
Modules/$(MF_CONFIG)/_codecsmodule.o: $(srcdir)/Modules/_codecsmodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/_codecsmodule.c -o Modules/$(MF_CONFIG)/_codecsmodule.o
Modules/$(MF_CONFIG)/_codecsmodule$(SO):  Modules/$(MF_CONFIG)/_codecsmodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/_codecsmodule.o   -o Modules/$(MF_CONFIG)/_codecsmodule$(SO)
Modules/$(MF_CONFIG)/zipimport.o: $(srcdir)/Modules/zipimport.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/zipimport.c -o Modules/$(MF_CONFIG)/zipimport.o
Modules/$(MF_CONFIG)/zipimport$(SO):  Modules/$(MF_CONFIG)/zipimport.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/zipimport.o   -o Modules/$(MF_CONFIG)/zipimport$(SO)
Modules/$(MF_CONFIG)/symtablemodule.o: $(srcdir)/Modules/symtablemodule.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/symtablemodule.c -o Modules/$(MF_CONFIG)/symtablemodule.o
Modules/$(MF_CONFIG)/_symtablemodule$(SO):  Modules/$(MF_CONFIG)/symtablemodule.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/symtablemodule.o   -o Modules/$(MF_CONFIG)/_symtablemodule$(SO)
Modules/$(MF_CONFIG)/xxsubtype.o: $(srcdir)/Modules/xxsubtype.c; $(CC) $(PY_CFLAGS)  -c $(srcdir)/Modules/xxsubtype.c -o Modules/$(MF_CONFIG)/xxsubtype.o
Modules/$(MF_CONFIG)/xxsubtype$(SO):  Modules/$(MF_CONFIG)/xxsubtype.o; $(BLDSHARED)  Modules/$(MF_CONFIG)/xxsubtype.o   -o Modules/$(MF_CONFIG)/xxsubtype$(SO)
