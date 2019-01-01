#!/bin/bash

# This script copies the relevant headers from the mozilla tree to the llmozlib 
# development tree.
# Note that the headers may be architecture-dependent, so they need to be copied 
# to the relevant architectural subdirectories of libraries.

# NOTE: this assumes a mozilla tree that was built with build_mozilla/linux-checkout_patch_build.sh 
export MOZ_OBJDIR="build_mozilla/objdir-mozilla-linux"

# install libraries and headers in llmozlib/libraries/
export LIBRARIES_DEST="."

export ARCH=`uname -m`-linux

# make sure the destination directories exist.  If they don't, the user running the script probably hasn't set them up properly.
if [ \( \! -d "$MOZ_OBJDIR" \) -o  \( \! -d "$LIBRARIES_DEST" \) ] ; then	
	echo "Either MOZ_OBJDIR or LIBRARIES_DEST are not set properly for your system.  Please modify $0 to set them correctly."
	exit 1	
fi


#############################

# copy a subset of the headers

SRC="$MOZ_OBJDIR"
DST="$LIBRARIES_DEST/libraries/$ARCH"
for dir in /include/webbrwsr/ /include/docshell/ /include/dom/ /include/xpcom/ /include/widget/ /include/gfx/ /include/string/ /include/uriloader/ /include/view/ /include/layout/ /include/content/ /include/locale/ /include/profdirserviceprovider/ /include/xulapp/ /include/pref/ /include/necko/ /include/nkcache/ /include/js/ /include/appshell/ /sdk/include/ ; do
	mkdir -p $DST/include/mozilla/${dir}
	cp -LR $SRC/dist/${dir}/* $DST/include/mozilla/${dir}
done
#cp -LR $SRC/dist/*.h $DST/include/mozilla/

# copy release (opt) libraries

mkdir -p $DST/lib_release

for file in libmozjs.so libnspr4.so libplc4.so libplds4.so libprofdirserviceprovider_s.a libxpcom.so libxul.so ; do
    cp -LR $SRC/dist/lib/$file $DST/lib_release/
done

# copy release (opt) runtimes

mkdir -p $DST/runtime_release

for glob in chrome components greprefs plugins res '*.so' ; do
    cp -LR `echo $SRC/dist/bin/$glob` $DST/runtime_release/
done

