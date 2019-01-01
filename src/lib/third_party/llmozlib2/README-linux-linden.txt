This document describes how to get a build of the llmozlib version of
Mozilla which is:
* Compatible with Second Life
* Compatible out-of-the-box with the widest variety of x86 Linux systems
as is practical - cross-distribution linux binary compatibility is quite
delicate so we conservatively fine-tune Mozilla's wild build dependencies.

If you are building apps which entirely rely upon system library versions
and thus aren't meant to be portable between Linux distributions, you can
skip this document.

These steps otherwise need to be completed before moving on to README-linux.txt

1. LIBJPEG
==========

<special instructions no longer applicable>

2. GTK AND FRIENDS
==================

Optional, recommended.

A GTK-using app's binary interfaces aren't really backward-compatible in the
slightest, which is inconvenient since building on your distro is likely
introducing binary dependancies on the latest gee-whiz version of GTK.
Thus, we build against an older hand-rolled version of GTK which just about
everyone will be run-time compatible with.

0a. Get a build of GTK 2.4 and its many dependencies.  For Linden, you can
    check out a recommended tree of prebuilt x86 linux goodness from
    lindenlib/trunk/GTK-2.4-i686-linux
0b. In that GTK tree, edit lib/pkgconfig/*.pc so that every 'prefix=' line
    points to the absolute location of your GTK-2.4-i686-linux directory -
    yes this is a pain.
1. Edit build_mozilla/linux-checkout_patch_build.sh
   * Uncomment the export PKG_CONFIG_PATH line
   * Edit the PKG_CONFIG_PATH to point to the /lib/pkgconfig directory
     inside your GTK-2.4-i686-linux tree.

3. BUILDING
===========

Proceed to README-linux.txt to build llmozlib and Mozilla, then come
back here to verify that the build is appropriately 'compatible'.

4. BASIC TESTS FOR COMPATIBILITY
================================

1. strings libraries/i686-linux/runtime_release/libxul.so |grep lib|grep cairo
   ... should return nothing; this demonstrates that you have not accidentally
   introduced a hard dependancy against cairo (indicating erroneous linking
   against a fairly recent GTK version).

5. USING WITH SECOND LIFE
=========================

Probably only of interest if you're a Second Life developer!

1. copy llmozlib2.h to ${SLSRC}/libraries/include/
2. copy libllmozlib2.a to ${SLSRC}/libraries/i686-linux/lib_release_client/
3. copy libraries/i686-linux/lib_release/libprofdirserviceprovider_s.a to ${SLSRC}/libraries/i686-linux/lib_release_client/
4. copy libraries/i686-linux/runtime_release to ${SLSRC}/indra/newview/app_settings/mozilla-runtime-linux-i686

5. Rebuild Second Life

6. TESTING WITH SECOND LIFE
===========================

Things to test:
1. The HTML Login screen - should work!
2. F1 Help
3. Sending a postcard should work - if it crashes, it's probably a libjpeg
   problem (see Section 1.)
4. Performing an upload should bring up a working file selector, otherwise it
   may be a GTK problem (see Second 2.)
