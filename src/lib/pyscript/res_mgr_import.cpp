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
#include "res_mgr_import.hpp"

#ifdef USE_RES_MGR_IMPORT_HOOK

#include "resmgr/bin_section.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/multi_file_system.hpp"

// This is for PyMarshal_*, and isn't in Python.h for some reason.
#include "marshal.h"

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

#ifndef CODE_INLINE
#include "res_mgr_import.ipp"
#endif


// -----------------------------------------------------------------------------
// Section: PyResMgrImportHook - Script definition
// -----------------------------------------------------------------------------

/*~ class NoModule.PyResMgrImportHook
 *	@components{ all }
 *	A hook type that operates as a factory callable for PyResMgrImportLoader
 *  instances specialised for a particular entry in sys.path.
 *	For internal BigWorld/Python integration operation.
 *  This type is callable, calls are forwarded to getImporter().
 */
/* We have a "PyObject* _pyCall( PyObject*, PyObject*, PyObject* )" method */
PY_TYPEOBJECT_WITH_CALL( PyResMgrImportHook )

PY_BEGIN_METHODS( PyResMgrImportHook )
	PY_METHOD( getImporter )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyResMgrImportHook )
PY_END_ATTRIBUTES()


// -----------------------------------------------------------------------------
// Section: PyResMgrImportHook - methods
// -----------------------------------------------------------------------------

/**
 *	The constructor for PyResMgrImportHook.
 */
PyResMgrImportHook::PyResMgrImportHook( PyTypePlus * pType ) :
		PyObjectPlus( pType )
{
	// Assume this hook will only be instantiated once. Though, if init() is
	// called twice it's not really a problem.
	PyResMgrImportLoader::init();
}


/**
 *	This method overrides the PyObjectPlus method.
 */
PyObject * PyResMgrImportHook::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method overrides the PyObjectPlus method.
 */
int PyResMgrImportHook::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


// -----------------------------------------------------------------------------
// Section: PyResMgrImportHook - Python callable methods
// -----------------------------------------------------------------------------

/*~ function PyResMgrImportHook.getImporter
 *	@components{ all }
 *
 *	This method returns an instance of PyResMgrImportLoader for the specified
 *  path or None if that path doesn't exist or looks like a folder that
 *  holds C Extensions.
 *
 *	@param	path	A string containing the resource path to produce a loader
 *					for.
 *
 *	@return			PyResMgrImportLoader instance if successful, or None
 *					otherwise.
 */
/**
 *	This method returns a PyResMgrImportLoader if the path exists and is
 *	usable, or Py_None otherwise.
 *
 *	@param path		A string containing the resource path to produce a loader
 *					for.
 */
PyObject * PyResMgrImportHook::getImporter( const std::string & path )
{
//	TRACE_MSG( "PyResMgrImportHook::getImporter: Looking for %s\n",
//		path.c_str() );

	DataSectionPtr pDataSection = BWResource::openSection( path );

	if (!pDataSection)
	{
		PyErr_Format( PyExc_ImportError, "No such path: '%s'", path.c_str() );
		return NULL;
	}

	if (path.find( "DLL" ) != std::string::npos ||
			path.find( "lib-dynload" ) != std::string::npos)
	{
//		WARNING_MSG( "PyResMgrImportHook::getImporter: %s: "
//			"We can't handle C_EXTENSIONS, leaving path for default "
//			"Python import routines\n", path.c_str() );
		Py_RETURN_NONE;
	}

	return new PyResMgrImportLoader( path, pDataSection );
}


// -----------------------------------------------------------------------------
// Section: PyResMgrImportLoader - Static initialisation
// -----------------------------------------------------------------------------

// Initialised by PyResMgrImportLoader::init().
PyResMgrImportLoader::SuffixLookupMap PyResMgrImportLoader::s_suffices_;

// -----------------------------------------------------------------------------
// Section: PyResMgrImportLoader - Script definition
// -----------------------------------------------------------------------------

/*~ class NoModule.PyResMgrImportLoader
 *	@components{ all }
 *	An implementation of the PEP 302 Importer Protocol that loads Python Source
 *  and Python compiled module files from ResMgr, produced by
 *  PyResMgrImportHook.
 *	For internal BigWorld/Python integration operation.
 */
PY_TYPEOBJECT( PyResMgrImportLoader )

PY_BEGIN_METHODS( PyResMgrImportLoader )
	PY_METHOD( find_module )
	PY_METHOD( load_module )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyResMgrImportLoader )
	PY_ATTRIBUTE( path )
PY_END_ATTRIBUTES()


// -----------------------------------------------------------------------------
// Section: PyResMgrImportLoader - General methods
// -----------------------------------------------------------------------------

/**
 *	Initialise the PyResMgrImportLoader class.
 */
/*static*/ void PyResMgrImportLoader::init()
{
//	TRACE_MSG( "PyResMgrImportLoader::init: %s.\n",
//		Py_OptimizeFlag?"Optimized":"Not optimized");

	s_suffices_["py"] = PY_SOURCE;
	s_suffices_[Py_OptimizeFlag ? "pyo" : "pyc"] = PY_OBJECT;

#if MF_SERVER
	// Not yet supported, we can't dlopen a memory block.
	s_suffices_["so"] = C_EXTENSION;
#else // MF_SERVER
	// Not yet supported, we can't load a pyd file unless we are linking to a
	// Python DLL, and as of this writing, we are not doing that.
	// Also, we can't LoadLibraryEx a memory block
	s_suffices_["pyd"] = C_EXTENSION;
#endif // MF_SERVER
}


/**
 *	The constructor for PyResMgrImportLoader
 */
PyResMgrImportLoader::PyResMgrImportLoader( const std::string & path,
		DataSectionPtr pDirectory, PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	path_( path ),
	pDirectory_( pDirectory ),
	lastLookupModuleName_(),
	lastLookupType_( NOT_FOUND )
{
//	TRACE_MSG( "PyResMgrImportLoader(%s)::PyResMgrImportLoader\n",
//		path_.c_str() );
}


/**
 *	This method overrides the PyObjectPlus method.
 */
PyObject* PyResMgrImportLoader::pyGetAttribute( const char* attr )
{
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This method overrides the PyObjectPlus method.
 */
int PyResMgrImportLoader::pySetAttribute( const char* attr, PyObject* value )
{
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


// -----------------------------------------------------------------------------
// Section: PyResMgrImportLoader - Script methods
// -----------------------------------------------------------------------------


/*~ function PyResMgrImportHook.find_module
 *	@components{ all }
 *
 *	This method implements the PEP 302 Importer Protocol's find_module
 *  method, returning a loader (ourselves) that can handle the named
 *  module, or None if we can't handle it.
 *
 *	@param	fullModuleName 	A string containing the full module name to find.
 *
 *	@return					PyResMgrImportLoader instance if found, or None
 *							otherwise.
 */
/**
 *	This method returns ourselves if we are able to load the supplied module
 *	name, or Py_None otherwise.
 *
 *	@param fullModuleName 	A string containing the full module name to find.
 */
PyObject * PyResMgrImportLoader::find_module(
		const std::string & fullModuleName )
{
	// name will be the fully-qualified package name in question, but
	// we are the loader for a given directory (eg. package) so we should
	// only get this if we are already the correct path for a given module.
	// We cache by fully-qualified name though, since that's what we need
	// everywhere else.

//	TRACE_MSG( "PyResMgrImportLoader::find_module(%s): %s\n", path_.c_str(),
//		fullModuleName.c_str() );

	const std::string shortModName = this->getShortModuleName( fullModuleName );

	PythonModuleType moduleType = this->findModuleType( shortModName, path_ );

	if (moduleType == C_EXTENSION)
	{
		ERROR_MSG( "PyResMgrImportLoader::find_module(%s): "
				"Can't load module %s as a C extension\n",
			path_.c_str(), fullModuleName.c_str() );
		Py_RETURN_NONE;
	}
	else if (moduleType == NOT_FOUND)
	{
		Py_RETURN_NONE;
	}

	lastLookupType_ = moduleType;
	lastLookupModuleName_ = fullModuleName;

	Py_INCREF( this );
	return this;
}


/*~ function PyResMgrImportHook.load_module
 *	@components{ all }
 *
 *	This method implements the PEP 302 Importer Protocol's load_module
 *  method, importing and returning the named module if possible or None
 *  if we can't handle it.
 *
 *	@param	fullModuleName 	A string containing the full module name to import.
 *
 *	@return					PyResMgrImportLoader instance if successful, or
 *							None otherwise.
 */
/**
 *	This method returns ourselves if we are able to load the named module,
 *  or Py_None otherwise.
 *
 *	@param fullModuleName 	A string containing the full module name to import.
 *
 *	@return					PyResMgrImportLoader instance if successful, or
 *							None otherwise.
 */
PyObject * PyResMgrImportLoader::load_module(
		const std::string & fullModuleName )
{
	//  According to PEP 302, load_module has a number of responsibilities:
	//  Note that loadPackage hands off to another module loader for handling
	//  __init__, which will happily overwrite supplied values where it sees
	//  fit.
	//  * Module must be added to sys.modules before loading, and if there is
	//    already one there, use it.
	//   - loadPackage: We call PyImport_AddModule
	//   - loadCompiledModule: We call PyImport_ExecCodeModuleEx
	//   - loadSourceModule: We call PyImport_ExecCodeModuleEx
	//  * __file__ must be set
	//   - loadPackage: We set it to the given file name
	//   - loadCompiledModule: We pass it to PyImport_ExecCodeModuleEx
	//   - loadSourceModule: We pass it to PyImport_ExecCodeModuleEx
	//  * __name__ must be set (PyImport_AddModule handles this)
	//   - loadPackage: We call PyImport_AddModule
	//   - loadCompiledModule: We call PyImport_ExecCodeModuleEx
	//   - loadSourceModule: We call PyImport_ExecCodeModuleEx
	//  * __path__ must be a list, if it's a package
	//   - loadPackage: We do this by hand, it gets sent back to getImporter
	//  * __loader__ should be set to the loader
	//   - loadPackage: We do this
	//   - loadCompiledModule: We do this
	//   - loadSourceModule: We do this

//	TRACE_MSG( "PyResMgrImportLoader(%s)::load_module: %s\n", path_.c_str(),
//		fullModuleName.c_str() );

	if (lastLookupModuleName_ != fullModuleName)
	{
		// We weren't called with the same module name as the lastcall to
		// find_module, so re-call find_module before we proceed.

		PyObject * pLoader = this->find_module( fullModuleName );

		if (!pLoader)
		{
			PyErr_Format( PyExc_ImportError, "loader cannot load module '%s'",
				fullModuleName.c_str() );
			return NULL;
		}
		Py_DECREF( pLoader );
	}

	MF_ASSERT( lastLookupType_ != NOT_FOUND );

	PythonModuleType lookupType = lastLookupType_;

	// Clear cached result
	lastLookupType_ = NOT_FOUND;
	lastLookupModuleName_.clear();

	switch (lookupType)
	{
	case PKG_DIRECTORY:
		return this->loadPackage( fullModuleName );
	case PY_OBJECT:
		return this->loadCompiledModule( fullModuleName, lookupType,
				pDirectory_ );
	case PY_SOURCE:
		return this->loadSourceModule( fullModuleName, lookupType,
				pDirectory_ );
	case NOT_FOUND:
	case C_EXTENSION:
		break;
	}

	Py_RETURN_NONE;
}


/**
 * This method imports the named package into Python and returns it.
 *
 *	@param	fullModuleName 	The full name of the python module to load.
 *
 *	@return	The imported package
 */
PyObject * PyResMgrImportLoader::loadPackage(
		const std::string & fullModuleName )
{
	// This method needs to emulate loadPackage in import.c in Python, but
	// from a DataSectionPtr

	const std::string shortModName = this->getShortModuleName( fullModuleName );
	const std::string moduleDir = path_ + "/" + shortModName;
	PyObject * pModule = PyImport_AddModule( fullModuleName.c_str() );

	if (pModule == NULL)
	{
		// Propagate the PyErr up
		return NULL;
	}

	// Borrowed reference
	PyObject * pModuleDict = PyModule_GetDict( pModule );
	PyObjectPtr pModulePath( PyString_FromString( moduleDir.c_str() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pModulePath)
	{
		return NULL;
	}

	PyObjectPtr pPaths( Py_BuildValue( "[O]", pModulePath.get() ),
			PyObjectPtr::STEAL_REFERENCE );

	if (!pPaths)
	{
		return NULL;
	}

	if (0 != PyDict_SetItemString( pModuleDict, "__file__",
				pModulePath.get() ))
	{
		return NULL;
	}

	if (0 != PyDict_SetItemString( pModuleDict, "__path__", pPaths.get() ))
	{
		return NULL;
	}

	if (0 != PyDict_SetItemString( pModuleDict, "__loader__", this ))
	{
		return NULL;
	}

//	TRACE_MSG( "PyResMgrImportLoader(%s)::load_package: processing %s\n",
//		path_.c_str(), fullModuleName.c_str() );

	// This call was tested in find_module_file earlier.
	PythonModuleType initType = this->findModuleType( "__init__", moduleDir );
	DataSectionPtr pDirSection = pDirectory_->findChild( shortModName );

	switch (initType)
	{
	case PY_OBJECT:
		return this->loadCompiledModule( fullModuleName, PKG_DIRECTORY,
				pDirSection );
	case PY_SOURCE:
		return this->loadSourceModule( fullModuleName, PKG_DIRECTORY,
				pDirSection );
	case NOT_FOUND:
	case PKG_DIRECTORY:
	case C_EXTENSION:
		break;
	}
	Py_RETURN_NONE;
}


/**
 *	This method imports the named Python compiled module into Python and
 *	returns it.
 *
 *	@param fullModuleName 	The fully qualified name of the python module
 *							to find.
 *	@param modType			The module type.
 *	@param pDirectory		The data section of the containing directory.
 *	@return	The imported package.
 */
PyObject * PyResMgrImportLoader::loadCompiledModule(
		const std::string & fullModuleName,
		PythonModuleType modType,
		DataSectionPtr pDirectory )
{
	// This method needs to emulate load_compiled_module in import.c in Python,
	// but with DataSectionPtr instead of using FILE *.

	const std::string shortModName = this->getShortModuleName( fullModuleName );
	std::string moduleCompiledFilename;
	std::string moduleCompiledPath;
	std::string moduleSourceFilename;
	std::string moduleSourcePath;
	this->getSourceAndCompiledModulePaths( shortModName,
			modType,
			moduleCompiledFilename, moduleCompiledPath,
			moduleSourceFilename, moduleSourcePath );

	DataSectionPtr pModuleSection = pDirectory->findChild(
			moduleCompiledFilename );
	MF_ASSERT( pModuleSection );

	return this->loadCompiledModuleFromBinary( fullModuleName, modType,
			pModuleSection->asBinary() );
}


/**
 *	This method imports the named Python Source into Python and returns it.
 *	If an up-to-date Python compiled module is found, that will be used
 *	instead. Otherwise, it will write out a Python compiled module file if the
 *	import of the source file is successful.
 *
 *	@param fullModuleName 	The name of the python module to find.
 *	@param modType 			The module type.
 *	@param pDirectory		A DataSection where the related compiled module or
 *							source module might be found.
 *	@return					The imported module object.
 */
PyObject * PyResMgrImportLoader::loadSourceModule(
		const std::string & fullModuleName, PythonModuleType modType,
		DataSectionPtr pDirectory )
{
	// This method needs to emulate loadSourceModule in import.c in python, but
	// with DataSectionPtr instead of FILE *.
	const std::string shortModName = this->getShortModuleName( fullModuleName );
	std::string moduleCompiledFilename;
	std::string moduleCompiledPath;
	std::string moduleSourceFilename;
	std::string moduleSourcePath;
	this->getSourceAndCompiledModulePaths( shortModName,
			modType,
			moduleCompiledFilename, moduleCompiledPath,
			moduleSourceFilename, moduleSourcePath );

	// Fetch mtime of py file
	time_t pyModTime = this->getFileModTime( moduleSourcePath );

	// If possible, palm this off to loadCompiledModule.
	DataSectionPtr pycSection = pDirectory->findChild( moduleCompiledFilename );

	if (pycSection)
	{
		BinaryPtr pycData = pycSection->asBinary();

		if (this->isCompiledModule( pycData ) &&
				this->isCompiledModuleUpToDate( pycData, pyModTime ))
		{
			// We know the module was valid and up-to-date, so trust the loader
			// to either load it or fail noisily.
			return this->loadCompiledModuleFromBinary( fullModuleName, modType,
					pycData, /* knownValid: */ true );
		}
	}

	if (pycSection)
	{
		// Get rid of our reference to the old compiled python file, and clear
		// the cache.
		pycSection = NULL;
		BWResource::instance().purge( moduleCompiledPath );
	}

	// Remove the potential cached copies of the source file and re-read the
	// .py source file.
	BWResource::instance().purge( moduleSourcePath );
	DataSectionPtr pSourceSection = pDirectory->findChild(
		moduleSourceFilename );

	if (!pSourceSection)
	{
		PyErr_Format( PyExc_ImportError,
			"Could not open module %s from '%s'\n",
			fullModuleName.c_str(),
			moduleSourcePath.c_str() );
		return NULL;
	}

	// We got here, the object file for this source either doesn't exist, isn't
	// valid, or isn't as recent as the source.
	// Emulate parse_source_module
	// Also, need to ensure there's no embedded nulls.
	// So have to make a copy of the string.
	// We shouldn't ever do this in release anyway.
	BinaryPtr pySourceData = pSourceSection->asBinary();
	std::string codeString( pySourceData->cdata(), pySourceData->len() );

	if (codeString.find( '\0' ) != std::string::npos)
	{
		PyErr_Format( PyExc_ImportError,
			"%s contains an embedded null character",
			moduleSourcePath.c_str() );
		return NULL;
	}

	this->tidyModuleCodeString( codeString );

	PyObjectPtr pCodeObject(
		Py_CompileString( codeString.c_str(),
			const_cast< char* >( moduleSourcePath.c_str() ),
			Py_file_input ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pCodeObject)
	{
		// Compiler didn't like it. Propagate the error up
		return NULL;
	}

	PyObject * pModule = this->importModuleFromCodeObject( pCodeObject.get(),
			fullModuleName, moduleSourcePath );

	if (pModule)
	{
		// It executed OK, so write out an object file for later use, if
		// possible.
		if (!this->writeModuleCodeObject( pDirectory, moduleCompiledFilename,
				pCodeObject.get(), pyModTime ))
		{
			WARNING_MSG( "PyResMgrImportLoader::writeModuleCodeObject(%s): "
					"Could not write compiled module %s to %s\n",
				path_.c_str(), fullModuleName.c_str(),
				moduleCompiledPath.c_str() );
		}
	}

	return pModule;
}


/**
 *	This method checks that the named module's Python compiled module data is
 *	valid, at least as far as the file header is concerned.
 *
 *	This method will not set a Python error if the object is invalid.
 *
 *	@param	pycData		A BinaryPtr holding the data from the Python compiled
 *						module.
 *	@return				Whether or not the file is considered valid and
 *						up-to-date.
 */
/*static*/ bool PyResMgrImportLoader::isCompiledModule( BinaryPtr pycData )
{
	// Check it's not too short
	if (pycData->len() < 8)
	{
		return false;
	}
	// Check we have PYC magic

	// PyImport_GetMagicNumber() returns a long, but the on-disk format is 4
	// bytes. If we don't do this unsigned, there's a sign-extension risk. So
	// we just truncate to 32-bits instead.
	// The current value as of Python 2.5.2 has the high-bit unset, and that
	// should never change.
	// If you decide to customise the PYC storage format, make sure you change
	// the magic number.

	// XXX: On-disk format is little-endian, we're not checking that here
	uint32 dataMagic = reinterpret_cast< const uint32* >( pycData->data() )[ 0 ];
	uint32 realMagic = static_cast< const uint32 >( PyImport_GetMagicNumber() );

	return (dataMagic == realMagic);
}


/**
 *	This method checks that the named module's Python compiled module data has
 *	the same mtime as the requested mtime.
 *
 *	This function does not set a python error if it is invalid.
 *
 *	@param	pycData		A BinaryPtr holding the data from the Python compiled
 *						module.
 *	@param	mtime		The mtime to test against.
 *	@return				True if the file is considered up-to-date, false
 *						otherwise.
 */
/*static*/ bool PyResMgrImportLoader::isCompiledModuleUpToDate(
		BinaryPtr pycData, time_t mTime )
{
	MF_ASSERT_DEBUG( PyResMgrImportLoader::isCompiledModule( pycData ) );
	const int32 pycModTime =
		(reinterpret_cast< const int32* >(pycData->data()))[ 1 ];

//	TRACE_MSG( "PyResMgrImportLoader::isCompiledModuleUpToDate: "
//			"mTime 0x%016ld with pyc 0x%08x\n",
//		mTime, pycModTime );

	// .pyc files only have four bytes to store their .py file's modification
	// time.
	const int32 truncatedModTime = static_cast< const int32 >( mTime );

	// XXX: On-disk format is little-endian, we're not checking that here.
	return (pycModTime == truncatedModTime);
}


/**
 *	This method fills the given strings (passed by reference) with the paths of
 *	the associated module, expected to be located at this loader's path and
 *	also to have an associated entry in the cache.
 *
 *	@param shortModName 			The module name.
 *	@param moduleCompiledFilename	The filename of the compiled module file.
 *	@param moduleCompiledPath		The resource path of the compiled module
 *									file.
 *	@param moduleSourceFilename		The filename of the source module file.
 *	@param moduleSourcePath			The resource path of the source module
 *									file.
 */
void PyResMgrImportLoader::getSourceAndCompiledModulePaths(
		const std::string & shortModName, PythonModuleType modType,
		std::string & moduleCompiledFilename,
		std::string & moduleCompiledPath,
		std::string & moduleSourceFilename,
		std::string & moduleSourcePath ) const
{
	MF_ASSERT( modType != NOT_FOUND );

	// Find source (.py) and object (.pyc/.pyo) files to
	// process for this source module or package.

	const std::string compiledExtension( Py_OptimizeFlag ? ".pyo" : ".pyc" );

	if (modType == PKG_DIRECTORY)
	{
		moduleCompiledFilename = "__init__" + compiledExtension;
		moduleSourceFilename = "__init__.py";
		moduleCompiledPath = path_ + "/" + shortModName + "/" +
			moduleCompiledFilename;
		moduleSourcePath = path_ + "/" + shortModName + "/" +
			moduleSourceFilename;
	}
	else
	{
		moduleCompiledFilename = shortModName + compiledExtension;
		moduleSourceFilename =  shortModName + ".py";
		moduleCompiledPath = path_ + "/" + moduleCompiledFilename;
		moduleSourcePath = path_ + "/" + moduleSourceFilename;
	}
}


/**
 *	This method returns the path to the appropriate compiled module file for
 *	the given module name (which must be already cached).
 *
 *	@param fullModuleName	The full module name.
 *	@param modType 			The module type.
 *
 *	@return 	The path to the appropriate compiled module file for that
 *				module name.
 */
const std::string PyResMgrImportLoader::getModuleCompiledPath(
		const std::string & shortModName, PythonModuleType modType ) const
{
	MF_ASSERT( modType != NOT_FOUND );

	const std::string compiledExtension( Py_OptimizeFlag ? ".pyo" : ".pyc" );

	if (modType == PKG_DIRECTORY)
	{
		return path_ + "/" + shortModName + "/__init__" + compiledExtension;
	}
	else
	{
		return path_ + "/" + shortModName + compiledExtension;
	}
}


/**
 *	This method writes out a module's compiled module (.pyc or .pyo) out to the
 *	specified path.
 *
 *	@param pDirectory			The directory datasection to save the module
 *								file into.
 *	@param compiledFilename 	The filename to write the compiled module file
 *								out to.
 *	@param pCodeObject			The module's code object.
 *	@param pyModTime			The source file's modification time.
 *
 *	@return 					True on success, false on error (python
 *								exception not set).
 */
bool PyResMgrImportLoader::writeModuleCodeObject( DataSectionPtr pDirectory,
		const std::string & compiledFilename,
		PyObject * pCodeObject,	time_t pyModTime )
{
	// Emulates write_compiled_module( co, cpathname, mtime )

	PyObjectPtr pMarshalledCodeString( PyMarshal_WriteObjectToString(
			pCodeObject, Py_MARSHAL_VERSION ),
		PyObjectPtr::STEAL_REFERENCE );

	if (pMarshalledCodeString == NULL ||
			!PyString_Check( pMarshalledCodeString.get() ))
	{
		ERROR_MSG( "PyResMgrImportLoader::loadSourceModule: "
				"Could not marshal module code while "
				"writing out compiled module to '%s/%s'\n",
			path_.c_str(), compiledFilename.c_str() );
		PyErr_Clear();
		return false;
	}

	// .pyc header is 4-byte magic number and 4 byte modification time
	const size_t dataBlockLen = 8 +
		PyString_Size( pMarshalledCodeString.get() );
	char * dataBlock = new char[ dataBlockLen ];

	// XXX: On-disk format is little-endian, we're not checking that here
	reinterpret_cast< uint32* >(dataBlock)[ 0 ] = PyImport_GetMagicNumber();
	reinterpret_cast< int32* >(dataBlock)[ 1 ] =
		static_cast< int32 >( pyModTime );
	memcpy( dataBlock + 8, PyString_AsString( pMarshalledCodeString.get() ),
		PyString_Size( pMarshalledCodeString.get() ) );

	// The following is a little nasty, we end up copying the data a couple
	// of times
	// Wrap dataBlock in a BinaryBlock (which takes a copy of it)
	BinaryPtr pycData = new BinaryBlock( dataBlock,	dataBlockLen,
		"PyResMgrImportLoader::loadSourceModule" );
	delete[] dataBlock;

	if (!pycData)
	{
		return false;
	}

	// Save out our new pyc file
	DataSectionPtr pycSection = pDirectory->openSection( compiledFilename,
		/* makeNewSection */ true, BinSection::creator() );

	if (!pycSection)
	{
		NOTICE_MSG( "PyResMgrImportLoader::writeModuleCodeObject( %s ): "
					"Could not open %s for writing\n",
			path_.c_str(), compiledFilename.c_str() );
		return false;
	}

	pycSection->setBinary( pycData );
	pycSection->save();

	return true;
}


/**
 *	Import a module from a code object.
 *
 *	@param pCodeObject		The code object.
 *	@param fullModuleName	The full module name.
 *	@param modulePath		The path to the module.
 *
 *	@return 	The module, or NULL with the Python error state set.
 */
PyObject * PyResMgrImportLoader::importModuleFromCodeObject(
		PyObject * pCodeObject, const std::string & fullModuleName,
		const std::string & modulePath )
{
	if (!PyCode_Check( pCodeObject ))
	{
		PyErr_Format( PyExc_ImportError,
				"module file from '%s' is a non-code object",
			modulePath.c_str() );
		return NULL;
	}

	// OK, we have a module, now we just execute it into the correct space.
	// Always call it a .py, even though we've created a .pyc
	PyObject * pModule = PyImport_ExecCodeModuleEx(
		const_cast< char* >( fullModuleName.c_str() ),
		pCodeObject,
		const_cast< char* >( modulePath.c_str() )	);

	if (!pModule)
	{
		return NULL;
	}

	if (0 != PyObject_SetAttrString( pModule, "__loader__", this ))
	{
		Py_DECREF( pModule );
		return NULL;
	}

//	TRACE_MSG( "PyResMgrImportLoader(%s)::importModuleFromCodeObject: "
//				"loaded module %s from '%s'\n",
//			path_.c_str(), fullModuleName.c_str(), modulePath.c_str() );

	return pModule;
}


/**
 *	This method imports the named Python compiled module into Python from the
 *	given module binary data and returns it.
 *
 *	@param name 			The full name of the Python module to load.
 *	@param modType			The module type.
 *	@param pModuleSection 	The module data section.
 *	@param knownValid		Optional boolean indicating whether the contents of
 *							pModuleSection have been validated already.
 *
 *	@return 	The module, or NULL with the Python error state set.
 */
PyObject * PyResMgrImportLoader::loadCompiledModuleFromBinary(
		const std::string & fullModuleName, PythonModuleType modType,
		BinaryPtr pycData, bool knownValid /* = false */ )
{
	const std::string shortModName = this->getShortModuleName( fullModuleName );
	const std::string moduleCompiledPath =
		this->getModuleCompiledPath( shortModName, modType );

	if (!knownValid && !this->isCompiledModule( pycData ))
	{
		PyErr_Format( PyExc_ImportError,
			"%s is not a valid Python compiled module file",
			moduleCompiledPath.c_str() );
		return NULL;
	}

	// The first four bytes are magic, the second four bytes are the source
	// modification date.
	// This does the same thing as read_compiled_module in import.c
	PyObjectPtr pCodeObject( PyMarshal_ReadObjectFromString(
			pycData->cdata() + 8, pycData->len() - 8 ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pCodeObject)
	{
		return NULL;
	}

	return this->importModuleFromCodeObject( pCodeObject.get(), fullModuleName,
		   moduleCompiledPath );
}


/**
 *	This static method identifies a Python module for the given name in the
 *	supplied path, returning the most appropriate type of module.
 *
 *	@param shortModName 	The unqualified name of the python module to
 *							find.
 *	@param path 			The directory to locate the module in.
 *
 *	@return		The PythonModuleType corresponding to the most appropriate
 *				module file.
 */
/*static*/
PyResMgrImportLoader::PythonModuleType PyResMgrImportLoader::findModuleType(
		const std::string & shortModName, const std::string & path )
{
	PythonModuleType result = NOT_FOUND;
	const std::string moduleDir = path + "/" + shortModName;
	MultiFileSystemPtr pFileSystem = BWResource::instance().fileSystem();
	IFileSystem::FileType dirFileType = pFileSystem->getFileType( moduleDir );

	if (dirFileType == IFileSystem::FT_DIRECTORY)
	{
		PythonModuleType initResult = PyResMgrImportLoader::findModuleType(
				"__init__", moduleDir );

		if (initResult != NOT_FOUND && initResult != PKG_DIRECTORY)
		{
			return PKG_DIRECTORY;
		}
	}

	// OK, module is not a package, search through known module file suffices.
	SuffixLookupMap::const_iterator iSuffix = s_suffices_.begin();

	while (iSuffix != s_suffices_.end())
	{
		IFileSystem::FileType fileType = pFileSystem->getFileType(
				path + "/" + shortModName + "." + iSuffix->first );

		if (fileType != IFileSystem::FT_NOT_FOUND)
		{
			// This shouldn't happen...
			MF_ASSERT( iSuffix->second != PKG_DIRECTORY );

			// We have a match, check if it's a better match than any known
			// match.
			if (iSuffix->second > result)
			{
				result = iSuffix->second;
			}
		}
		++iSuffix;
	}

	return result;
}


/**
 *	This method returns the given resource path's modification time.
 *
 *	@param path 	The resource path.
 *	@return 		The modification time.
 */
/*static*/ time_t PyResMgrImportLoader::getFileModTime(
		const std::string & path )
{
	time_t modTime = static_cast< time_t >( -1 );
	IFileSystem::FileInfo fInfo;
	IFileSystem::FileType fType =
		BWResource::instance().fileSystem()->getFileType( path,
			&fInfo );

	if (fType != IFileSystem::FT_NOT_FOUND)
	{
		modTime = fInfo.modified;
	}
	return modTime;
}


/**
 *	This method does some cleanup (in-place) of the given module codestring
 *	before it is executed.
 *
 *	@param codeString 	The string containing module source code to be
 *						executed.
 */
/*static*/ void PyResMgrImportLoader::tidyModuleCodeString(
		std::string & codeString )
{
	// The code string needs to have (\n) as a line seperator, and needs an EOF
	// (-1) or null termination, and has to end in a newline.

	std::string::size_type winNLpos;
	// Convert any Windows newlines into UNIX newlines
	if ((winNLpos = codeString.find( "\r\n", 0 ) ) != std::string::npos)
	{
		do
		{
			codeString.replace( winNLpos, 2, "\n" );
			winNLpos = codeString.find( "\r\n", winNLpos );
		} while (winNLpos != std::string::npos);
	}

	// Ensure we're newline-terminated
	codeString.append( "\n" );
}


#endif // USE_RES_MGR_IMPORT_HOOK

// res_mgr_import.cpp
