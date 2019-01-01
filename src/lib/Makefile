
ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib,,$(CURDIR))
endif

ifndef MF_CONFIG
	MF_CONFIG=Hybrid
	ifeq ($(shell uname -m),x86_64)
		 MF_CONFIG=Hybrid64
	endif
endif

ifneq (,$(findstring 64, $(MF_CONFIG)))
	OPENSSL_CONFIG+="x86_64=1"
else
	OPENSSL_CONFIG+=
endif

# This directory must match the directory in src/build/common.mak
LIBDIR=$(MF_ROOT)/bigworld/src/lib/bin

all install clean full:
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	cd chunk && $(MAKE) $@
	cd connection && $(MAKE) $@
	cd cstdmf && $(MAKE) $@
	cd dbmgr_lib && $(MAKE) $@
	cd dbmgr_mysql && $(MAKE) $@
	cd dbmgr_xml && $(MAKE) $@
	cd entitydef && $(MAKE) $@
	cd math && $(MAKE) $@
	cd moo && $(MAKE) $@
	cd network && $(MAKE) $@
	cd physics2 && $(MAKE) $@
	cd pyscript && $(MAKE) $@
	cd resmgr && $(MAKE) $@
	cd server && $(MAKE) $@
	cd speedtree && $(MAKE) $@
	cd sqlite && $(MAKE) $@
	cd terrain && $(MAKE) $@
	cd unit_test_lib && $(MAKE) $@
	cd waypoint && $(MAKE) $@
	cd zip && $(MAKE) $@
	cd third_party/openssl && $(MAKE) $(OPENSSL_CONFIG) $@
	cd third_party/CppUnitLite2 && $(MAKE) $@
	cd png && $(MAKE) $@

UNITTEST_LIBS = cstdmf \
				entitydef \
				math \
				network \
				physics2 \
				pyscript \
				resmgr \
				terrain \
				

unit_tests: unit_tests_build unit_tests_run

unit_tests_build:
	# Build the unit tests
	@for i in $(UNITTEST_LIBS); do		\
        $(MAKE) -C $$i/unit_test;		\
		if [ $$? != 0 ]; then			\
			exit 1;						\
		fi								\
     done

unit_tests_run:
	# Run the unit tests
	@for i in $(UNITTEST_LIBS); do			\
        echo "Running unit test:" $$i ;		\
        $(MAKE) -s -C $$i/unit_test run;	\
		if [ $$? != 0 ]; then				\
			exit 1;							\
		fi									\
     done


unit_tests_clean:
	$(MAKE) -C unit_test_lib clean
	# Clean the unit tests
	@for i in $(UNITTEST_LIBS); do			\
        $(MAKE) -C $$i/unit_test clean;		\
		if [ $$? != 0 ]; then				\
			exit 1;							\
		fi									\
     done

version bwversion bw_version:
	@$(MAKE) -s -C server $@
