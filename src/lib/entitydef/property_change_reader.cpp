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

#include "property_change_reader.hpp"

#include "property_change.hpp" // Just for MAX_SIMPLE_PROPERTY_CHANGE_ID

#include "bit_reader.hpp"
#include "data_type.hpp"
#include "property_owner.hpp"

#include "cstdmf/binary_stream.hpp"
#include "pyscript/script.hpp"

// In property_change.cpp
int bitsRequired( int numValues );


// -----------------------------------------------------------------------------
// Section: PropertyChangeReader
// -----------------------------------------------------------------------------

/**
 *	This method reads and applies a property change.
 *
 *	@param stream  The stream to read from.
 *	@param pOwner  The top-level owner of the property.
 *	@param clientServerID The message id which could be the clientServerID.
 *	@param ppOldValue If not NULL, this is set to the old value of the property.
 *
 *	@return The top-level index of the change.
 */
int PropertyChangeReader::readAndApply( BinaryIStream & stream,
		PropertyOwnerBase * pOwner, int clientServerID,
		PyObjectPtr * ppOldValue,
		PyObjectPtr * ppChangePath )
{
	int topLevelIndex = -1;

	if (clientServerID == -1)
	{
		uint8 size;
		stream >> size;

		for (uint8 i = 0; i < size; ++i)
		{
			int32 index;
			stream >> index;

			if (pOwner)
			{
				pOwner = pOwner->getChildPropertyOwner( index );
			}

			if (!pOwner)
			{
				ERROR_MSG( "PropertyChangeReader::readAndApply: "
						"pOwner is NULL. %d/%d. "
						"curr index = %d. topLevelIndex = %d\n",
					i, size, index, topLevelIndex );
			}

			if (topLevelIndex == -1)
			{
				topLevelIndex = index;
			}
		}

		int index = this->readExtraBits( stream );

		if (topLevelIndex == -1)
		{
			topLevelIndex = index;
		}
	}
	else if (clientServerID <= MAX_SIMPLE_PROPERTY_CHANGE_ID)
	{
		this->setIndex( clientServerID );
		topLevelIndex = clientServerID;
	}
	else
	{
		BitReader bits( stream );

		while ((bits.get( 1 ) != 0) && pOwner)
		{
			int numProperties = pOwner->getNumOwnedProperties();
			int index = bits.get( bitsRequired( numProperties ) );

			if (topLevelIndex == -1)
			{
				topLevelIndex = index;
			}
			else if (ppChangePath)
			{
				this->updatePath( ppChangePath, pOwner->getPyIndex( index ) );
			}

			pOwner = pOwner->getChildPropertyOwner( index );
		}

		if (pOwner)
		{
			int index =
				this->readExtraBits( bits, pOwner->getNumOwnedProperties() );

			if (topLevelIndex == -1)
			{
				topLevelIndex = index;
			}
			else
			{
				this->updatePath( ppChangePath );
			}
		}
	}

	if (pOwner)
	{
		PyObjectPtr pOldValue = this->apply( stream, pOwner,
				ppChangePath ? *ppChangePath : NULL );

		if (!pOldValue)
		{
			ERROR_MSG( "PropertyChangeReader::readAndApply: "
					"Old value is NULL\n" );
		}
		else if (ppOldValue)
		{
			*ppOldValue = pOldValue;
		}
	}
	else
	{
		ERROR_MSG( "PropertyChangeReader::readAndApply: Invalid path to owner. "
					"clientServerID = %d. topLevelIndex = %d\n",
				clientServerID, topLevelIndex );

		if (topLevelIndex >= 0)
		{
			return -1 - topLevelIndex;
		}
	}

	return topLevelIndex;
}


/**
 *	This method appends the input index to the change path.
 */
void PropertyChangeReader::updatePath( PyObjectPtr * ppChangePath,
		PyObjectPtr pIndex ) const
{
	if (ppChangePath)
	{
		if ((*ppChangePath) == NULL)
		{
			*ppChangePath = PyObjectPtr( PyList_New( 0 ),
					PyObjectPtr::STEAL_REFERENCE );
		}

		if (pIndex)
		{
			if (PyList_Append( (*ppChangePath).get(), pIndex.get() ) == -1)
			{
				ERROR_MSG( "PropertyChangeReader::updatePath: "
						"PyList_Append failed\n" );
				PyErr_Print();
			}
		}
	}
}


// -----------------------------------------------------------------------------
// Section: SinglePropertyChangeReader
// -----------------------------------------------------------------------------

/**
 *	This method applies this property change.
 *
 *	@param The stream containing the new value.
 *	@param The low-level owner of the property change.
 *
 *	@return The old value.
 */
PyObjectPtr SinglePropertyChangeReader::apply( BinaryIStream & stream,
		PropertyOwnerBase * pOwner, PyObjectPtr pChangePath )
{
	if (pChangePath)
	{
		PyList_Append( pChangePath.get(),
				pOwner->getPyIndex( leafIndex_ ).get() );
	}

	return pOwner->setOwnedProperty( leafIndex_, stream );
}


/**
 *	This method reads the extra data specific to this PropertyChange type. This
 *	version is used for server-internal changes.
 */
int SinglePropertyChangeReader::readExtraBits( BinaryIStream & stream )
{
	stream >> leafIndex_;

	return leafIndex_;
}


/**
 *	This method reads the extra data specific to this PropertyChange type. This
 *	version is used for client-server changes.
 */
int SinglePropertyChangeReader::readExtraBits( BitReader & reader,
		int leafSize )
{
	const int numBitsRequired = bitsRequired( leafSize );
	leafIndex_ = reader.get( numBitsRequired );

	return leafIndex_;
}


// -----------------------------------------------------------------------------
// Section: SlicePropertyChangeReader
// -----------------------------------------------------------------------------

/**
 *	This method applies this property change.
 *
 *	@param The stream containing the new value.
 *	@param The low-level owner of the property change.
 *
 *	@return The old value.
 */
PyObjectPtr SlicePropertyChangeReader::apply( BinaryIStream & stream,
		PropertyOwnerBase * pOwner, PyObjectPtr pChangePath )
{
	int oldSize = pOwner->getNumOwnedProperties();

	PyObjectPtr pOldValues =
		pOwner->setOwnedSlice( startIndex_, endIndex_, stream );

	int newSize = pOwner->getNumOwnedProperties();

	if (pChangePath && pOldValues)
	{
		PyObjectPtr pIndex( Py_BuildValue( "(ii)", startIndex_,
					endIndex_ + newSize - oldSize ),
			PyObjectPtr::STEAL_REFERENCE );

		PyList_Append( pChangePath.get(), pIndex.get() );
	}

	return pOldValues;
}


/**
 *	This method reads the extra data specific to this PropertyChange type. This
 *	version is used for server-internal changes.
 */
int SlicePropertyChangeReader::readExtraBits( BinaryIStream & stream )
{
	stream >> startIndex_ >> endIndex_;

	return -1;
}


/**
 *	This method reads the extra data specific to this PropertyChange type. This
 *	version is used for client-server changes.
 */
int SlicePropertyChangeReader::readExtraBits( BitReader & reader,
		int leafSize )
{
	const int numBitsRequired = bitsRequired( leafSize + 1 );
	startIndex_ = reader.get( numBitsRequired );
	endIndex_ = reader.get( numBitsRequired );

	return -1;
}

// property_change_reader.cpp
