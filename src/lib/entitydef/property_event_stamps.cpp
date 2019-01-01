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

#include "property_event_stamps.hpp"

#ifndef CODE_INLINE
#include "property_event_stamps.ipp"
#endif

// -----------------------------------------------------------------------------
// Section: PropertyEventStamps
// -----------------------------------------------------------------------------

/**
 *	This method adds this object to the input stream.
 */
void PropertyEventStamps::addToStream(
		BinaryOStream & stream ) const
{
	Stamps::const_iterator iter = eventStamps_.begin();

	while (iter != eventStamps_.end())
	{
		stream << (*iter);

		iter++;
	}
}


/**
 *	This method removes this object to the input stream.
 */
void PropertyEventStamps::removeFromStream(
		BinaryIStream & stream )
{
	Stamps::iterator iter = eventStamps_.begin();

	while (iter != eventStamps_.end())
	{
		stream >> (*iter);

		iter++;
	}
}

// property_event_stamps.cpp
