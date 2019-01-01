/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once

#include "resource.h"
#include "common/property_list.hpp"


class BaseView;

namespace
{
	class BasePropertyTableImpl;
}


class BasePropertyTable
{
public:
	
	BasePropertyTable();
	virtual ~BasePropertyTable();
	
	virtual int addView( BaseView* view );

	virtual int addItemsForView( BaseView* view );

	virtual int addItemsForViews();

	virtual void update( int interleaveStep = 0, int maxTimeMS = 0 );
	
	virtual void clear();

	PropertyList* propertyList();

protected:
	std::list< BaseView* > & viewList();

private:
	SmartPointer< BasePropertyTableImpl > pImpl_;
	int guiInterleave_;
};
