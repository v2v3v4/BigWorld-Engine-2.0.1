The projects and scripts in this directory create the llmozlib2 library and package it with the mozilla runtime.

If you just need to rebuild the llmozlib2 library, you can skip ahead to building the xcode project.

Install libIDL
--------------

Building mozilla requires you to have libIDL installed on the build machine.  It's used during the build process, but not actually included as part of the final build package, so it doesn't need to be built universal or installed in the application bundle.

There are a number of ways to install libIDL.  I installed it via "MacPorts" as outlined on this page:

http://developer.mozilla.org/en/docs/Mac_OS_X_Build_Prerequisites

but if you prefer to use Fink or build it from source, that's your choice.

You should only need to do this once, since it's not tied to a particular version of llmozlib2 or your checked-out sandbox.


Building mozilla from scratch
-----------------------------

If you want to start from scratch, or are updating to a new mozilla release tag, you should first build the mozilla project.  The script checkout_patch_build.sh in the build_mozilla directory will check out the mozilla sources from the mozilla cvs server, apply our local patches, and build the whole thing.  Embedded in this script is the CVS tag to check out, which should correspond to one of the official mozilla releases.  If you update to a later version, please commit the change to checkout_patch_build.sh so that others will be able to tell which version is current.

Once you have the mozilla library built, run the copy_products_mac.sh script in this directory to pull the necessary bits across.  This will repopulate the architecture-specific portions of the libraries directory here, so you may want to do a preemptive 'svn remove' and commit first.  It will also create mozilla-universal-darwin-original.tgz which contains the necessary runtime bits that need to go into the application bundle.

At this point, you can 'svn add' any new files in ./libraries, commit them and mozilla-universal-darwin-original.tgz, and someone else will be able to check out this directory and rebuild llmozlib2 without having to do a full mozilla build.  This is the reason for splitting the build process at this point.  

Rebuilding llmozlib2 (Xcode)
---------------------------

*** WARNING ***

Build will fail the first time!  This is expected!  There's some voodoo going on with include directory names.  Open the project in Xcode (or use the command line equivalent listed at the end of this section).  Build it.  Now close the Xcode project and reopen it and build it again (or just run the command line equivalent again).

Once you've got the mozilla libraries in place, open llmozlib2.xcodeproj and build the "Release" build style.  If the build fails with a bunch of missing include files, close the project and reopen it, then try again.  (A script phase in the project creates a symlink, but xcode only reevaluates the wildcard header path search when the project is opened.  You should only have to do this once per checkout.)

If the llmozlib2 build still fails due to missing header files after doing the steps above, you'll want to figure out where in the mozilla build they live and modify copy_products_mac.sh to copy them, run it (so they get pulled into ./libraries), 'svn add' the new files, and commit.

If there's any chance the public interface (i.e. member functions) of the LLMozLib class has changed, you need to regenerate the exports file and (re)build the "Release" target.  See the NOTE about llmozlib2.exp below.

If you did a full mozilla build and you're certain you have all the header files you need copied into ./libraries, it's now safe to delete the files that the mozilla build generated:

	build_mozilla/cvsco.log
	build_mozilla/mozilla
	build_mozilla/objdir-mozilla-universal

After the xcode build completes, run repackage_runtime.sh to do the final packaging step (including creating mozilla-universal-darwin.tgz from mozilla-universal-darwin-original.tgz and the llmozlib2 build products) and copy the necessary pieces into your sandbox.  The script takes a single argument, which is the location of the sandbox directory.  (Note that this should be the directory containing the 'libraries' and 'indra' directories.)  The script should be run from the llmozlib2 directory, and the sandbox path can either be relative or absolute (i.e. './repackage_runtime.sh ../../BRANCH' will work fine).

NOTE:  Command line build command:

xcodebuild -project llmozlib2.xcodeproj -target llmozlib2 -configuration Release build

(You can use "clean" instead of "build")

-----------
NOTE: about llmozlib2.exp

In order to hide internal symbols in the mozilla library, we have to use an explicit export list when building llmozlib2.

The list of symbols to export is in llmozlib2.exp.  The symbols are in "name mangled" form, making them a bit of a pain to read.

If changes are made to any of the class definitions in llmozlib2.h, this list may need to be regenerated.  The script update-mac-symbols.sh in this directory now takes care of doing this step.  The following directions are what I used to do manually, preserved for posterity (or in case the script breaks in the future).  If you run the script and it works, you don't need to read any further.


To regenerate the list of symbols:

- Open up the xcode project
- Build the "All Symbols" configuration
- Execute the following commands:

otool -vT 'build/All Symbols/libllmozlib2.dylib' >symbols.txt
sed -n -e '/.*\.eh/d' -e 's/^single module    \(.*\)/\1/p' <symbols.txt >symbols2.txt

- symbols2.txt should now contain the list of global symbols that the library would export with no controls in place.

Now comes the hard part.  The list must be manually pruned.  At the moment, deleting all symbols that don't begin with __ZN8LLMozLib seems to be correct.  Look at the existing llmozlib2.exp for guidance.

Once you've got the new list, paste it into llmozlib2.exp and change the "Exported Symbols File" setting back to "llmozlib2.exp".

NOTE:  Command line build command:

xcodebuild -project llmozlib2.xcodeproj -target llmozlib2 -configuration Release build

(You can use "clean" instead of "build")

-----------
NOTE: about llmozlib2_stub.c

Due to the way the Mac linker works, it was necessary to create a 'stub' library.  This is a shared library containing nothing but the export symbols that is used to make the linker happy when building the application.  The real shared library then gets referenced during runtime.

There's a shell script phase in the project that runs a 'sed' script which creates llmozlib2_stub.c from llmozlib2.exp.  It's about as stupid as possible.  

Make sure the stub library gets copied into libraries/universal-darwin/lib_{debug|release} and that the real library gets put into the application bundle.  Getting either one into the other's place will cause problems.
