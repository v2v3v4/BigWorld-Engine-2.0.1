ifeq (,$(MF_CONFIG))
MF_CONFIG = Hybrid
	ifeq ($(shell uname -m),x86_64)
		 MF_CONFIG=Hybrid64
	endif
endif
