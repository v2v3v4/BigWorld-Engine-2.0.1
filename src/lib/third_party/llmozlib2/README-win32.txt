Getting, patching, building and using LLMozLib2 on the Windows platform
=======================================================================


Table of Contents

A. Getting, Patching and Building using Microsoft Visual Studio 2005 (vc8)
B. Getting, Patching and Building using Microsoft Visual Studio 2003 (vc7.1)

========================================================================

A. Getting, Patching and Building using Microsoft Visual Studio 2005 (vc8)
These instructions describe how to build the Windows version of LLMozlib2 for use within the Second Life client. THey have been tested using Microsoft Visual Studio 2005 (vc8) - instructions for 2003 (vc7.1) can be found later in this document. Other versions may work but not have been tried or tested.

LLMozLib2 currently uses code from 1.8.1.x branc of the mozilla codebase -similar to Firefox 2.0. Officially, compiling this version of mozilla with Visual Studio 2005 is not supported. However, with a few adjustments, it is possible. As of the time of this writing, Visual Studio 2005 is the officially supported development environment of Linden Lab. Instructions for using Visual Studio 2003 are included at the end of this document for historical and compatibility purposes.

Here are the steps necessary to get, patch, and build LLMozlib2 from a standard development setup:

1. Prepare the Mozilla prerequisites.

1.a. Install Cygwin with a minimum of the following packages:
 * ash
 * coreutils
 * diffutils
 * findutils
 * gawk
 * grep
 * libiconv
 * make 3.80 (NOT 3.81)
 * patchutils
 * perl
 * sed
 * unzip
 * zip

The default cygwin installer no longer has access to make 3.80. As such, you can get it from paracoda (http://cygwin.paracoda.com/release/make/make-3.80-1.tar.bz2).
To install, untar this archive into your cygwin/bin directory.

If you have any difficulty with this step or wish to get more details about the packages, please refer to http://developer.mozilla.org/en/docs/Windows_Build_Prerequisites

1.b. Download and install mozilla-build and moztools

The Mozilla-build package contains all of the essential tools for building mozilla. Download it at http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/MozillaBuildSetup-1.3.exe

This guide was written based on the 1.3 version of mozilla-build. In order to check for a more up to date version, check http://developer.mozilla.org/en/Windows_Build_Prerequisites

Next, download the static version of moztools. You can download the archive at http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/moztools-static.zip
When unzipping, be certain to preserve the directory structure. It is easiest to place the extracted directory in C:\mozilla-build\moztools. If you decide to leave this directory elsewhere, be certain to set the environment variable MOZ_TOOLS. 

If you have any difficulty getting moztools downloaded or installed, please refer to http://developer.mozilla.org/en/Windows_Build_Prerequisites_on_the_1.7_and_1.8_Branches

1.c. Download and install the Windows Platform SDK

It is recommended to use the windows SDK for Windows Vista located at:
http://www.microsoft.com/downloads/details.aspx?familyid=4377F86D-C913-4B5C-B87E-EF72E5B4E065&displaylang=en

However, the Windows Server 2003 SP1 platform SDK has also been successfully used. 

2. Checkout LLMozlib2 from Linden Labs
As of the time of this writing, the latest version of LLMozlib2 is available via SVN checkout at:
https://svn.secondlife.com/svn/llmozlib/trunk/llmozlib2

Use your favorite SVN client software to download this branch to your local machine.

3. Open an MSYS console and obtain the mozilla source
Run start-msvc8.bat located in C:\mozilla-build to start a MSYS console. Continue using this console for any console commands for the rest of this document until specified otherwise. If you have difficulty opening the console, consult the windows build prerequisites pages for mozilla, located at 

http://developer.mozilla.org/en/Windows_Build_Prerequisites

and

http://developer.mozilla.org/en/Windows_Build_Prerequisites_on_the_1.7_and_1.8_Branches

Once you have your MSYS console open, execute the following commands to check out the client.mk file to your C:\mozilla directory. NOTE: this will fail if the directory C:\mozilla exists. Please delete this folder and its contents if it already exists.

	cd /c
	export CVSROOT=':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot'
	cvs checkout -r FIREFOX_2_0_0_13_RELEASE mozilla/client.mk

Next, use the client.mk file to obtain the rest of the mozilla source & build tree:

	cd mozilla
	make -f client.mk checkout MOZ_CO_PROJECT=xulrunner

This will take a little while. Take your first coffee break while this finishes.

4. Copy over linden-specific files and patch the mozilla source

copy the necessary linden-specific files to your C:\mozilla directory from the LLMozLib2 checkout:

 * .mozconfig.debug
 * .mozconfig.optimized
 * linden.patch

Once this is done, apply the patch by executing the following command (still in the MSYS console):

	patch -p0 < linden.patch

If you get patch errors, verify that you checked out the correct version of Mozilla/gecko.

5. Build the optimized version of gecko:

First, set the active config to optimized with the following command:

	cp .mozconfig.optimized .mozconfig

Next, start the build process:

	make -f client.mk build

NOTE: this process will take a while, but ultimately will fail, claiming to be unable to find MSVCr80.dll. This is due to one of the issues with compiling gecko 1.8.1 under vc8. However, in the process of attempting the build, the proper manifest files will be generated. To move these files into the correct location, execute the following commands:

	cd objdir-opt-xulrunner-small/
	find ./ -iname *.exe.manifest -print0 | xargs -0 -t -i cp {} dist/bin
	cd ..

Now that the manifest files have been moved into the proper directory, your build should succeed. You should be in the /c/mozilla directory again while running this command (this will take a while):

	make -f client.mk build

6. Build the debug version of gecko:
This is the same process as building the optimized version, but using a different config file and a different object directory. Note the differences:

First, set the active config to optimized with the following command:

	cp .mozconfig.debug .mozconfig

Next, start the build process:

	make -f client.mk build

NOTE: this process will take a while, but ultimately will fail, claiming to be unable to find MSVCr80.dll. This is due to one of the issues with compiling gecko 1.8.1 under vc8. However, in the process of attempting the build, the proper manifest files will be generated. To move these files into the correct location, execute the following commands:

	cd objdir-debug-xulrunner-small/
	find ./ -iname *.exe.manifest -print0 | xargs -0 -t -i cp {} dist/bin
	cd ..

Now that the manifest files have been moved into the proper directory, your build should succeed. You should be in the /c/mozilla directory again while running this command (this will take a while):

	make -f client.mk build

7. Unify the runtime libraries for both builds (this will improve runtime performance)

First, unify the libraries for the optimized build:

	cd /c/mozilla/objdir-opt-xulrunner-small/dist/bin/
	./xpt_link all.tmp components/*.xpt
	rm components/*.xpt
	mv all.tmp components/all.xpt
	
Next, do the same for the debug build:

	cd /c/mozilla/objdir-debug-xulrunner-small/dist/bin/
	./xpt_link all.tmp components/*.xpt
	rm components/*.xpt
	mv all.tmp components/all.xpt

8. copy results of mozilla/gecko build to LLMozLib2

From a windows command prompt, run the batch file labeled copy_products_windows.bat from the LLMozLib2 directory.
NOTE: the script assumes you used C:\mozilla\ to checkout and build gecko.

9. Build LLMozlib2

Open the solution named llmozlib2_vc8.sln and proceed to build BOTH the debug and release versions in the normal fashion.

NOTE: If you would like to update both the vc8 and vc7 versions of the library, get both versions to compile up to this point before proceeding.

NOTE: LLMozLib2 must successfully compile - the other two projects in the solution are sample programs for testing LLMozlib and are not required in order to use LLMozLib2 in the SecondLife viewer.

10. Copy LLMozLib2 products to the branch of your viewer

First, edit the batch file called copy_llmozlib2_to_branch.bat located in your LLMozlib2 checkout.
You should only have to modify the second line, which sets a variable called DEST_DIR which should point to the root folder of your viewer source checkout (the root folder not indra).

Then, run this batch file from a windows command prompt from the LLMozLib2 checkout directory to copy the products of LLMozLib2 to your viewer source checkout.

11. Compile the viewer and test your configuration

Now you are ready to compile the viewer. If you have already built the viewer, you need to at least ensure the windows libraries are copied to the correct location and re-link the viewer. However, a full rebuild is recommended.

If your newly-compiled viewer brings up the main window of the login screen properly, then it is correctly finding and utilizing LLMozlib2. 


========================================================================


B. Getting, Patching and Building using Microsoft Visual Studio 2003 (vc7.1)

	These instructions describe how to build the Windows version of LLMozLib2 for use within the Second Life client. They have been tested using Microsoft Visual Studio 2003 (v7.1) - other versions of the Microsoft compiler may also work but they have not been tried or tested.

LLMozLib2 currently uses code from the 1.8.1.x branch of the Mozilla codebase - somewhat similar to what is used in Firefox 2.0.

Here are the steps you need to take to set up your build environment, get the Mozilla source, patch it, build it and copy it to the right place in the LLMozLib2 source tree. After that, you can build LLMozLib2 normally using the Visual Studio solution file.

* Checkout llmozlib2 from the Second Life public SVN repository: https://svn.secondlife.com/svn/llmozlib/trunk/llmozlib2/ 

* Follow the instructions on the Mozilla Windows Build Prerequisites page - http://developer.mozilla.org/en/docs/Windows_Build_Prerequisites

* Typically this consists of downloading and installing the MozillaBuild package into C:\mozilla-build - refer to the Web page for details since it's likely this will change over time and that page will obviously always have the most recent version.

* Get to an MSYS command prompt as per instructions described in the aforementioned Web page.

* Change directory to the MSYS /c drive (the C: drive in Windows) 

   cd /c 

* Checkout the Mozilla configuration file 

    export CVSROOT=':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot'
    cvs checkout -r FIREFOX_2_0_0_13_RELEASE mozilla/client.mk 
    
    Notes:  * replace the FIREFOX_2_0_0_13_RELEASE tag with one that refers to the version you want
            * the CVS password is 'anonymous'
    
* Change to the directory you checked out in 

    cd mozilla 

* Checkout the source code 

    make -f client.mk checkout MOZ_CO_PROJECT=xulrunner 

* Copy the following files from the llmozlib2/build_mozilla directory into the C:\mozilla directory you just checked out 

    * .mozconfig.debug
    * .mozconfig.optimized
    * linden.patch 

* Patch the Mozilla source with the Linden patch

    patch -p0 < linden.patch 

* Copy over the .mozconfig file for the optimized (release) build 

    cp .mozconfig.optimized .mozconfig 

* Start a build: 

    make -f client.mk build 

* Wait - takes about 20 minutes on a typical development system

* Copy over the .mozconfig file for the debug build 

    cp .mozconfig.debug .mozconfig 

* Start a build: 

    make -f client.mk build 

* Wait - takes about 40 minutes on a typical development system 

* Unify the Mozilla runtime type libraries - convert the *.xpt files into a single, unified one - improves startup time considerably. 

    * Using the MSYS shell from the /c/mozilla/objdir-opt-xulrunner-small/dist/bin directory:

        ./xpt_link all.tmp components/*.xpt 
        rm components/*.xpt 
        mv all.tmp components/all.xpt 

    * Using the MSYS shell from the /c/mozilla/objdir-debug-xulrunner-small/dist/bin directory:

        ./xpt_link all.tmp components/*.xpt 
        rm components/*.xpt 
        mv all.tmp components/all.xpt 

* Copy over the necessary Mozilla files into the LLMozLib2 directory 

    copy_products_windows.bat 
    
    Note: (run from the LLMozLib2 directory) 

* Build LLMozLib2 using Microsoft Visual Studio 2003 (v7.1) 

    Open the LLMozLib2 solution file llmozlib2.sln. 

* Build the LLMozLib Debug and Release configurations in the normal fashion. 

* Copy the LLMozLib2 files to the Second Life client branch 

    copy_llmozlib2_to_branch.bat 
    
    Note1: (run from the LLMozLib2 directory) 
    Note2: (edit the batch file to reflect the branch you want to copy to) 

* That completes the process - you should now have a new set of LLMozLib2/Mozilla runtime files in the branch you specified. The files that are copied are:
    * Header file (llmozlib2.h) in libraries\include\
    * Static library (llmozlib2.lib) (debug) in libraries\i686-win32\lib_debug\
    * Static library (llmozlib2d.lib) (release) in libraries\i686-win32\lib_release\
    * DLLs (debug) in libraries\i686-win32\lib_debug\
    * DLLs (release) in libraries\i686-win32\lib_release\
    * Runtime misc files (debug) in indra\newview\app_settings\mozilla_debug\
    * Runtime misc files (release) in indra\newview\app_settings\mozilla_release\

* If you want to experiment with the test applications (uBrowser, testGL for example), you can change the startup project in Visual Studio and build the one you want. A separate document in this directory contains notes on using each test app.

---- end of doc ----