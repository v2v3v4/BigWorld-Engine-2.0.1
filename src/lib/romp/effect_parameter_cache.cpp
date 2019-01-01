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
#include "effect_parameter_cache.hpp"
#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )


EffectParameterCache::~EffectParameterCache()
{
	this->spEffect_ = NULL;
}

void EffectParameterCache::cache(const std::string& name )
{		
	D3DXHANDLE handle = spEffect_->GetParameterByName(NULL,name.c_str());
	if (handle)
		parameters_[name] = handle;
	else
		ERROR_MSG( "Could not find parameter %s in EffectMaterial\n", name.c_str() );
}
