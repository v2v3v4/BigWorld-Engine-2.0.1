/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "cstdmf/bw_util.hpp"
#include "py_output_writer.hpp"
#include "personality.hpp"

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

#ifndef CODE_INLINE
#include "py_output_writer.ipp"
#endif

/*static*/ FILE *	PyOutputWriter::s_file_ = NULL;
/*static*/ int		PyOutputWriter::s_fileRefCount_ = 0;
/*static*/ std::ostream*	PyOutputWriter::s_outFile_ = NULL;

// -----------------------------------------------------------------------------
// Section: PyOutputWriter
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyOutputWriter )

/*~ function PyOutputWriter.write
 *	@components{ all }
 *
 *  Write a string to this writer's outputs. The Python io system calls this.
 *  @param string The string to write.
 *  @return None
 */
PY_BEGIN_METHODS( PyOutputWriter )
	PY_METHOD( write )
	PY_METHOD( flush )
PY_END_METHODS()

/*~ attribute PyOutputWriter.softspace
 *	@components{ all }
 *
 *  This is required for use by the Python io system so
 *  that instances of PyOutputWriter can be used as streams.
 *  @type Read-Write String
 */
PY_BEGIN_ATTRIBUTES( PyOutputWriter )
	PY_ATTRIBUTE( softspace )
PY_END_ATTRIBUTES()



/**
 *	Constructor
 */
PyOutputWriter::PyOutputWriter( const char * fileText,
		bool shouldWritePythonLog,
		PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	softspace_( false ),
	shouldWritePythonLog_( shouldWritePythonLog )
{
	if (shouldWritePythonLog_)
	{
		if (s_fileRefCount_ == 0)
		{
			if (s_outFile_ == NULL) // If a log file has not been specified then create one
			{
				const char * PYTHON_LOG = "python.log";

				s_file_ = bw_fopen( PYTHON_LOG, "a" );

				if (s_file_ != NULL)
				{
					fprintf( s_file_,
						"\n/------------------------------------------------------------------------------\\\n" );

					if (fileText)
					{
						fprintf( s_file_, "%s", fileText );
					}
				}
				else
				{
					ERROR_MSG( "PyOutputWriter::PyOutputWriter: Could not open '%s'\n",
						PYTHON_LOG );
				}
			}
		}

		s_fileRefCount_++;
	}
}


/**
 *	Destructor
 */
PyOutputWriter::~PyOutputWriter()
{
	if ((shouldWritePythonLog_) && (--s_fileRefCount_ == 0))
	{
		fini();
	}
}

/*static*/ void PyOutputWriter::fini()
{
	flush();
	if ((s_outFile_ == NULL) && (s_file_ != NULL))
	{
		fprintf( s_file_, "\\--------------------------------------------------------------------------------/\n" );
		fclose( s_file_ );
		s_file_ = NULL;
		s_fileRefCount_ = 0;
	}
}

/**
 *	This static method is used to flush the log file.
 */
/*static*/ void PyOutputWriter::flush()
{
	if (s_outFile_ != NULL)
	{
		s_outFile_->flush();
	}
	else if (s_file_ != NULL)
	{
		fflush( s_file_ );
	}
}


/**
 *	This method implements the Python flush method.
 */
PyObject * PyOutputWriter::py_flush( PyObject * args )
{
	PyOutputWriter::flush();

	Py_Return;
}


/**
 *	This method returns the attributes associated with this object.
 */
PyObject * PyOutputWriter::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method sets the attributes associated with this object.
 */
int PyOutputWriter::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	This method implements the Python write method. It is used to redirect the
 *	write calls to this object's printMessage method.
 */
PyObject * PyOutputWriter::py_write( PyObject * args )
{
	char * msg = NULL;
	Py_ssize_t msglen = 0;

	if (!PyArg_ParseTuple( args, "s#", &msg, &msglen ))
	{
		ERROR_MSG( "PyOutputWriter::py_write: Bad args\n" );
		return NULL;
	}

	if (memchr( msg, '\0', msglen ))
	{
		PyObject * pRepr = PyObject_Repr( PyTuple_GET_ITEM( args, 0 ) );
		this->printMessage( std::string( PyString_AsString( pRepr ),
										 PyString_GET_SIZE( pRepr ) ) );
		Py_DECREF( pRepr );
		Py_Return;
	}

	this->printMessage( std::string( msg, msglen ) );
	Py_Return;
}


/**
 *	This method implements the default behaviour for printing a message. Derived
 *	class should override this to change the behaviour.
 */
void PyOutputWriter::printMessage( const std::string & msg )
{
	msg_ += msg;
	if (!msg_.empty() && msg_[ msg_.size() - 1 ] == '\n')
	{
		// This is done so that the hack to prefix the time in cell and the base
		// applications works (needs a \n in the format string).
		msg_.resize( msg_.size() - 1 );
		SCRIPT_MSG( "%s\n", msg_.c_str() );
		msg_ = "";
	}

	PyOutputWriter::logToFile( msg );
}


/**
 *	This static method overrides the stdout and stderr members of the sys module
 *	with a new PyOutputWriter.
 */
bool PyOutputWriter::overrideSysMembers( bool shouldWritePythonLog )
{
	PyObject * pSysModule = PyImport_ImportModule( "sys" );

	if (pSysModule != NULL)
	{
		PyObject * pOutputWriter =
				new PyOutputWriter( "", shouldWritePythonLog );
		PyObject_SetAttrString( pSysModule, "stdout", pOutputWriter );
		PyObject_SetAttrString( pSysModule, "stderr", pOutputWriter );
		Py_DECREF( pOutputWriter );

		Py_DECREF( pSysModule );
	}

	return true;
}

/**
 *	This static method performs actual file writes the appropriate
 *	log file.
 */
void PyOutputWriter::logToFile( const std::string& msg )
{
	if (s_outFile_ != NULL)
	{
		*s_outFile_ << "SCRIPT: " << msg << std::endl;
	}
	else if (s_file_ != NULL)
	{
		fprintf( s_file_, "%s", msg.c_str() );
	}
}


// -----------------------------------------------------------------------------
// Section: PyInputSubstituter
// -----------------------------------------------------------------------------


/**
 *	Perform dollar substitution on this line, using the named function from the
 *	provided module.  If the module isn't provided, the personality module is
 *	used.
 */
std::string PyInputSubstituter::substitute( const std::string & line,
	PyObject* pModule, const char * funcName )
{
	// Use the personality module if none provided
	if (pModule == NULL)
	{
		pModule = Personality::instance();
		if (pModule == NULL)
			return line;
	}

	PyObjectPtr pFunc = PyObjectPtr(
		PyObject_GetAttrString( pModule, funcName ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pFunc)
		return line;

	if (!PyCallable_Check( pFunc.getObject() ))
	{
		PyErr_Format( PyExc_TypeError,
			"Macro expansion function '%s' is not callable", funcName );
		PyErr_Print();
		return "";
	}

	PyObjectPtr pExpansion = PyObjectPtr(
		PyObject_CallFunction( pFunc.getObject(), "s", line.c_str() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pExpansion == NULL)
	{
		PyErr_Print();
		return "";
	}

	else if (!PyString_Check( pExpansion.getObject() ))
	{
		PyErr_Format( PyExc_TypeError, "Macro expansion returned non-string" );
		PyErr_Print();
		return "";
	}

	else
	{
		return std::string( PyString_AsString( pExpansion.getObject() ) );
	}
}

// py_output_writer.cpp
