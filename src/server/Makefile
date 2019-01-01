ifneq ($(wildcard cellappmgr/Makefile),cellappmgr/Makefile)
	LICENSE_SERVER=1
endif

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server,,$(CURDIR))
endif

all clean realclean all_config install unit_tests unit_tests_build unit_tests_run unit_tests_clean::

# Check with and without the trailing slash
ifneq ($(CURDIR),$(MF_ROOT)/bigworld/src/server)
ifneq ($(CURDIR),$(MF_ROOT)bigworld/src/server)
	@echo 'Error: MF_ROOT=$(MF_ROOT)'
	@echo '       is not the root of $(CURDIR)'
	@echo 'Remove the MF_ROOT environment variable.'
	@false
endif
endif

	$(MAKE) -C ../lib $@
	$(MAKE) -C baseapp $@
	$(MAKE) -C baseappmgr $@
	$(MAKE) -C cellapp $@
ifndef LICENSE_SERVER
	$(MAKE) -C cellappmgr $@
endif
	$(MAKE) -C dbmgr $@
	$(MAKE) -C loginapp $@
	$(MAKE) -C reviver $@
	$(MAKE) -C web/python $@
	$(MAKE) -C tools $@

ifdef MF_CONFIG
	@echo completed $@ \(MF_CONFIG = $(MF_CONFIG)\)
else
	@echo completed $@
endif
	$(MAKE) done


clean realclean::
# Explicitly clean out Python library
	$(MAKE) -C ../lib/python $@
# and the Python web integration module
	$(MAKE) -C web/python $@
# and the unit tests
	$(MAKE) -C ../lib unit_tests_clean

done:
ifdef DO_NOT_BELL
else
	echo -n 
endif

server:
	$(MAKE) all
