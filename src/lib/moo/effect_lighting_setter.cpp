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
#include "effect_lighting_setter.hpp"
#include "managed_effect.hpp"

namespace Moo
{


EffectLightingSetter::EffectLightingSetter( ManagedEffectPtr pEffect ):
	pEffect_( pEffect ),
	lastBatch_( NO_BATCH )
{
	BW_GUARD;
	if (pEffect_)
	{
		//Cache all lighting specific semantics.
		this->addSemantic( "Ambient" );
		this->addSemantic( "DirectionalLightCount" );
		this->addSemantic( "PointLightCount" );
		this->addSemantic( "SpotLightCount" );
		this->addSemantic( "DirectionalLightsObjectSpace" );
		this->addSemantic( "PointLightsObjectSpace" );
		this->addSemantic( "SpotLightsObjectSpace" );
		this->addSemantic( "DirectionalLights" );
		this->addSemantic( "PointLights" );
		this->addSemantic( "SpotLights" );
		this->addSemantic( "SpecularDirectionalLightCount" );
		this->addSemantic( "SpecularPointLightCount" );
		this->addSemantic( "SpecularDirectionalLightsObjectSpace" );
		this->addSemantic( "SpecularPointLightsObjectSpace" );
		this->addSemantic( "SpecularDirectionalLights" );
		this->addSemantic( "SpecularPointLights" );
	}
}


/**
 *	This method looks up a semantic by name, and adds it to the
 *	semantics cache if it exists.
 *
 *	@param		name		name of the semantic to try and cache.
 */
void EffectLightingSetter::addSemantic( const std::string& name )
{
	BW_GUARD;
	D3DXHANDLE h;
	h = pEffect_->pEffect()->GetParameterBySemantic(NULL, name.c_str());

	if (h != NULL)
	{
		SemanticMapping sm;
		sm.name_ = name;
		sm.handle_ = h;
		sm.value_ = NULL;
		semantics_.push_back( sm );
	}
}


/**
 *	This method should be called once before using a material multiple
 *	times.  It retrieves all the current EffectConstantValues that will
 *	be used during multiple calls to apply.
 */
void EffectLightingSetter::begin()
{
	BW_GUARD;
	//Cache all the semantics used by the material.
	for (uint32 i=0; i<semantics_.size(); i++)
	{
		SemanticMapping& sm = semantics_[i];
		sm.value_ = EffectConstantValue::get( sm.name_ );
	}
}


/**
 *	This method applies the lighting container to the effect.
 *	TODO : It will only set those semantics that have changed
 *	from the previous lighting container.
 *	NOW : It applies every semantic used by the effect.
 *
 *	@param	pDiffuse		The diffuse lighting container.
 *	@param	pSpecular		The specular lighting container.
 *	@param	commitChanges	Set to true if the apply method should
 *	call CommitChanges on the effect.  Only set this to true if
 *	you are not going to set any other variables on the effect before
 *	rendering.
 *	@param	batch			If not NO_BATCHING, and the same as the
 *                          last BatchCookie provided, avoid reapplying
 *                          previously set lighting.
 */
void EffectLightingSetter::apply(
	LightContainerPtr pDiffuse,
	LightContainerPtr pSpecular,
	bool commitChanges/* = true*/,
	BatchCookie batch/* = NO_BATCHING*/)
{	
	BW_GUARD;
	if (batch == lastBatch_)
		return;		

	if (semantics_.size())
	{
		LightContainerPtr pOldLC = rc().lightContainer();
		LightContainerPtr pOldSLC = rc().specularLightContainer();
		
		rc().lightContainer( pDiffuse );
		rc().specularLightContainer( pSpecular );

		ID3DXEffect* pEffect = pEffect_->pEffect();

		for (uint32 i=0; i<semantics_.size(); i++)
		{
			SemanticMapping& sm = semantics_[i];
			(**sm.value_)(pEffect, sm.handle_);
		}

		if (commitChanges)
		{
			pEffect->CommitChanges();
		}

		rc().lightContainer( pOldLC );
		rc().specularLightContainer( pOldSLC );
	}

	if (batch != NO_BATCHING)
		lastBatch_ = batch;
}


}	//namespace Moo
