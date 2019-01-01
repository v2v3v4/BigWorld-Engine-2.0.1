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
#include "terrain_finder.hpp"

using namespace Terrain;

TerrainFinder::Details::Details() :
    pBlock_(NULL),
    pMatrix_(NULL),
    pInvMatrix_(NULL)
{
}

/*virtual*/ TerrainFinder::~TerrainFinder()
{
}
