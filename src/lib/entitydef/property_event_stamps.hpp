/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROPERTY_EVENT_STAMPS_HPP
#define PROPERTY_EVENT_STAMPS_HPP

#include "network/basictypes.hpp"

#include <vector>

class DataDescription;
class EntityDescription;

/**
 *	This class is used to store the event number when a property last
 *	changed for each property in an entity that is 'otherClient'.
 */
class PropertyEventStamps
{
public:
	void init( const EntityDescription & entityDescription );
	void init( const EntityDescription & entityDescription,
		   EventNumber lastEventNumber );

	void set( const DataDescription & dataDescription,
			EventNumber eventNumber );

	EventNumber get( const DataDescription & dataDescription ) const;

	void addToStream( BinaryOStream & stream ) const;
	void removeFromStream( BinaryIStream & stream );

private:
	typedef std::vector< EventNumber > Stamps;
	Stamps eventStamps_;
};


#include "entity_description.hpp"

#ifdef CODE_INLINE
#include "property_event_stamps.ipp"
#endif

#endif // PROPERTY_EVENT_STAMPS_HPP
