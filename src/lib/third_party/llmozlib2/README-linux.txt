Before you begin: If you're building a Linux Mozilla tree for embedding
into an app for redistribution - for example if you're Linden Lab -
then you should read README-linux-linden.txt first for some preconfiguration
info which ensures that the Mozilla you build will be binary-compatible with
the widest range of x86 Linux versions.

1. BUILDING A LLMOZLIB-COMPATIBLE MOZILLA
=========================================

You can skip this part if you already have a llmozlib-compatible mozilla build
(say, in libraries/i686-linux) and just want to hack on llmozlib itself.

0. Edit build_mozilla/linux-libxul-bits/mozconfig:
   * Change GCC_VERSION if desired
1. cd build_mozilla
2. ./linux-checkout_patch_build.sh
[If prompted for a password, simply press return]

This will check an appropriate source tree out of the Mozilla CVS repository,
patch it with our custom modifications and build it according to our desired
configuration.  May take an hour or two to run.

3. cd ..
4. ./copy_products_linux.sh

This copies the parts of the built Mozilla tree that we care about, into
libraries/i686-linux/*

2. BUILDING LLMOZLIB
====================

0. Edit build-linux-llmozlib.sh
   * Change CXX as you see fit.
1. ./build-linux-llmozlib.sh

This simple builder-script will build the libllmozlib2.a library.

3. BUILDING THE TEST APPLICATIONS
=================================

This is an optional step.

The main test application (and the only one regularly tested under Linux)
is uBrowser, which is a simple web browser which can be interacted with
on a 3D surface in OpenGL.  It requires the GLUI library which can be
downloaded from http://glui.sourceforge.net/

0. Edit tests/ubrowser/ubrowser-linux-build.sh
   * Change CXX if appropriate.
1. cd tests/ubrowser/
2. ./browser-linux-build.sh

This simple builder-script will build the 'ubrowser' program.  To run:

3. cd ../../libraries/i686-linux/runtime_release
4. LD_LIBRARY_PATH=`pwd` ../../../tests/ubrowser/ubrowser

