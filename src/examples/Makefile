all clean full:
	$(MAKE) -C cellapp_extension $@
	$(MAKE) -C client1 $@
	$(MAKE) -C client2 $@
	$(MAKE) -C client3 $@
	$(MAKE) -C client4 $@

# This rule is a nop. The top level Makefile assumes everything
# it invokes will have this rule.
unit_tests unit_tests_clean unit_tests_run:
