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

#include "property_owner.hpp"
#include "property_change.hpp"
#include "property_change_reader.hpp"

#include "data_type.hpp"

#include "cstdmf/binary_stream.hpp"
#include "pyscript/script.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )


// -----------------------------------------------------------------------------
// Section: PropertyOwnerBase
// -----------------------------------------------------------------------------

/**
 *	This method changes a property owned by this one. It propagates this change
 *	to the top-level owner.
 */
bool PropertyOwnerBase::changeOwnedProperty( PyObjectPtr & rpOldValue,
		PyObject * pNewValue, DataType & dataType, int index )
{
	if (dataType.canIgnoreAssignment( rpOldValue.get(), pNewValue ))
	{
		return true;
	}

	bool changed = dataType.hasChanged( rpOldValue.get(), pNewValue );

	SinglePropertyChange change( index, dataType );
	PropertyOwnerBase * pTopLevelOwner = NULL;

	if (changed)
	{
		if (!this->getTopLevelOwner( change, pTopLevelOwner ))
		{
			return false;
		}
	}

	PyObjectPtr pRealNewValue = dataType.attach( pNewValue, this, index );

	if (!pRealNewValue)
	{
		return false;
	}

	dataType.detach( rpOldValue.get() );
	rpOldValue = pRealNewValue;

	if (pTopLevelOwner != NULL)
	{
		change.setValue( pRealNewValue );
		pTopLevelOwner->onOwnedPropertyChanged( change );
	}

	return true;
}



// -----------------------------------------------------------------------------
// Section: TopLevelPropertyOwner
// -----------------------------------------------------------------------------

/**
 *	This method sets an owned property from a stream that has been sent within
 *	the server.
 */
int TopLevelPropertyOwner::setPropertyFromInternalStream(
		BinaryIStream & stream,
		PropertyChangeType type )
{
	return (type == PROPERTY_CHANGE_TYPE_SINGLE) ?
		this->setPropertyFromStream( stream ) :
		this->setSliceFromStream( stream );
}


/**
 *	This method sets an owned property from a stream that has been sent from the
 *	server to a client.
 */
int TopLevelPropertyOwner::setPropertyFromExternalStream( BinaryIStream & stream,
		int clientServerIndex,
		PyObjectPtr * ppOldValue,
		PyObjectPtr * ppChangePath )
{
	return (clientServerIndex == PROPERTY_CHANGE_ID_SLICE) ?
		this->setSliceFromStream( stream, clientServerIndex,
				ppOldValue, ppChangePath ) :
		this->setPropertyFromStream( stream, clientServerIndex,
				ppOldValue, ppChangePath );
}


/**
 *	This method sets a single, owned property from a stream.
 *
 *	@param stream	The stream to read the property from.
 *	@param clientServerID The message id or -1 if internal stream.
 *	@param ppOldValue If not NULL, this is set to the old value.
 *
 *	@return The index of the top-level property.
 */
int TopLevelPropertyOwner::setPropertyFromStream( BinaryIStream & stream,
		int clientServerID,
		PyObjectPtr * ppOldValue,
		PyObjectPtr * ppChangePath )
{
	SinglePropertyChangeReader reader;

	return reader.readAndApply( stream, this, clientServerID,
			ppOldValue, ppChangePath );
}


/**
 *	This method replaces a slice of an owned array property from a stream.
 *
 *	@param stream	The stream to read the new properties from.
 *	@param clientServerID The message id or -1 if internal stream.
 *
 *	@return The index of the top-level property.
 */
int TopLevelPropertyOwner::setSliceFromStream( BinaryIStream & stream,
		int clientServerID,
		PyObjectPtr * ppOldValue,
		PyObjectPtr * ppChangePath )
{
	SlicePropertyChangeReader reader;

	return reader.readAndApply( stream, this, clientServerID,
			ppOldValue, ppChangePath );
}

// property_owner.cpp
