echo
echo Message from BigWorld
echo ---------------------
echo You are trying to build a stand-alone Python interpreter. This is usually
echo the result of running \'make\' from the mf/src/lib/python directory. This
echo operation will fail due to the modifications that BigWorld has made to
echo Python.
echo
echo To build the Python library required by BigWorld server, please run
echo \'make libpython2.5.a\'. This is usually not necessary as one is included
echo in the package.
echo
echo To build the shared modules \(.so\'s\) that are part of Python\'s standard
echo library, please follow the elaborate build instructions at the end of
echo mf/src/lib/python/changes.txt. This is usually not necessary as all the
echo required .so\'s are included in the package - in
echo mf/bigworld/res/entities/common/lib-dynload.
echo
echo If you wish to re-run the configure script, you must include the option
echo \'--enable-unicode=ucs4\', otherwise it will not link with BigWorld.
echo
exit 2
