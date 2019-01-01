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

#include "entity_method_descriptions.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )


// -----------------------------------------------------------------------------
// Section: EntityMethodDescriptions
// -----------------------------------------------------------------------------

/**
 *	This method initialises this collection of methods from a data section.
 *	NOTE: Despite its name, this method may be called more than once to add
 *	the methods from implemented interfaces.
 */
bool EntityMethodDescriptions::init( DataSectionPtr pMethods,
		MethodDescription::Component component, const char * interfaceName )
{
	if (!pMethods)
	{
		WARNING_MSG( "EntityMethodDescriptions::init: pMethods is NULL\n" );

		return false;
	}

	DataSectionIterator iter = pMethods->begin();

	while (iter != pMethods->end())
	{
		MethodDescription methodDescription;

		if (!methodDescription.parse( *iter, component ))
		{
			WARNING_MSG( "Error parsing method %s\n",
				methodDescription.name().c_str() );
			return false;
		}
		if (component == MethodDescription::CLIENT)	// all client methods are exposed
			methodDescription.setExposed();

		methodDescription.internalIndex( internalMethods_.size() );
		internalMethods_.push_back( methodDescription );

		if (methodDescription.isExposed())
		{
			internalMethods_.back().exposedIndex( exposedMethods_.size() );
			exposedMethods_.push_back( methodDescription.internalIndex() );
		}

		if (map_.find( methodDescription.name() ) != map_.end())
		{
			ERROR_MSG( "EntityMethodDescriptions::init: "
					"method %s appears more than once\n",
				methodDescription.name().c_str() );
		}

		map_[ methodDescription.name() ] = methodDescription.internalIndex();

		iter++;
	}

	this->checkExposedForSubSlots();

	this->checkExposedForPythonArgs( interfaceName );

	return true;
}

void EntityMethodDescriptions::checkExposedForPythonArgs(
		const char * interfaceName )
{

	for (uint eindex = 0; eindex < exposedMethods_.size(); eindex++)
	{
		uint iindex = exposedMethods_[eindex];
		MethodDescription & mdesc = internalMethods_[iindex];

		if (mdesc.hasPythonArg())
		{
			WARNING_MSG( "%s.%s is an Exposed method but takes a PYTHON arg "
					"(potential security hole)\n",
				interfaceName, mdesc.name().c_str() );
		}
	}
}


/**
 *	Helper method to check for subslots and tell MethodDescriptions about them
 */
void EntityMethodDescriptions::checkExposedForSubSlots()
{
	int numExposed = (int)exposedMethods_.size();
	int numSubSlots = (numExposed-63 + 255) / 255;
	int begSubSlot = 62 - numSubSlots;

	if (numSubSlots <= 0) return;
	// never have to reset MethodDescription...  once it's been sub-slotted
	// it will always remain so, even when derived from

	for (uint eindex = 0; eindex < exposedMethods_.size(); eindex++)
	{
		uint iindex = exposedMethods_[eindex];
		MethodDescription & mdesc = internalMethods_[iindex];

		int overBy = eindex - begSubSlot;
		if (overBy < 0)
			mdesc.exposedIndex( eindex, -1 );
		else
			mdesc.exposedIndex( begSubSlot + (overBy>>8), uint8(overBy) );
	}
}


/**
 *	This method supersedes the methods in this collection.
 */
void EntityMethodDescriptions::supersede()
{
	map_.clear();
	for (List::iterator it = internalMethods_.begin();
		it != internalMethods_.end(); it++)
	{
		std::string & str = const_cast<std::string&>( it->name() );
		str = "old_" + str;

		map_[ it->name() ] = it - internalMethods_.begin();
	}
}


/**
 *	This method returns the number of methods associated with this entity.
 *
 *	@return The number of methods associated with this entity.
 */
unsigned int EntityMethodDescriptions::size() const
{
	return internalMethods_.size();
}


/**
 *	This method returns the description of the method associated with this
 *	entity that has the input index number.
 */
MethodDescription * EntityMethodDescriptions::internalMethod(
		unsigned int index ) const
{
	if (index < internalMethods_.size())
	{
		return const_cast<MethodDescription *>( &internalMethods_[ index ] );
	}
	else
	{
		ERROR_MSG( "EntityDescription::serverMethod: "
				"Do not have server method %d. There are only %d.\n"
				"	Check that entities.xml is up-to-date.\n",
				(int)index, (int)internalMethods_.size() );
		return NULL;
	}
}


/**
 *	This method returns the description of the exposed method associated with
 *	this entity that has the input index number.
 */
MethodDescription * EntityMethodDescriptions::exposedMethod(
	uint8 topIndex, BinaryIStream & data ) const
{
	int numExposed = (int)exposedMethods_.size();
	// see if topIndex implies a subIndex on the stream
	int numSubSlots = (numExposed-63 + 255) / 255;
	int begSubSlot = 62 - numSubSlots;
	int curSubSlot = topIndex - begSubSlot;
	int index = curSubSlot < 0 ? topIndex :
		(begSubSlot + (curSubSlot<<8) + *(uint8*)data.retrieve( 1 ));

	if (index < numExposed)
	{
		int internalIndex = exposedMethods_[ index ];
		return const_cast<MethodDescription *>(
			&internalMethods_[ internalIndex ] );
	}
	else
	{
		ERROR_MSG( "EntityDescription::serverMethod: "
				"Do not have exposed method %d. There are only %d.\n"
				"	Check that entities.xml is up-to-date.\n",
				(int)index, (int)exposedMethods_.size() );
		return NULL;
	}
}


/**
 *	This method returns the description of the server method with the input
 *	name.
 */
MethodDescription *
	EntityMethodDescriptions::find( const std::string & name ) const
{
	Map::const_iterator iter = map_.find( name );

	return (iter != map_.end()) ? this->internalMethod( iter->second ) : NULL;
}

#if ENABLE_WATCHERS

WatcherPtr EntityMethodDescriptions::pWatcher()
{
	typedef SmartPointer< SequenceWatcher< List > > SequenceWatcherPtr;

	static SequenceWatcherPtr watchMe = NULL;

	if (watchMe == NULL)
	{
		EntityMethodDescriptions * pNull = NULL;
		watchMe = new SequenceWatcher< List >( pNull->internalMethods_ );
		watchMe->setLabelSubPath( "name" );
		watchMe->addChild( "*", MethodDescription::pWatcher() ); 
	}

	return watchMe;
}
#endif // ENABLE_WATCHERS


// entity_method_descriptions.cpp
