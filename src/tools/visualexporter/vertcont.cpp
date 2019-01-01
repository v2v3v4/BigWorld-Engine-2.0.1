/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma warning (disable : 4530 )

#include "vertcont.hpp"


VertexContainer::VertexContainer()
{
	v_ = tv_ = 0;
}

VertexContainer::VertexContainer(int v, int tv)
{
	v_ = v; tv_ = tv;
}


VertexContainer::~VertexContainer()
{
}

VertexContainer::VertexContainer(const VertexContainer &vc)
{
	*this = vc;
}
VertexContainer& VertexContainer::operator = (const VertexContainer &vc)
{
	v_ = vc.v_;
	tv_ = vc.tv_;

	return *this;
}
bool VertexContainer::operator == (const VertexContainer &vc) const
{
	return ( ( v_ == vc.v_ ) && ( tv_ == vc.tv_ ) );
}

int VertexContainer::getV(void)
{
	return v_;
}
int VertexContainer::getTV(void)
{
	return tv_;
}

void VertexContainer::setV( int i)
{
	v_ = i;
}
void VertexContainer::setTV( int i)
{
	tv_ = i;
}

// vertcont.cpp
