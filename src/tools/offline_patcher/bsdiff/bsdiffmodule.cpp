#include "pch.hpp"

#include "bsdiffmodule.hpp"


#include <string.h>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <errno.h>
#	include <unistd.h>
#endif

namespace // anonymous
{

/**
 *	Convert the BSLib error code to an appropriate Python exception type.
 *
 *	@param errCode 	The BSLib error code.
 *
 *	@return 		The appropriate Python exception type for the error code.
 */
PyObject * bslibErrToException( int errCode )
{
	switch( errCode )
	{
		case BSLIB_OK:			return NULL;

		case BSLIB_IO_ERROR:	return PyExc_IOError;
		case BSLIB_MEM_ERROR:	return PyExc_MemoryError;

		case BSLIB_PARAM_ERROR:
		case BSLIB_BZ2_ERROR:
		case BSLIB_BAD_PATCH_ERROR:
								return PyExc_BSLibException;

		default: 				return PyExc_SystemError;
	}
}


/**
 *	_bsdiff.diffFilesToFile( srcPath, dstPath, patchPath )
 * 
 *	Perform a binary diff on the files at the two paths, srcPath -> dstPath, 
 *	and create a patch file at patchPath.
 */
PyObject *
py_bsdiff_diffFilesToFile( PyObject * self, PyObject * args, PyObject * kwargs )
{
	const char * srcPath = NULL;
	const char * dstPath = NULL;
	const char * patchPath = NULL;

	char * kwlist[] = { "srcPath", "dstPath", "patchPath", NULL };
	if (!PyArg_ParseTupleAndKeywords(
				args, kwargs, "sss:diffFilesToFile", kwlist,
				&srcPath, &dstPath, &patchPath ) )
	{
		return NULL;
	}

	// stat the source file
	struct stat srcStat;
	if (0 != stat( srcPath, &srcStat ))
	{
		PyErr_Format( PyExc_ValueError, "Could not stat source path '%s': %s",
			srcPath, strerror( errno ) );
		return NULL;
	}
	if (!S_ISREG( srcStat.st_mode ))
	{
		PyErr_Format( PyExc_ValueError, "source path does not refer to a "
				"regular file: '%s'",
			srcPath );
		return NULL;
	}
	// stat the dest file
	struct stat dstStat;
	if (0 != stat( dstPath, &dstStat ))
	{
		PyErr_Format( PyExc_ValueError, "Could not stat destination path "
				"'%s': %s",
			dstPath, strerror( errno ) );
		return NULL;
	}
	if (!S_ISREG( dstStat.st_mode ))
	{
		PyErr_Format( PyExc_ValueError, "destination path does not refer to a "
				"regular file: '%s'",
			dstPath );
		return NULL;
	}

	// read in the source file
	FILE * srcFile = fopen( srcPath, "rb" );
	if (!srcFile)
	{
		PyErr_Format( PyExc_IOError, "Could not open source path for reading "
					"'%s': %s",
				srcPath, strerror( errno ) );
		return NULL;
	}
	char * srcBuf = new char[srcStat.st_size + 1]; // cater for 0-size files

	if (srcStat.st_size > 0 &&
			(1 != fread( srcBuf, srcStat.st_size, 1, srcFile )))
	{
		PyErr_Format( PyExc_IOError, "Could not read source file '%s'",
			srcPath );
		delete [] srcBuf;
		fclose( srcFile );
		return NULL;
	}
	fclose( srcFile );

	// read in the dest file
	FILE * dstFile = fopen( dstPath, "rb" );
	if (!dstFile)
	{
		PyErr_Format( PyExc_IOError, "Could not open destination path for "
					"reading '%s': %s",
				dstPath, strerror( errno ) );
		return NULL;
	}

	char * dstBuf = new char[dstStat.st_size + 1]; // cater for 0-size files
	if (dstStat.st_size > 0 &&
			(1 != fread( dstBuf, dstStat.st_size, 1, dstFile )))
	{
		PyErr_Format( PyExc_IOError, "Could not read from "
			"destination file '%s'",
			dstPath );
		fclose( dstFile );
		delete [] srcBuf;
		delete [] dstBuf;
		return NULL;
	}
	fclose( dstFile );


	// do the diffs on the byte strings
	int res = bsdiff_diffToPatchFile( srcBuf, srcStat.st_size,
		dstBuf, dstStat.st_size, patchPath );

	delete [] srcBuf;
	delete [] dstBuf;

	if (res != BSLIB_OK)
	{
		PyObject * exception = bslibErrToException( res );
		PyErr_Format( exception, "%s: %s",
			bsdiff_errString( res ), bsdiff_errDescription() );
		return NULL;
	}

	Py_RETURN_NONE;
}

/**
 *	_bsdiff.haveDiffs( path1, path2, bufSize=4096 )
 * 
 *	Return True if the files at the two paths (path1, path2) differ. It first tests the file sizes before doing a byte-by-byte comparison. 
 *	The parameter bufSize is used to control the number of bytes to compare at a time.
 */
PyObject *
py_bsdiff_haveDiffs( PyObject * self, PyObject * args, PyObject * kwargs )
{
	char * path1;
	char * path2;
	long bufSize_l = 4096;
	char* kwlist[] = {"path1", "path2", "bufSize", NULL};
	if (!PyArg_ParseTupleAndKeywords( args, kwargs,
			"ss|l:haveDiffs", kwlist, &path1, &path2, &bufSize_l ))
	{
		return NULL;
	}

	if (bufSize_l <= 0)
	{
		PyErr_Format( PyExc_ValueError, "bufSize must be greater than 0" );
		return NULL;
	}

	unsigned long bufSize = bufSize_l;

	struct stat stat1, stat2;
	if (0 != stat( path1, &stat1 ) )
	{
		PyErr_Format( PyExc_IOError, "Could not stat file '%s': %s",
			path1, strerror( errno ) );
		return NULL;
	}
	if (0 != stat( path2, &stat2 ) )
	{
		PyErr_Format( PyExc_IOError, "Could not stat file '%s': %s",
			path2, strerror( errno ) );
		return NULL;
	}
	if (!S_ISREG( stat1.st_mode ))
	{
		PyErr_Format( PyExc_TypeError, "Not a regular file: '%s'",
			path1 );
		return NULL;
	}
	if (!S_ISREG( stat2.st_mode ) )
	{
		PyErr_Format( PyExc_TypeError, "Not a regular file: '%s'",
			path2 );
		return NULL;
	}

	// compare size reported by stat
	if (stat1.st_size != stat2.st_size)
	{
		Py_RETURN_TRUE;
	}

	FILE * file1 = fopen( path1, "rb" );
	if (file1 == NULL)
	{
		PyErr_Format( PyExc_IOError, "Could not read file '%s': %s",
			path1, strerror( errno ) );
		return NULL;
	}

	FILE * file2 = fopen( path2, "rb" );
	if (file2 == NULL)
	{
		PyErr_Format( PyExc_IOError, "Could not read file '%s': %s",
			path2, strerror( errno ) );
		return NULL;
	}

	char * buf1 = new char[bufSize + 1];
	char * buf2 = new char[bufSize + 1];
	buf1[bufSize] = '\0';
	buf2[bufSize] = '\0';

	do
	{
		size_t readLen1 = 0;
		while (readLen1 < bufSize && !feof( file1 ))
		{
			size_t chunk = fread( buf1 + readLen1, 1,
				bufSize - readLen1, file1 );
			readLen1 += chunk;
		}
		size_t readLen2 = 0;
		while (readLen2 < bufSize && !feof( file2 ))
		{
			size_t chunk = fread( buf2 + readLen2, 1,
				bufSize - readLen2, file2 );
			readLen2 += chunk;
		}

		if (ferror( file1 ))
		{
			PyErr_Format( PyExc_IOError, "read error with '%s': %s",
				path1, strerror( errno ) );
			delete [] buf1;
			delete [] buf2;
			fclose( file1 );
			fclose( file2 );

			return NULL;
		}
		else if (ferror( file2 ))
		{
			PyErr_Format( PyExc_IOError, "read error with '%s': %s",
				path2, strerror( errno ) );
			delete [] buf1;
			delete [] buf2;
			fclose( file1 );
			fclose( file2 );

			return NULL;
		}

		if (memcmp( buf1, buf2, readLen1 ) != 0)
		{
			delete [] buf1;
			delete [] buf2;
			fclose( file1 );
			fclose( file2 );
			Py_RETURN_TRUE;
		}
	} while (!feof( file1 ) && !feof( file2 ));
	delete [] buf1;
	delete [] buf2;
	fclose( file1 );
	fclose( file2 );
	Py_RETURN_FALSE;
}


/**
 *	_bsdiff.patchFileFromFile( srcPath, dstPath, patchPath )
 *
 *	Patch a file at srcPath from patchPath -> dstPath.
 */
PyObject *
py_bsdiff_patchFilesFromFile( PyObject * self, PyObject * args,
		PyObject * kwargs )
{
	const char * srcPath;
	const char * dstPath;
	const char * patchPath;

	char* kwlist[] = {"srcPath", "dstPath", "patchPath", NULL};
	if (!PyArg_ParseTupleAndKeywords( args, kwargs,
			"sss:patchFromFile", kwlist, &srcPath, &dstPath, &patchPath ))
	{
		return NULL;
	}

	FILE * srcFile = fopen( srcPath, "rb" );
	if (!srcFile)
	{
		PyErr_Format( PyExc_IOError, "could not open src '%s': %s",
			srcPath, strerror( errno ) );
		return NULL;
	}

	u_char * dstBuf;
	bs_off_t dstBufLen;

	int res = bsdiff_patchFromPatchFile( patchPath, srcFile,
		&dstBuf, &dstBufLen );
	fclose( srcFile );

	if (res != BSLIB_OK)
	{
		PyObject * exception = bslibErrToException( res );
		PyErr_Format( exception, "%s: %s",
			bsdiff_errString( res ), bsdiff_errDescription() );
		return NULL;
	}

	FILE * dstFile = fopen( dstPath, "wb" );
	if (!dstFile)
	{
		PyErr_Format( PyExc_IOError, "could not open dst '%s': %s",
			dstPath, strerror( errno ) );
		return NULL;
	}
	if (1 != fwrite( (char*)dstBuf, dstBufLen, 1, dstFile ))
	{
		if (ferror( dstFile ))
		{
			PyErr_Format( PyExc_IOError, "error writing out destination file" );
		}
		else
		{
			PyErr_Format( PyExc_IOError, "unknown error writing out "
				"destination file" );
		}
		fclose( dstFile );
		delete [] dstBuf;
		return NULL;
	}
	fclose( dstFile );
	delete [] dstBuf;

	Py_RETURN_NONE;
}


PyMethodDef bsdiff_methods[] = {
	{"haveDiffs", (PyCFunction)py_bsdiff_haveDiffs,
		METH_VARARGS | METH_KEYWORDS,
		"Return True if the files at the two paths (path1, path2) differ."},
	{"diffFilesToFile", (PyCFunction)py_bsdiff_diffFilesToFile,
		METH_VARARGS | METH_KEYWORDS,
		"Perform a binary diff on the files at the two paths, "
		"srcPath -> dstPath, and create a patch file at patchPath."
	},
	{"patchFilesFromFile", (PyCFunction)py_bsdiff_patchFilesFromFile,
		METH_VARARGS | METH_KEYWORDS,
		"Patch a file at srcPath from patchPath -> dstPath." },
	{NULL, NULL, 0, NULL}
};

} // namespace (anonymous)

/**
 *	Python module initialisation function for _bsdiff.
 */
PyMODINIT_FUNC init_bsdiff( void )
{
	PyObject * module = Py_InitModule( "_bsdiff", bsdiff_methods );

	PyExc_BSLibException = PyErr_NewException( "bsdiff.error", NULL, NULL );
	PyModule_AddObject( module, "error", PyExc_BSLibException );
}

// bsdiffmodule.cpp
