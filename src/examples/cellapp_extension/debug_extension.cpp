/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cellapp/entity.hpp"
#include "cellapp/real_entity.hpp"

#include "server/stream_helper.hpp"

// This file contains some example Python methods that may be useful during
// debugging. These will be placed in a BWDebug module.

/*
 *	This class is used by to generate a Python list of the data associated with
 *	an entity as it woud appear on a network stream.
 */
class StreamSizeVisitor : public IDataDescriptionVisitor
{
public:
	StreamSizeVisitor( PyObject * pEntity ) :
		pEntity_( pEntity ),
		pList_( PyList_New( 0 ) )
	{
	}

	~StreamSizeVisitor()
	{
		Py_DECREF( pList_ );
	}

	/*
	 *	This method is called for each of the DataDescription objects associated
	 *	with the entity in a specified subset. For example, all data flagged as
	 *	CELL_DATA.
	 */
	virtual bool visit( const DataDescription & propDesc )
	{
		PyObject * pProp = PyObject_GetAttrString( pEntity_.get(),
				const_cast< char * >( propDesc.name().c_str() ) );

		if (pProp == NULL)
		{
			PyErr_Print();
			return false;
		}

		MemoryOStream stream;
		uint64 startTime = ::timestamp();
		propDesc.addToStream( pProp, stream, /*isPersistentOnly*/ false );
		double timeTaken = (::timestamp() - startTime)/::stampsPerSecondD();

		Py_DECREF( pProp );

		PyObject * pTuple = PyTuple_New( 4 );
		PyTuple_SET_ITEM( pTuple, 0,
			PyString_FromString( propDesc.name().c_str() ) );

		PyTuple_SET_ITEM( pTuple, 1,
			PyString_FromStringAndSize(
				static_cast< char * >( stream.data() ), stream.size() ) );

		PyTuple_SET_ITEM( pTuple, 2,
			PyFloat_FromDouble( timeTaken ) );

		PyTuple_SET_ITEM( pTuple, 3,
			PyString_FromString( propDesc.getDataFlagsAsStr() ) );

		PyList_Append( pList_, pTuple );
		Py_DECREF( pTuple );

		return true;
	}

	PyObject * pList() const	{ return pList_; }

private:
	PyObjectPtr pEntity_;
	PyObject * pList_;
};


/*
 *	This method returns some debug information about the entity's streamed data
 *	sizes.
 *
 *	It returns a Python list. Each entry corresponds to a CELL_DATA property is
 *	a tuple of size four containing the property's name, a blob as would appear
 *	on the network, the time taken to generate the blob and the flags associated
 *	with the property.
 */
PyObject * calcStreamedProperties( PyObjectPtr pEnt )
{
	if (!Entity::Check( pEnt.get() ))
	{
		PyErr_SetString( PyExc_TypeError,
				"Expected an entity as the argument" );
		return NULL;
	}

	Entity * pEntity = static_cast< Entity * >( pEnt.get() );
	if (!pEntity->isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
				"Can only be called on a real entity" );
		return false;
	}

	EntityTypePtr pType = pEntity->pType();
	const EntityDescription & desc = pType->description();

	StreamSizeVisitor visitor( static_cast< PyObject * >( pEntity ) );
	// Currently hard-coded to be only CELL_DATA. This could be made to be
	// configurable.
	desc.visit( EntityDescription::CELL_DATA, visitor );

	PyObject * pReturn = visitor.pList();
	Py_INCREF( pReturn );

	return pReturn;
}
PY_AUTO_MODULE_FUNCTION( RETOWN,
		calcStreamedProperties, ARG( PyObjectPtr, END ), BWDebug )


/*
 *	This method returns a Python blob (string) containing the data that would
 *	be added to the network when a real entity is being offloaded.
 */
PyObject * calcOffloadData( PyObjectPtr pEnt )
{
	if (!Entity::Check( pEnt.get() ))
	{
		PyErr_SetString( PyExc_TypeError,
				"Expected an entity as the argument" );
		return NULL;
	}

	Entity * pEntity = static_cast< Entity * >( pEnt.get() );

	if (!pEntity->isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"BWDebug.calcOffloadData: can only be called on a real entity" );
		return false;
	}

	MemoryOStream stream;
	pEntity->writeRealDataToStream( stream, Mercury::Address( 0, 0 ), true );

	return PyString_FromStringAndSize( static_cast< char * >( stream.data() ),
			stream.size() );
}
PY_AUTO_MODULE_FUNCTION( RETOWN,
		calcOffloadData, ARG( PyObjectPtr, END ), BWDebug )


/*
 *	This method returns a Python blob (string) containing the data that would
 *	be added to the network when a real entity is being backed up.
 */
PyObject * calcCellBackupData( PyObjectPtr pEnt )
{
	if (!Entity::Check( pEnt.get() ))
	{
		PyErr_SetString( PyExc_TypeError,
				"Expected an entity as the argument" );
		return NULL;
	}

	Entity * pEntity = static_cast< Entity * >( pEnt.get() );

	if (!pEntity->isRealToScript())
	{
		PyErr_SetString( PyExc_TypeError,
			"BWDebug.calcCellBackupData: can only be called on a real entity" );
		return false;
	}

	MemoryOStream stream;
	pEntity->pReal()->writeBackupProperties( stream );

	return PyString_FromStringAndSize( static_cast< char * >( stream.data() ),
			stream.size() );
}
PY_AUTO_MODULE_FUNCTION( RETOWN,
		calcCellBackupData, ARG( PyObjectPtr, END ), BWDebug )


// debug_extension.cpp
