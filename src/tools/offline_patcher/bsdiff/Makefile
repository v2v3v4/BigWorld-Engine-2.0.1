BZ2_SOURCES = blocksort bzlib compress crctable decompress huffman randtable

ifeq ($(shell uname -m),x86_64)
	PLATFORM=linux_64bit
else
	PLATFORM=linux_32bit
endif

OUTPUT_DIR = ../../../../tools/misc/offline_patcher/patcherlib/bin/$(PLATFORM)

.PHONY: bsdiff clean

all:: bsdiff
	
libbz2.a: $(addsuffix .o,$(addprefix bz2/,$(BZ2_SOURCES)))
	$(AR) r $@ $^
	
CFLAGS += -fPIC

bsdiff: libbz2.a
	python setup.py clean
	python setup.py build
	mkdir -p $(OUTPUT_DIR)
	touch $(OUTPUT_DIR)/__init__.py
	find -name '_bsdiff.so' -exec cp -v {} $(OUTPUT_DIR) \;

clean:
	$(RM) bz2/*.o
	$(RM) -r build
	$(RM) libbz2.a
	$(RM) $(OUTPUT_DIR)/_bsdiff.so
	python setup.py clean
