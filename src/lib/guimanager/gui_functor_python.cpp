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
#include "gui_functor_python.hpp"
#include "py_gui_item.hpp"
#include "pyscript/script.hpp"

BEGIN_GUI_NAMESPACE

PyObject* PythonFunctor::call( std::string function, ItemPtr item )
{
	BW_GUARD;

	std::string module = defaultModule_;
	if( function.find( '.' ) != function.npos )
	{
		module = function.substr( 0, function.find( '.' ) );
		function = function.substr( function.find( '.' ) + 1 );
	}
	if( modules_.find( module ) == modules_.end() )
	{
		PyObject* obj = PyImport_ImportModule( (char*)module.c_str() );
		if( !obj )
		{
			PyErr_Clear();
			return NULL;
		}
		modules_[ module ] = obj;
	}


	PyObject* result = NULL;
	PyObject* func = PyObject_GetAttrString( modules_[ module ], (char*)( function.c_str() ) );
	if ( func != NULL )
	{
		PyItem* pyitem = new PyItem( item );
		PyObject* obj = Py_BuildValue( "(O)", pyitem );
		Py_XDECREF( pyitem );
		if( obj )
		{
			result = Script::ask( func,  obj, ( module + "." + function + ':' ).c_str() );
			if( !result )
				PyErr_Clear();
		}
		else
			PyErr_Clear();
	}
	else
		PyErr_Clear();
	return result;
}
const std::string& PythonFunctor::name() const
{
	static std::string name = "Python";
	return name;
}

bool PythonFunctor::text( const std::string& textor, ItemPtr item, std::string& result )
{
	BW_GUARD;

	result.clear();
	PyObjectPtr obj( call( textor, item ), PyObjectPtr::STEAL_REFERENCE);
	if( obj.exists() )
	{
		char* str;
		if( PyString_Check( obj.get() ) && ( str = PyString_AsString( obj.get() ) ) )
		{
			result = str;
			return true;
		}
	}
	return false;
}

bool PythonFunctor::update( const std::string& updater, ItemPtr item, unsigned int& result )
{
	BW_GUARD;

	PyObjectPtr obj( call( updater, item ), PyObjectPtr::STEAL_REFERENCE) ;
	if( obj.exists() )
	{
		if (PyInt_Check( obj ) )
		{
			result = PyInt_AsLong( obj.get() );
			return true;
		}
	}
	return false;
}

DataSectionPtr PythonFunctor::import( const std::string& importer, ItemPtr item )
{
	return NULL;
}

bool PythonFunctor::act( const std::string& action, ItemPtr item, bool& result )
{
	BW_GUARD;

	PyObjectPtr obj( call( action, item ), PyObjectPtr::STEAL_REFERENCE) ;
	if( obj.exists() )
	{
		if (PyInt_Check( obj ) )
		{
			result = ( PyInt_AsLong( obj.get() ) != 0 );
			return true;
		}
	}
	return false;
}

void PythonFunctor::defaultModule( const std::string& module )
{
	defaultModule_ = module;
}

END_GUI_NAMESPACE
