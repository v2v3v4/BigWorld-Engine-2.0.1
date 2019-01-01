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

#include "property_change.hpp"

#include "bit_writer.hpp"
#include "data_type.hpp"
#include "property_owner.hpp"

#include "cstdmf/binary_stream.hpp"
#include "pyscript/script.hpp"


// -----------------------------------------------------------------------------
// Section: Helper functions
// -----------------------------------------------------------------------------

/**
 *	This function returns the number of bits required to identify the input
 *	number of values.
 *
 *	It is ceil( log( numValues, 2 ) ). That is, the log base 2 of the input
 *	number rounded up.
 */
int bitsRequired( int numValues )
{
	if (numValues <= 1)
	{
		return 0;
	}

	// get the expected bit width of the current index
	numValues--;

	register int nbits;
#ifdef _WIN32
	_asm bsr eax, numValues	// Bit Scan Reverse(numValues)->eax
	_asm mov nbits, eax		// eax-> nbits
#else
	__asm__ (
		"bsr 	%1, %%eax" 	// Bit Scan Reverse(numValues)->eax
		:"=a"	(nbits) 	// output eax: nbits
		:"r"	(numValues)	// input %1: numValues
	);
#endif
	return nbits+1;
}


/*
 *	This method implements simple streaming of ChangePath. It is not compressed.
 */
void PropertyChange::writePathSimple( BinaryOStream & stream ) const
{
	stream << uint8( path_.size() );

	for (int i = path_.size()-1; i >= 0; --i)
	{
		stream << path_[i];
	}
}


// -----------------------------------------------------------------------------
// Section: PropertyChange
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param type	The type of the value changing.
 */
PropertyChange::PropertyChange( const DataType & type ) :
	type_( type ),
	path_()
{
}


/**
 *	This method adds the path to the owner of the changing property to a stream.
 */
uint8 PropertyChange::addPathToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int clientServerID ) const
{
	uint8 ret = 0xff;

	if (clientServerID == -1)
	{
		this->writePathSimple( stream );
		this->addExtraBits( stream );
	}
	else if ((clientServerID <= MAX_SIMPLE_PROPERTY_CHANGE_ID) && path_.empty())
	{
		// we needn't add anything if this is a top-level property update
		// of a low-numbered property
		ret = uint8(clientServerID);
	}
	// otherwise we need to do more encoding
	else
	{
		// message id PROPERTY_CHANGE_ID_SINGLE as an escape indicating more
		// encoding
		ret = PROPERTY_CHANGE_ID_SINGLE;

		BitWriter bits;

		// Put on the index of each property in reverse order, with an extra bit
		// between each to say 'keep going'.
		const PropertyOwnerBase * pCurrOwner = pOwner;

		for (int i = path_.size()-1; i >= 0; --i)
		{
			int curIdx = path_[i];

			// For top-level, use client-server index, not internal index.
			const bool isTopLevel = (i == (int)path_.size()-1);
			int streamedIndex = isTopLevel ? clientServerID : curIdx;

			MF_ASSERT( pCurrOwner != NULL );
			int numProperties = pCurrOwner->getNumOwnedProperties();

			MF_ASSERT( numProperties >= 0 );

			bits.add( 1, 1 ); // add a 1 bit to say: "not done yet"
			bits.add( bitsRequired( numProperties ), streamedIndex );

			pCurrOwner = pCurrOwner->getChildPropertyOwner( curIdx );
		}

		bits.add( 1, 0 ); // add a 0 bit to say: "stop"

		// if we can still have an owner here, need to put on
		// something to say this is the end of the list
		if (pCurrOwner != NULL)
		{
			// TODO: It would be better if the client/server id was the same as
			// the internal id so that this does not need to be special-cased.

			if (path_.empty())
			{
				bits.add( bitsRequired( pCurrOwner->getNumOwnedProperties() ),
							clientServerID );
			}
			else
			{
				this->addExtraBits( bits, pCurrOwner->getNumOwnedProperties() );
			}
		}
		else
		{
			ERROR_MSG( "PropertyChange::addToStream: "
					"Invalid path. pCurrOwner is NULL\n" );
		}

		// and put it on the stream (to the nearest byte)
		int used = bits.usedBytes();

		memcpy( stream.reserve( used ), bits.bytes(), used );
	}

	return ret;
}


// -----------------------------------------------------------------------------
// Section: SinglePropertyChange
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param leafIndex	The index of the changing property.
 *	@param type			The type of the property changing.
 */
SinglePropertyChange::SinglePropertyChange( int leafIndex,
			const DataType & type ) :
		PropertyChange( type ),
		leafIndex_( leafIndex ),
		pValue_( NULL )
{
}


/**
 *	This method adds this property change to a stream.
 *
 *	@param stream	The stream to add to.
 *	@param pOwner	The top-level owner of the property.
 *	@param clientServerID	The top-level id used for client-server.
 *
 *	@return The message id that should be used to send this change.
 */
uint8 SinglePropertyChange::addToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int clientServerID ) const
{
	uint8 msgID = this->addPathToStream( stream, pOwner, clientServerID );

	type_.addToStream( pValue_.get(), stream, false );

	return msgID;
}


/**
 *	This method adds extra data specific to this property change. This version
 *	is called when streaming internally to the server.
 */
void SinglePropertyChange::addExtraBits( BinaryOStream & stream ) const
{
	stream << leafIndex_;
}


/**
 *	This method adds extra data specific to this property change. This version
 *	is called when streaming to the client.
 */
void SinglePropertyChange::addExtraBits( BitWriter & writer, int leafSize ) const
{
	writer.add( bitsRequired( leafSize ), leafIndex_ );
}


// -----------------------------------------------------------------------------
// Section: SlicePropertyChange
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param startIndex The index of the first value to change.
 *	@param endIndex   One greater than the index of the last value to change.
 *	@param newValues  The new values to add.
 *	@param type       The type of the new values.
 */
SlicePropertyChange::SlicePropertyChange(
		Py_ssize_t startIndex, Py_ssize_t endIndex,
		const std::vector< PyObjectPtr > & newValues,
		const DataType & type ) :
	PropertyChange( type ),
	startIndex_( startIndex ),
	endIndex_( endIndex ),
	newValues_( newValues )
{
}


/**
 *	This method adds this slice change to the input stream.
 */
uint8 SlicePropertyChange::addToStream( BinaryOStream & stream,
			const PropertyOwnerBase * pOwner, int clientServerID ) const
{
	this->addPathToStream( stream, pOwner, clientServerID );

	// Read until stream is empty. In PyArrayDataInstance::setOwnedSlice.

	std::vector< PyObjectPtr >::const_iterator iter = newValues_.begin();

	while (iter != newValues_.end())
	{
		type_.addToStream( iter->get(), stream, false );

		++iter;
	}

	return PROPERTY_CHANGE_ID_SLICE;
}


/**
 *	This method adds extra data specific to this property change. This version
 *	is called when streaming internally to the server.
 */
void SlicePropertyChange::addExtraBits( BinaryOStream & stream ) const
{
	stream << startIndex_ << endIndex_;
}


/**
 *	This method adds extra data specific to this property change. This version
 *	is called when streaming to the client.
 */
void SlicePropertyChange::addExtraBits( BitWriter & writer, int leafSize ) const
{
	int origSize = leafSize + (endIndex_ - startIndex_) - newValues_.size();

	// The value of slice indexes is [0, size], not [0, size - 1].
	const int numBitsRequired = bitsRequired( origSize + 1 );

	writer.add( numBitsRequired, startIndex_ );
	writer.add( numBitsRequired, endIndex_ );
}

// property_change.cpp
