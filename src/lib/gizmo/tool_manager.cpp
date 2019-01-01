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
#include "tool_manager.hpp"

DECLARE_DEBUG_COMPONENT2( "Tool", 2 );

ToolManager::ToolManager()
{
}

ToolManager & ToolManager::instance()
{
	static ToolManager s_instance;

	return s_instance;
}

void ToolManager::pushTool( ToolPtr tool )
{
	BW_GUARD;

	tool->onPush();
	tools_.push_back( tool );
}


void ToolManager::popTool()
{
	BW_GUARD;

	if ( !tools_.empty() )
	{
		(*tools_.rbegin())->onPop();
		tools_.pop_back();
	}
}

ToolPtr ToolManager::tool()
{
	BW_GUARD;

	if ( !tools_.empty() )
		return (*tools_.rbegin());

	return NULL;
}

void ToolManager::changeSpace( const Vector3& worldRay )
{
	BW_GUARD;

	ToolStack ts = tools_;
	for( ToolStack::iterator iter = ts.begin(); iter != ts.end(); ++iter )
	{
		(*iter)->calculatePosition( worldRay );
		(*iter)->update( 0.f );
	}
}
