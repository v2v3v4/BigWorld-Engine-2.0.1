/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_OUTPUT_WRITER_HPP
#define PY_OUTPUT_WRITER_HPP

#include "pyobject_plus.hpp"
#include "script.hpp"

#include <stdio.h>
#include <fstream>

/*~ class NoModule.PyOutputWriter
 *	@components{ all }
 *  This class is used by BigWorld to implement a Python object that redirects
 *  Python's stdout and stderr streams to Dev Studio's output window and a log
 *  file which defaults to Python.log. The BigWorld client actually uses an
 *  instance of a subclass of this called BWOutputWriter, which also sends
 *  stdout and stderr to the BigWorld Python colsole. This is automatically set
 *  up by the engine, and neither of these classes nor any instances of them
 *  can be accessed via script.
 */
/**
 *	This class implements a Python object that is used to redirect Python's
 *	stdout and stderr to Dev Studio's output window.
 */
class PyOutputWriter : public PyObjectPlus
{
	Py_Header( PyOutputWriter, PyObjectPlus )

public:
	PyOutputWriter( const char * fileText,
		bool shouldWritePythonLog,
		PyTypePlus * pType = &PyOutputWriter::s_type_ );

	virtual ~PyOutputWriter();

	static void fini();

	static void logFile( std::ostream* outFile ) { s_outFile_ = outFile; }
	static void logToFile( const std::string& msg );

	static void flush();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_METHOD_DECLARE( py_write )
	PY_METHOD_DECLARE( py_flush )

	PY_RW_ATTRIBUTE_DECLARE( softspace_, softspace )

	static bool overrideSysMembers( bool shouldWritePythonLog );

protected:
	virtual void printMessage( const std::string & msg );

private:
	PyOutputWriter( const PyOutputWriter& );
	PyOutputWriter& operator=( const PyOutputWriter& );

	bool				softspace_;
	bool				shouldWritePythonLog_;
	std::string			msg_;

	static FILE*		 s_file_;
	static int			 s_fileRefCount_;
	static std::ostream* s_outFile_;
};


/**
 *	This is a helper class to substitute strings
 */
class PyInputSubstituter
{
public:
	static std::string substitute( const std::string & line,
		PyObject* pModule = NULL, const char * funcName = "expandMacros" );
};

#ifdef CODE_INLINE
#include "py_output_writer.ipp"
#endif

#endif // PY_OUTPUT_WRITER_HPP
