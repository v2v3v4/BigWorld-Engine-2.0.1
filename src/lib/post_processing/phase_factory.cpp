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
#include "phase_factory.hpp"

BW_SINGLETON_STORAGE( PostProcessing::PhaseCreators );

namespace PostProcessing
{

void PhaseFactory::registerType( const std::string& label, PhaseCreator creator )
{
	static PostProcessing::PhaseCreators s_creators;
	INFO_MSG( "Registering factory for %s\n", label.c_str() );
	PhaseCreators::instance().insert( std::make_pair(label, creator) );
}


/**
 *	This method loads the given section assuming it is a phase
 */
/*static*/Phase* PhaseFactory::loadItem( DataSectionPtr pSection )
{
	BW_GUARD;
	
	PhaseCreators::iterator found = PhaseCreators::instance().find( pSection->sectionName() );
	if (found != PhaseCreators::instance().end())
		return (*found->second)( pSection );

	return NULL;
}


Phase* PhaseFactory::create( DataSectionPtr pDS )
{
	return creator_(pDS);
}


}	//namespace PostProcessing