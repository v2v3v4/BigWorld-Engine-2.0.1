LIB = network

SRCS =							\
	basictypes					\
	blocking_reply_handler		\
	bsd_snprintf				\
	bundle						\
	channel						\
	channel_owner				\
	compression_stream			\
	condemned_channels			\
	delayed_channels			\
	endpoint					\
	error_reporter				\
	event_dispatcher			\
	event_poller				\
	file_stream					\
	forwarding_string_handler	\
	fragmented_bundle			\
	frequent_tasks				\
	interface_element			\
	interface_minder			\
	interface_table				\
	irregular_channels			\
	keepalive_channels			\
	logger_message_forwarder	\
	machine_guard				\
	machined_utils				\
	monitored_channels			\
	network_interface			\
	netmask						\
	once_off_packet				\
	packet						\
	packet_filter				\
	packet_receiver				\
	packet_receiver_stats		\
	process_socket_stats_helper	\
	remote_stepper				\
	request						\
	request_manager				\
	rescheduled_sender			\
	sending_stats				\
	unacked_packet				\
	unpacked_message_header		\
	watcher_connection			\
	watcher_nub					\
	watcher_packet_handler		\
	zip_stream					\

ifndef USE_OPENSSL
	USE_OPENSSL = 1
endif

ifeq ($(USE_OPENSSL),1)
	SRCS +=							\
		encryption_filter			\
		public_key_cipher			\

endif

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

include $(MF_ROOT)/bigworld/src/build/common.mak

all::

ifeq ($(wildcard bsd_snprintf.cp*),bsd_snprintf.cpp)
# Do not optimise this file. bsdFormatFloat does not perform
# correctly when it is optimised past O2
$(MF_CONFIG)/bsd_snprintf.o: bsd_snprintf.cpp
ifdef QUIET_BUILD
	test -e $(MSG_FILE) && cat $(MSG_FILE); rm -f $(MSG_FILE)
	@echo $<
endif
	$(COMPILE.cc) -O0 $< $(OUTPUT_OPTION)
	@if test -e bsd_snprintf.d; then 						\
		echo -n $(MF_CONFIG)/ > $(MF_CONFIG)/bsd_snprintf.d;	\
		cat bsd_snprintf.d >> $(MF_CONFIG)/bsd_snprintf.d;\
		rm bsd_snprintf.d;									\
	fi
endif

