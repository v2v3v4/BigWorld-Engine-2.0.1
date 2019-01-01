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
#include "edge.hpp"
#include "node.hpp"


namespace Graph
{


/**
 *	Constructor.
 *
 *	@param start	Starting node.
 *	@param end		Ending node.
 */
Edge::Edge( const NodePtr & start, const NodePtr & end ) :
	start_( start ),
	end_( end )
{
}


/**
 *	Destructor.
 */
Edge::~Edge()
{
}


} // namespace Graph
