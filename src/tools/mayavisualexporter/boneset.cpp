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

#include "boneset.hpp"

void BoneSet::addBone( std::string bone )
{
	if( indexes.find( bone ) != indexes.end() )
		return; // already added
	
	bones.push_back( bone );
	indexes[bone] = bones.size() - 1;
}
