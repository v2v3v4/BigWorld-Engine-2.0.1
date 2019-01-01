#!/bin/bash

# This script is expected to be run from the "build-mozilla" directory inside llmozlib.
# it will check out the mozilla source to the "mozilla" directory, 
# and build to "objdir-mozilla-universal", so that those two
# will also be in build-mozilla.

# check out the mozilla source
export CVSROOT=':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot'
echo 'use the password "anonymous" if prompted'
cvs login
#cvs checkout -r MOZILLA_1_8_BRANCH mozilla/client.mk
cvs checkout -r FIREFOX_2_0_0_18_RELEASE mozilla/client.mk
(cd mozilla && make -f client.mk checkout MOZ_CO_PROJECT=xulrunner)

# install our special .mozconfig file
cp mac-universal-libxul-bits/mozconfig mozilla/.mozconfig

# The makefile that builds the installable xulrunner package requires some setuid 
# shell scripts that I don't want to set up.  Since we don't need to build the 
# package, just comment this out.
# NOTE: this is now part of the linden.patch file instead.
#cp mac-universal-libxul-bits/installer-mac-Makefile.in mozilla/xulrunner/installer/mac/Makefile.in

# replace the 'flight.mk' with one that does the right thing for the xulrunner build
# NOTE: this will break other types of product builds!
# NOTE: This may have been fixed in 1.5.0.7... testing now.
# cp mac-universal-libxul-bits/macosx-universal-flight.mk mozilla/build/macosx/universal/flight.mk

# apply a patch to the mozilla source so that it doesn't try to use the OS 
# versions of scrollbars, combo boxes, etc.
cat linden.patch | (cd mozilla && patch -p0)

(cd mozilla && make -f client.mk build)

# the universal version of XUL.framework will be put in:
# ./objdir-mozilla-universal/ppc/dist/universal/XUL.framework

# we may also need libraries from:
# ./objdir-mozilla-universal/ppc/dist/lib
# ./objdir-mozilla-universal/i386/dist/lib

# and header files from various places.

