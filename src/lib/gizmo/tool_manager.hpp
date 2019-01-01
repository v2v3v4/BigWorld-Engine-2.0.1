/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TOOL_MANAGER_HPP
#define TOOL_MANAGER_HPP

#include "tool.hpp"
#include <stack>

class ToolManager
{
public:
	static ToolManager & instance();


	void	pushTool( ToolPtr tool );
	void	popTool();
	ToolPtr tool();
	void changeSpace( const Vector3& worldRay );

private:
	ToolManager();

	ToolManager( const ToolManager& );
	ToolManager& operator=( const ToolManager& );

	typedef std::vector<ToolPtr>	ToolStack;
	ToolStack	tools_;
};

#endif // TOOL_MANAGER_HPP
