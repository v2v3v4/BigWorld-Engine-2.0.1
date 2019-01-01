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
#include "stipple_draw_override.hpp"
#include "material_draw_override.hpp"
#include "resmgr/auto_config.hpp"
#include "moo/effect_constant_value.hpp"


// -----------------------------------------------------------------------------
// Section: EffectConstantValues used by the StippleDrawOverride class.
// -----------------------------------------------------------------------------
AutoConfigString s_stippleMapName( "system/stippleMap" );

/**
 * TODO: to be documented.
 */
class StippleMapSetter : public Moo::EffectConstantValue
{
public:
	StippleMapSetter()
	{
		pTexture_ = Moo::TextureManager::instance()->get( s_stippleMapName );
	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		pEffect->SetTexture(constantHandle, pTexture_->pTexture());
		return true;
	}
	Moo::BaseTexturePtr pTexture_;
};

static StippleMapSetter* s_stippleMapSetter = NULL;


// -----------------------------------------------------------------------------
// Section: StippleDrawOverride
// -----------------------------------------------------------------------------
/**
 *	Constructor.
 */
StippleDrawOverride::StippleDrawOverride():
	pDrawOverride_(NULL)
{
	BW_GUARD;
	if ( !s_stippleMapSetter )
	{
		s_stippleMapSetter = new StippleMapSetter;
		*Moo::EffectConstantValue::get( "StippleMap" ) = s_stippleMapSetter;		
	}	
}


/**
 *	Destructor.
 */
StippleDrawOverride::~StippleDrawOverride()
{
}


void StippleDrawOverride::begin()
{
	BW_GUARD;
	if (Moo::Visual::s_pDrawOverride)
		return;

	if (!pDrawOverride_)
	{
		pDrawOverride_ = new MaterialDrawOverride( "shaders/std_effects/stipple/stipple", true );
	}

	Moo::Visual::s_pDrawOverride = pDrawOverride_;	
}

void StippleDrawOverride::end()
{
	Moo::Visual::s_pDrawOverride = NULL;
}

// stipple_draw_override.cpp
