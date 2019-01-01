#!/bin/bash

# This script is expected to be run from the "build-mozilla" directory inside llmozlib.
# it will check out the mozilla source to the "mozilla" directory, 
# and build to "objdir-mozilla-linux", so that those two
# will also be in build-mozilla.

# check out the mozilla source
export CVSROOT=':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot'
echo 'use the password "anonymous" if prompted'
cvs login
cvs checkout -r FIREFOX_2_0_0_18_RELEASE mozilla/client.mk
(cd mozilla && make -f client.mk checkout MOZ_CO_PROJECT=xulrunner)

# install our special .mozconfig file
cp linux-libxul-bits/mozconfig mozilla/.mozconfig

# apply a patch to the mozilla source so that it doesn't try to use the OS 
# versions of scrollbars, combo boxes, etc.
cat linden.patch | (cd mozilla && patch -p0)

# XXX FIXME: ideally we need to bring custom GTK in-tree and do some magic on its .pc files - yuck.  Only Linden Lab's conservative-deps binary builds really care though.
export PKG_CONFIG_PATH=/tmp/GTKBUILD24/lib/pkgconfig
(cd mozilla && make -f client.mk build)

