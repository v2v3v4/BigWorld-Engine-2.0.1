Issues to discuss:


5. Fix the web bone tv/ get new models.
13. try changing the editfield to be a window with a text component inside of it. (check why online doesn't allow writing). anchor point to be left at start and right if too big  ==> done


TBD : change editfield to work on a window with a child of edit field :

********************
HOW to build mozilla
********************
Use instructions in http://svn.secondlife.com/trac/llmozlib/browser/trunk/llmozlib2/README-win32.txt
see also http://wiki.secondlife.com/wiki/LLMozLib2#Getting_the_necessary_Mozilla_code.2C_building_it_and_then_building_LLMozLib2
special considertaions:

1. after installing the windows sdk in order to run the start-msvc8.bat you need to do: 
	set SDKDIR=C:\Program Files\Microsoft SDKs\Windows\v6.1\bin
	cd mozilla-build
	start-msvc8.bat
	
2. Copy to the fantasy demo directory the following C:\mf_trunk_2\src\lib\third_party\LLMozlib2\libraries\i686-win32\runtime_release

3. All build configurations now use llmozlib as release (as it requires different dlls for debug and release)



============
