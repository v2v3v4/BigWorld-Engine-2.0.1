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

#include "chunk/chunk_item.hpp"
#include "chunk/editor_chunk_item.hpp"
#include "array_properties_helper.hpp"
#include "resmgr/xml_section.hpp"
#include "entitydef/data_types.hpp"


/**
 *	The constructor for this class only initialises it's pointer members.
 */
ArrayPropertiesHelper::ArrayPropertiesHelper() :
	BasePropertiesHelper(),
	pSeq_( NULL )
{
}


/**
 *	The destructor for this class only decrements the refcnt of pSeq_.
 */
ArrayPropertiesHelper::~ArrayPropertiesHelper()
{
	BW_GUARD;

	Py_XDECREF( pSeq_ );
}


/**
 *	This method initialises the helper with the values it needs to be able
 *	to manage an array of properties.
 *
 *	@param pItem			Chunk item that owns the properties.
 *	@param dataType			Data type of the array itself.
 *	@param pSeq				Py object that contains the array as a Py sequence.
 *	@param changedCallback	Optional callback, called when a property changes.
 */
void ArrayPropertiesHelper::init(
	EditorChunkItem* pItem,
	DataTypePtr dataType,
	PyObject* pSeq,
	BWBaseFunctor1<int>* changedCallback /*=NULL*/ )
{
	BW_GUARD;

	pItem_ = pItem;
	dataType_ = dataType;
	pSeq_ = pSeq;
	Py_XINCREF( pSeq_ ); // init needs a new reference.
	changedCallback_ = changedCallback;
}


/**
 *	This method returns the python sequence object that contains the array's
 *	items.
 *
 *	@return		Python sequence object that contains the array's items.
 */
PyObject* ArrayPropertiesHelper::pSeq() const
{
	return pSeq_;
}


/**
 *	This method adds an item to the array.
 *
 *	@return		true if successful.
 */
bool ArrayPropertiesHelper::addItem()
{
	BW_GUARD;

	PyObject* newItem = Py_BuildValue(
		"[O]", dataType_->pDefaultValue().getObject() );

	return
		PySequence_InPlaceConcat( pSeq_, newItem ) != NULL;
}


/**
 *	This method deletes an item from the array.
 *
 *	@param index	index of the item to delete.
 *	@return			true if successful.
 */
bool ArrayPropertiesHelper::delItem( int index )
{
	BW_GUARD;

	return PySequence_DelItem( pSeq_, index ) != -1;
}


/**
 *	This method returns the name of the property at the passed index.
 *
 *	@param	index	The index of the property in the array.
 *	@return	The name of the property at the passed index.
 */
std::string ArrayPropertiesHelper::propName( PropertyIndex index )
{
	BW_GUARD;

	std::stringstream ss;
	ss << "[" << index.valueAt(0) << "]";
	return ss.str();
}


/**
 *	This method returns the number of properties in the array.
 *
 *	@return	The number of properties in the array.
 */
int ArrayPropertiesHelper::propCount() const
{
	BW_GUARD;

    return (int)PySequence_Size( pSeq_ );
}


/**
 *	This method returns the index of the named property.
 *
 *	@param	name	The name of the property.
 *	@return	The index of the named property.
 */
PropertyIndex ArrayPropertiesHelper::propGetIdx( const std::string& name ) const
{
	BW_GUARD;

	PropertyIndex result;

	uint32 openBracket = name.find_first_of( '[' );
	if ( openBracket == std::string::npos )
		return result;

	uint32 closeBracket = name.find_first_of( ']', openBracket );
	if ( closeBracket == std::string::npos || closeBracket <= openBracket )
		return result;

	std::string index = name.substr( openBracket+1, closeBracket - openBracket - 1 );
	result.append( atoi( index.c_str() ) );

	return result;
}


/**
 *	This method returns the property in PyObject form (New Reference).
 *
 *	@param	index	The index of the property in the array.
 *	@return	The property in PyObject form (New Reference).
 */
PyObject* ArrayPropertiesHelper::propGetPy( PropertyIndex index )
{
	BW_GUARD;

	MF_ASSERT( index.valueAt(0) < propCount() );

	return PySequence_GetItem( pSeq_, index.valueAt(0) );
}


/**
 *	This method sets the PyObject form of a property.
 *
 *	@param	index	The index of the property in the array.
 *	@param	pObj	The PyObject of the property.
 *	@return	Boolean success or failure.
 */
bool ArrayPropertiesHelper::propSetPy( PropertyIndex index, PyObject * pObj )
{
	BW_GUARD;

	MF_ASSERT( index.valueAt(0) < (int)propCount() );
	if ( PySequence_SetItem( pSeq_, index.valueAt(0), pObj ) == -1 )
	{
		ERROR_MSG( "ArrayPropertiesHelper::propSetPy: Failed to set value\n" );
		PyErr_Print();
		return false;
	}

	return true;
}


/**
 *	This method returns the property in datasection form.
 *
 *	@param	index	The index of the property in the array.
 *	@return	The property in datasection form.
 */
DataSectionPtr ArrayPropertiesHelper::propGet( PropertyIndex index )
{
	BW_GUARD;

	MF_ASSERT( index.valueAt(0) < (int)propCount() );
	PyObjectPtr pValue( PySequence_GetItem( pSeq_, index.valueAt(0) ), PyObjectPtr::STEAL_REFERENCE );

	if (pValue == NULL)
	{
		PyErr_Clear();
		return NULL;
	}

	DataSectionPtr pTemp = new XMLSection( "temp" );

	dataType_->addToSection( pValue.getObject(), pTemp );

	return pTemp;
}


/**
 *	This method sets the datasection form of a property.
 *
 *	@param	index	The index of the property in the array.
 *	@param	pTemp	The datasection form of property.
 *	@return	Boolean success or failure.
 */
bool ArrayPropertiesHelper::propSet( PropertyIndex index, DataSectionPtr pTemp )
{
	BW_GUARD;

	MF_ASSERT( index.valueAt(0) < (int)propCount() );
	PyObjectPtr pNewVal = dataType_->createFromSection( pTemp );
	if (!pNewVal)
	{
		PyErr_Clear();
		return false;
	}

	PySequence_SetItem( pSeq_, index.valueAt(0), pNewVal.get() );

	if ( changedCallback_ != NULL )
		(*changedCallback_)( index.valueAt(0) );

	return true;
}


/**
 *	This method sets the property to its default value.
 *
 *	@param	index	The index of the property in pType.
 */
void ArrayPropertiesHelper::propSetToDefault( int index )
{
	BW_GUARD;

	propSetPy( index, dataType_->pDefaultValue().getObject() );
}


/**
 *	Finds out if a property is a UserDataObject-to-UserDataObject link.
 *
 *	@param index	Property index.
 *	@return			true if it's a UserDataObject-to-UserDataObject link.
 */
bool ArrayPropertiesHelper::isUserDataObjectLink( int index )
{
	BW_GUARD;

	return dataType_->typeName() == "UDO_REF";
}


/**
 *	Finds if a property is an array of UserDataObject-to-UserDataObject links.
 *
 *	@param index	Property index.
 *	@return			true if it's an array of UserDataObject-to-UserDataObject links.
 */
bool ArrayPropertiesHelper::isUserDataObjectLinkArray( int index )
{
	// TODO: support arrays of arrays
	return false;
}
