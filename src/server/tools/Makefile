
ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/server/tools,,$(CURDIR))
endif

all all_config clean realclean install::
	@cd bwmachined && $(MAKE) $@
	@cd bots && $(MAKE) $@
	@cd message_logger && $(MAKE) $@
	@cd consolidate_dbs && $(MAKE) $@
	@cd snapshot_helper && $(MAKE) $@
	@cd sync_db && $(MAKE) $@
	@cd clear_auto_load && $(MAKE) $@

ifdef BUILD_BSDIFF
all::
	@cd $(MF_ROOT)/bigworld/src/tools/offline_patcher/bsdiff && python setup.py build &&	cp `find -name _bsdiff.so | head -n1` $(MF_ROOT)/bigworld/tools/misc/offline_patcher/patcherlib/

clean::
	@cd $(MF_ROOT)/bigworld/src/tools/offline_patcher/bsdiff && python setup.py clean
	rm -f $(MF_ROOT)/bigworld/tools/misc/offline_patcher/patcherlib/bin/*/_bsdiff.so
endif

# TODO: Add support for unit_tests in the tools directory.
unit_tests unit_tests_build unit_tests_run unit_tests_clean::

