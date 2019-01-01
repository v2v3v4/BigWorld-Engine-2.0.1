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
#include "py_gui_item.hpp"
#include "pyscript/py_data_section.hpp"

DECLARE_DEBUG_COMPONENT2( "Script", 0 )

PyObject * PyItem::pyNew( PyObject * args )
{
	BW_GUARD;

	PyDataSection* section;

	if (!PyArg_ParseTuple( args, "O", &section ))
	{
		PyErr_SetString( PyExc_TypeError, "PyItem() "
			"expected a data section argument" );
		return NULL;
	}

	PyObject* result = new PyItem( new GUI::Item( section->pSection() ) );
	Py_DECREF( section );
	return result;
}

PyObject * PyItem::s_subscript( PyObject * self, PyObject * index )
{
	BW_GUARD;

	return ((PyItem *) self)->subscript( index );
}

int PyItem::s_length( PyObject * self )
{
	BW_GUARD;

	return (int)((PyItem *) self)->pItem_->num();
}

// -----------------------------------------------------------------------------
// Section: Script definition
// -----------------------------------------------------------------------------

/**
 *	This structure contains the function pointers necessary to provide a Python
 *	Mapping interface.
 */
static PyMappingMethods g_itemMapping =
{
	PyItem::s_length,	// mp_length
	PyItem::s_subscript,	// mp_subscript
	NULL						// mp_ass_subscript
};

PY_TYPEOBJECT_WITH_MAPPING( PyItem, &g_itemMapping )

PY_BEGIN_METHODS( PyItem )
	PY_METHOD( has_key )
	PY_METHOD( keys )
	PY_METHOD( items )
	PY_METHOD( values )

	PY_METHOD( createItem )
	PY_METHOD( deleteItem )

	PY_METHOD( copy )

	PY_METHOD( child )
	PY_METHOD( childName )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyItem )
	/*~ attribute DataSection.name
	 *	@components{ all }
	 *
	 *	This attribute is the name of the data section.  It is read-only,
	 *	and consists of the tag surrounding this particular section
	 *
	 *	@type	Read-Only String
	 */
	PY_ATTRIBUTE( name )
	PY_ATTRIBUTE( displayName )
PY_END_ATTRIBUTES()

PY_FACTORY_NAMED( PyItem, "Item", GUI )

PY_SCRIPT_CONVERTERS( PyItem )

PyItem::PyItem( GUI::ItemPtr pItem, PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	pItem_( pItem )
{
	BW_GUARD;

	MF_ASSERT( pItem_ );
}

PyItem::~PyItem()
{
}

GUI::ItemPtr	PyItem::pItem() const
{
	return pItem_;
}

PyObject* PyItem::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	// try it as a subitem
	if (attr[0] == '_' && (attr[1] != '_' && attr[1] != 0))
	{
		GUI::ItemPtr pChildItem = (*pItem_)( attr+1 );
		if (pChildItem)
		{
			return new PyItem( pChildItem );
		}
	}

	return PyObjectPlus::pyGetAttribute( attr );
}

int PyItem::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

PyObject * PyItem::subscript( PyObject* pathArg )
{
	BW_GUARD;

	char * path = PyString_AsString( pathArg );

	if(PyErr_Occurred())
	{
		return NULL;
	}

	GUI::ItemPtr pChildItem = (*pItem_)( path );

	if (!pChildItem)
	{
		Py_Return;
	}

	return new PyItem( pChildItem );
}


/**
 *	This method returns the number of child data sections.
 */
int PyItem::length()
{
	BW_GUARD;

	return pItem_->num();
}

PyObject* PyItem::py_has_key( PyObject* args )
{
	BW_GUARD;

	char * itemName;

	if (!PyArg_ParseTuple( args, "s", &itemName ))
	{
		PyErr_SetString( PyExc_TypeError, "Expected a string argument." );
		return NULL;
	}

	if ((*pItem_)( itemName ))
	{
		return PyInt_FromLong(1);
	}
	else
	{
		return PyInt_FromLong(0);
	}
}

PyObject* PyItem::py_keys( PyObject* /*args*/ )
{
	BW_GUARD;

	int size = (int)pItem_->num();

	PyObject * pList = PyList_New( size );

	for (int i = 0; i < size; i++)
	{
		PyList_SetItem( pList, i,
			PyString_FromString( (*pItem_)[ i ]->name().c_str() ) );
	}

	return pList;
}

PyObject* PyItem::py_values(PyObject* /*args*/)
{
	BW_GUARD;

	int size = (int)pItem_->num();
	PyObject* pList = PyList_New( size );

	for( int i = 0; i < size; ++i )
		PyList_SetItem( pList, i, new PyItem( (*pItem_)[ i ] ) );

	return pList;
}

PyObject* PyItem::py_items( PyObject* /*args*/ )
{
	BW_GUARD;

	int size = (int)pItem_->num();
	PyObject* pList = PyList_New( size );

	for( int i = 0; i < size; ++i )
	{
		GUI::ItemPtr pChild = (*pItem_)[ i ];

		PyObject * pTuple = PyTuple_New( 2 );

		PyTuple_SetItem( pTuple, 0,
			PyString_FromString( pChild->name().c_str() ) );
		PyTuple_SetItem( pTuple, 1, new PyItem( pChild ) );

		PyList_SetItem( pList, i, pTuple );
	}

	return pList;
}

/*
 *	This is a simple helper function used by the read functions.
 */
inline char * getStringArg( PyObject * args )
{
	BW_GUARD;

	if (PyTuple_Size( args ) == 1 &&
		PyString_Check( PyTuple_GetItem( args, 0 ) ))
	{
		return PyString_AsString( PyTuple_GetItem( args, 0 ) );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Expected one string argument." );
		return NULL;
	}
}


/**
 *	This method creates the section at the input path.
 */
PyObject * PyItem::py_createItem( PyObject * args )
{
	BW_GUARD;

	throw 1; //TODO:)
	Py_Return;
/*	char * path = getStringArg( args );

	if (path == NULL)
	{
		return NULL;
	}

	std::string sectionBase = BWResource::getFilePath( path );
	// Strip the trailing / off the directory name
	// (this gets added even if the path has no dir blank)
	sectionBase = sectionBase.substr( 0, sectionBase.length() - 1 );
	std::string itemName = BWResource::getFilename( path );

	// We do this, so that names such as foldername/filename.xml can be passed in
	GUI::ItemPtr pBaseSection = pItem_;
	if ( !sectionBase.empty() )
	{
		pBaseSection = pItem_->openSection( sectionBase, true );
		if (!pBaseSection)
			Py_Return;
	}

	GUI::ItemPtr pSection = pBaseSection->newSection( itemName );
	if (pSection)
	{
		return new PyItem( pSection );
	}
	else
	{
		Py_Return;
	}*/
}

PyObject * PyItem::py_deleteItem( PyObject * args )
{
	BW_GUARD;

	char * path = getStringArg( args );

	if( path == NULL )
	{
		return NULL;
	}

	throw 1;//TODO:)
/*	GUI::ItemPtr item = (*pItem_)( path );
	if( item && item->)
	{
		item
	}*/
	bool result = true;

	return Script::getData( result );
}

/*~ function Item.copy
 *	@components{ all }
 *
 *	This function makes this Item into a copy of the specified
 *	Item.
 *
 *	@param	source the Item to copy this one from.
 */
PyObject * PyItem::py_copy( PyObject * args )
{
	BW_GUARD;

	PyErr_SetString( PyExc_NotImplementedError, "PyItem.copy: not supported" );
	return NULL;
#if 0
	PyObject * pPyItem;
	if (!PyArg_ParseTuple( args, "O:PyItem.copy", &pPyItem))
	{
		return NULL;
	}
	if (!PyItem::Check(pPyItem))
	{
		PyErr_SetString( PyExc_TypeError, "PyItem.copy: expected a PyItem.");
		return NULL;
	}

//	pItem_->copy( static_cast<PyItem*>( pPyItem )->pItem_ );

	Py_Return;
#endif
}

PyObject * PyItem::child( int index )
{
	BW_GUARD;

	if( 0 <= index && index < (int)pItem_->num() )
	{
		GUI::ItemPtr pChild = (*pItem_)[ index ];
		if (pChild) return new PyItem( pChild );

		const std::string childName = (*pItem_)[ index ]->name();
		PyErr_Format( PyExc_EnvironmentError,
			"child index %d '%s' has disappeared",
			index, childName.c_str() );
		return NULL;
	}
	else
	{
		PyErr_SetString( PyExc_IndexError, "child index out of range" );
		return NULL;
	}
}

PyObject* PyItem::childName( int index )
{
	BW_GUARD;

	if( 0 <= index && index < (int)pItem_->num() )
	{
		return Script::getData( (*pItem_)[ index ]->name() );
	}
	else
	{
		PyErr_SetString( PyExc_IndexError, "child index out of range" );
		return NULL;
	}
}
