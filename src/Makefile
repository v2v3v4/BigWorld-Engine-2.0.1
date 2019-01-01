ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src,,$(CURDIR))
endif


all clean unit_tests unit_tests_clean unit_tests_run:
	$(MAKE) -C lib $@
	$(MAKE) -C examples $@
	$(MAKE) -C server $@
	$(MAKE) -C tools $@


ARCH=
ifeq ($(shell uname -m),x86_64)
	 ARCH=64
endif

all_configs:
	$(MAKE) MF_CONFIG=Hybrid$(ARCH) all
	$(MAKE) MF_CONFIG=Debug$(ARCH) all


all_configs_test:
	$(MAKE) MF_CONFIG=Hybrid$(ARCH) unit_tests
	$(MAKE) MF_CONFIG=Debug$(ARCH) unit_tests


all_configs_clean:
	$(MAKE) MF_CONFIG=Hybrid$(ARCH) cleanall
	$(MAKE) MF_CONFIG=Debug$(ARCH) cleanall


cleanall:
	$(MAKE) clean
	$(MAKE) unit_tests_clean

