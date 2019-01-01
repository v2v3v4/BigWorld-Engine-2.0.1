/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RES_MGR_IMPORT_HPP
#define RES_MGR_IMPORT_HPP

#if !BWCLIENT_AS_PYTHON_MODULE
#define USE_RES_MGR_IMPORT_HOOK
#endif // !BWCLIENT_AS_PYTHON_MODULE

#ifdef USE_RES_MGR_IMPORT_HOOK

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "resmgr/datasection.hpp"

/**
 *	This class creates PyResMgrImportLoaders specialised to a directory in the
 *	Python script tree.
 */
class PyResMgrImportHook : public PyObjectPlus
{
	Py_Header( PyResMgrImportHook, PyObjectPlus );

public:
	PyResMgrImportHook( PyTypePlus * pType = &PyResMgrImportHook::s_type_ );

	/** Destructor. */
	virtual ~PyResMgrImportHook() {}

	// PyObjectPlus overrides
	virtual PyObject *	pyGetAttribute( const char * attr );
	virtual int			pySetAttribute( const char * attr, PyObject * value );

	// Return a PyResMgrImportLoader specialised to the given path
	PyObject * getImporter( const std::string & path );
	PY_AUTO_METHOD_DECLARE( RETOWN, getImporter, ARG( std::string, END ) );

	/**
	 *	This method routes a call on an instance to getImporter().
	 */
	static PyObject * _pyCall( PyObject * self, PyObject * args, PyObject * kw )
	{ return _py_getImporter( self, args, kw ); }

};


/**
 *	This class locates and loads Python modules. It is both loader and importer
 *	as per PEP 302.
 */
/*
 *	The optional extensions to the loader protocol in PEP 302 are not yet
 *	implemented.
 *	loader.get_data isn't really useful, ResMgr already handles this better.
 *	loader.is_package, get_code and get_source could be useful and easy.
 */
class PyResMgrImportLoader : public PyObjectPlus
{
	Py_Header( PyResMgrImportLoader, PyObjectPlus );

public:
	static void init();

	PyResMgrImportLoader( const std::string & path, DataSectionPtr pDirectory,
		PyTypePlus * pType = &PyResMgrImportLoader::s_type_ );

	/** Destructor. */
	virtual ~PyResMgrImportLoader() {}

	// PyObjectPlus overrides
	virtual PyObject *	pyGetAttribute( const char * attr );
	virtual int			pySetAttribute( const char * attr, PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( path_, path )

	// Python Module Importer interface
	/* Check that we have a source for a given module name */
	PyObject * find_module( const std::string & path );
	PY_AUTO_METHOD_DECLARE( RETOWN, find_module, ARG( std::string, END ) );

	// Python Module Loader interface
	/* Return the module for a given module name */
	PyObject * load_module( const std::string & path );
	PY_AUTO_METHOD_DECLARE( RETOWN, load_module, ARG( std::string, END ) );

	// These are basically borrowed from Python's importdl.h, and should be in
	// order of increasing preference
	// PKG_DIRECTORY isn't preferenced, per se, we just take it if we get a
	// respath without suffix, that has an appropriate child.
	// PY_SOURCE is more preferred than PY_OBJECT since the PY_SOURCE handler
	// will load the PY_OBJECT if PY_SOURCE is older, and create a PY_OBJECT
	// appropriately.
	// C_EXTENSION should probably be higher in the list, but we can't handle
	// it, so we don't want to take it if a PY or PYC/PYO is present
	typedef enum { NOT_FOUND, C_EXTENSION, PY_OBJECT, PY_SOURCE, PKG_DIRECTORY }
		PythonModuleType;

private:
	// The actual loading methods
	// These all emulate their namesakes in Python's import.c
	PyObject * loadPackage( const std::string & fullModuleName );

	PyObject * loadCompiledModule( const std::string & fullModuleName,
		PythonModuleType modType,
		DataSectionPtr pDirectory );

	PyObject * loadSourceModule( const std::string & fullModuleName,
		PythonModuleType modType,
		DataSectionPtr pDirectory );

	// Compiled module utility methods

	static bool isCompiledModule( BinaryPtr pycData );

	static bool isCompiledModuleUpToDate( BinaryPtr pycData, time_t mTime );

	// General module importing utility methods

	void getSourceAndCompiledModulePaths( const std::string & fullModuleName,
		PythonModuleType modType,
		std::string & moduleCompiledFilename,
		std::string & moduleCompiledPath,
		std::string & moduleSourceFilename,
		std::string & moduleSourcePath ) const;

	const std::string getModuleCompiledPath(
		const std::string & fullModuleName, PythonModuleType modType ) const;

	bool writeModuleCodeObject( DataSectionPtr pDirectory,
		const std::string & compiledFilename,
		PyObject * pCodeObject, time_t mTime );

	PyObject * importModuleFromCodeObject( PyObject * pCodeObject,
		const std::string & name,
		const std::string & modulePath );

	PyObject * loadCompiledModuleFromBinary(
			const std::string & fullModuleName,
			PythonModuleType modType,
			BinaryPtr pycData, 
			bool knownValid = false );

	// Static utility methods

	static PythonModuleType findModuleType( const std::string & fullModuleName,
		const std::string & path );

	static time_t getFileModTime( const std::string & path );

	static void tidyModuleCodeString( std::string & codeString );

	static inline const std::string getShortModuleName(
		const std::string & fullModuleName );


	// Data members

	const std::string 		path_;
	DataSectionPtr 			pDirectory_;

	// The last successful call to find_module()'s results
	std::string 			lastLookupModuleName_;
	PythonModuleType 		lastLookupType_;

	// A map of known suffices, and their type
	// Whatever Script::init() does to flip Py_OptimizeFlag, should delete the
	// appropriate entry from this map.
	typedef std::map< const std::string, PythonModuleType > SuffixLookupMap;
	static SuffixLookupMap 	s_suffices_;
};


#ifdef CODE_INLINE
#include "res_mgr_import.ipp"
#endif

#endif // USE_RES_MGR_IMPORT_HOOK

#endif // RES_MGR_IMPORT_HPP
