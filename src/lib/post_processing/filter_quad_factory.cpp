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
#include "filter_quad_factory.hpp"

BW_SINGLETON_STORAGE( PostProcessing::FilterQuadCreators );

namespace PostProcessing
{
	
void FilterQuadFactory::registerType( const std::string& label, FilterQuadCreator creator )
{
	static PostProcessing::FilterQuadCreators s_creators;
	INFO_MSG( "Registering factory for %s\n", label.c_str() );
	FilterQuadCreators::instance().insert( std::make_pair(label, creator) );
}


/**
 *	This method loads the given section assuming it is a filter quad
 */
/*static*/FilterQuad* FilterQuadFactory::loadItem( DataSectionPtr pSection )
{
	BW_GUARD;
	
	FilterQuadCreators::iterator found = FilterQuadCreators::instance().find( pSection->sectionName() );
	if (found != FilterQuadCreators::instance().end())
		return (*found->second)( pSection );

	return NULL;
}


FilterQuad* FilterQuadFactory::create( DataSectionPtr pDS )
{
	return creator_(pDS);
}


}	//namespace PostProcessing