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

#include "terrain_texture_setter.hpp"


DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

namespace Terrain
{

EffectFileTextureSetter::~EffectFileTextureSetter()
{
	this->pEffect_ = NULL;
}

} // namespace Terrain

// terrain_texture_setter.cpp
