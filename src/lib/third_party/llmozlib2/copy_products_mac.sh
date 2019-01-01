#!/bin/bash

# This script copies the relevant headers from the mozilla tree to the llmozlib 
# development tree.
# Note that the headers may be architecture-dependent, so they need to be copied 
# to the relevant architectural subdirectories of libraries.

# NOTE: this assumes a mozilla tree that was built with build_mozilla/checkout_patch_build.sh 
export MOZ_OBJDIR="build_mozilla/objdir-mozilla-universal"

# install libraries and headers in llmozlib/libraries/
export LIBRARIES_DEST="."

# make sure the destination directories exist.  If they don't, the user running the script probably hasn't set them up properly.
if [ \( \! -d "$MOZ_OBJDIR" \) -o  \( \! -d "$LIBRARIES_DEST" \) ] ; then	
	echo "Either MOZ_OBJDIR or LIBRARIES_DEST are not set properly for your system.  Please modify $0 to set them correctly."
	exit 1	
fi

# add --dry-run to just see what this will do (without actually doing it)
RSYNC_OPTIONS="--recursive --checksum --verbose --progress --copy-unsafe-links"

#############################

# copy universal shared libraries to debug and release directories
for j in lib_release ; do
	mkdir -p "$LIBRARIES_DEST/libraries/universal-darwin/$j"
	rsync --include-from=- $RSYNC_OPTIONS \
		"$MOZ_OBJDIR/ppc/dist/universal/xulrunner/XUL.framework/Versions/Current/" \
		"$LIBRARIES_DEST/libraries/universal-darwin/$j" \
		<<EOF
+ XUL
+ libmozjs.dylib
+ libnspr4.dylib
+ libplc4.dylib
+ libplds4.dylib
+ libxpcom.dylib
- *
EOF

done

#############################

# copy static libraries to all architecture-specific directories
for i in i386 powerpc ; do
	SRC="$MOZ_OBJDIR/$i"
	DST="$LIBRARIES_DEST/libraries/$i-darwin"

	if [ $i = powerpc ] ; then
		SRC="$MOZ_OBJDIR/ppc"
	fi

	for j in lib_release ; do
		mkdir -p "$DST/$j"
		rsync --include-from=- $RSYNC_OPTIONS \
			"$SRC/dist/lib/" \
			"$DST/$j/" \
			<<EOF
+ libprofdirserviceprovider_s.a
- *
EOF

	done
done

#############################

# copy a subset of the headers per-architecture

for i in i386 powerpc ; do
	SRC="$MOZ_OBJDIR/$i"
	DST="$LIBRARIES_DEST/libraries/$i-darwin"

	if [ $i = powerpc ] ; then
		SRC="$MOZ_OBJDIR/ppc"
	fi

	mkdir -p "$DST/include/mozilla/"
	rsync --include-from=- $RSYNC_OPTIONS "$SRC/dist/" "$DST/include/mozilla/" <<EOF
+ /include/
+ /include/webbrwsr/
+ /include/docshell/
+ /include/dom/
+ /include/xpcom/
+ /include/widget/
+ /include/gfx/
+ /include/string/
+ /include/uriloader/
+ /include/view/
+ /include/layout/
+ /include/content/
+ /include/locale/
+ /include/profdirserviceprovider/
+ /include/xulapp/
+ /include/pref/
+ /include/necko/
+ /include/nkcache/
+ /include/js/
+ /sdk/
+ /sdk/include/
+ /sdk/include/*/
+ *.h
+ *.tbl
- *
EOF

done

#############################

# tar up the runtime and llmozlib 

rm -f "./mozilla-universal-darwin-temp.tgz"
tar -zcf "./mozilla-universal-darwin-temp.tgz" \
	-C "$MOZ_OBJDIR/ppc/dist/universal/xulrunner/XUL.framework/Versions/Current/" . 

echo "Unpacking mozilla-universal-darwin-temp.tgz..."

# expand the runtime archive
rm -rf mozilla-universal-darwin-temp
mkdir -p mozilla-universal-darwin-temp
(cd mozilla-universal-darwin-temp && tar -zxf ../mozilla-universal-darwin-temp.tgz)

echo "Consolidating .xpt components"
# (don't do this in the original build directory, since it's a one-way process)
"$MOZ_OBJDIR/ppc/dist/bin/xpt_link" mozilla-universal-darwin-temp/all.xpt mozilla-universal-darwin-temp/components/*.xpt
rm mozilla-universal-darwin-temp/components/*.xpt
mv mozilla-universal-darwin-temp/all.xpt mozilla-universal-darwin-temp/components/

echo "Creating mozilla-universal-darwin-original.tgz..."

tar -zcf "./mozilla-universal-darwin-original.tgz" \
	-C "mozilla-universal-darwin-temp/" . 
