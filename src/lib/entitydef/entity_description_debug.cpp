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

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "entity_description_debug.hpp"
#include "entity_description_map.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )

namespace EntityDescriptionDebug
{
void dump( const MethodDescription & mDesc, int detailLevel )
{
	DEBUG_MSG( "\t\t%2d/%2d %s\n",
		mDesc.exposedIndex(), mDesc.internalIndex(), mDesc.name().c_str() );
}


/**
 *	This function dumps out information about the input EntityDescription.
 */
void dump( const EntityDescription & eDesc, int detailLevel )
{
	DEBUG_MSG( "%s\n", eDesc.name().c_str() );
	DEBUG_MSG( "hasScript: Client %d, Base %d, Cell %d)\n",
			eDesc.hasClientScript(),
			eDesc.hasBaseScript(),
			eDesc.hasCellScript() );

	DEBUG_MSG( "\tProperties:\n" );
	for (uint i = 0; i < eDesc.propertyCount(); ++i)
	{
		DataDescription * pDD = eDesc.property( i );
		DEBUG_MSG( (detailLevel > 2) ?
						"\t\t%2d/%2d %-12s %-8s %3d %s%s%s\n" :
						"\t\t%2d/%2d %-12s %-8s\n",
			pDD->clientServerFullIndex(),
			pDD->index(),
			pDD->name().c_str(),
			pDD->dataType()->typeName().c_str(),
			pDD->detailLevel(),
			pDD->getDataFlagsAsStr(),
			pDD->isPersistent() ? " DB" : "",
			pDD->isIdentifier() ? " ID" : "" );
	}

	if (eDesc.clientMethodCount())
	{
		DEBUG_MSG( "\tClient Methods:\n" );
		for (uint i = 0; i < eDesc.clientMethodCount(); ++i)
		{
			dump( *eDesc.client().internalMethod( i ), detailLevel );
		}
	}

	if (eDesc.base().size())
	{
		DEBUG_MSG( "\tBase Methods:\n" );
		for (uint i = 0; i < eDesc.base().size(); ++i)
		{
			dump( *eDesc.base().internalMethod( i ), detailLevel );
		}
	}

	if (eDesc.cell().size())
	{
		DEBUG_MSG( "\tCell Methods:\n" );
		for (uint i = 0; i < eDesc.cell().size(); ++i)
		{
			dump( *eDesc.cell().internalMethod( i ), detailLevel );
		}
	}

	if (detailLevel > 2)
	{
		DEBUG_MSG( "\tVolatile Info:\n" );
		DEBUG_MSG( "\t\tposition %f\n", eDesc.volatileInfo().positionPriority() );
		DEBUG_MSG( "\t\tyaw %f\n", eDesc.volatileInfo().yawPriority() );
		DEBUG_MSG( "\t\tpitch %f\n", eDesc.volatileInfo().pitchPriority() );
		DEBUG_MSG( "\t\troll %f\n", eDesc.volatileInfo().rollPriority() );

#ifdef MF_SERVER
		DEBUG_MSG( "\tLoD Levels (%d):\n", eDesc.lodLevels().size() );

		for (int i = 0; i < eDesc.lodLevels().size(); ++i)
		{
			const DataLoDLevel & level = eDesc.lodLevels().getLevel( i );
			DEBUG_MSG( "\t\t%10s %10.0f %10.0f %10.0f %10.0f\n",
					level.label().c_str(),
					(level.start() == FLT_MAX) ? 99999 : level.start(),
					(level.hyst()  == FLT_MAX) ? 99999 : level.hyst(),
					(level.low()   == FLT_MAX) ? 99999 : level.low(),
					(level.high()  == FLT_MAX) ? 99999 : level.high() );
		}
#endif
	}
}


/**
 *	This function dumps out information about the input EntityDescriptionMap.
 */
void dump( const EntityDescriptionMap & map, int detailLevel )
{
	if (detailLevel < 1) return;

	int numTypes = map.size();
	DEBUG_MSG( "Num types = %d\n", numTypes );

	DEBUG_MSG( "ID    : %-15sExposed/Not Exp.  Client,  Base,  Cell\n",
			"Name");

	for (EntityTypeID typeIndex = 0; typeIndex < numTypes; ++typeIndex)
	{
		const EntityDescription & eDesc = map.entityDescription( typeIndex );

		DEBUG_MSG( "%2d(%2d): %-15s Props %2d/%2d. Methods %2d, %2d/%-2d, %2d/%-2d\n",
			eDesc.index(),
			eDesc.clientIndex(),
			eDesc.name().c_str(),
			eDesc.clientServerPropertyCount(),
			eDesc.propertyCount(),
			eDesc.clientMethodCount(),
			eDesc.base().exposedSize(),
			eDesc.base().size(),
			eDesc.cell().exposedSize(),
			eDesc.cell().size() );
	}

	if (detailLevel < 2) return;

	for (EntityTypeID typeIndex = 0; typeIndex < numTypes; ++typeIndex)
	{
		const EntityDescription & eDesc = map.entityDescription( typeIndex );
		dump( eDesc, detailLevel );
	}
}

} // namespace EntityDescriptionDebug

// entity_description_debug.cpp
