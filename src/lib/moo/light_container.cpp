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

#include "light_container.hpp"
#include "render_context.hpp"
#include "math/boundbox.hpp"

#ifndef CODE_INLINE
#include "light_container.ipp"
#endif

namespace Moo
{

PROFILER_DECLARE( LightContainer_init, "LightContainer Init" );

/**
 *	Constructor
 */
LightContainer::LightContainer( const LightContainerPtr & pLC, const BoundingBox& bb, bool limitToRenderable, bool dynamicOnly )
{
	BW_GUARD;
	init( pLC, bb, limitToRenderable );
}

/**
 *	Constructor
 */
LightContainer::LightContainer()
:	ambientColour_( 0.1f, 0.1f, 0.1f, 0 )
{
}


/**
 *	Helper class used to sort the omni lights by their attenuation.
 */
class SortOmnis
{
public:
	SortOmnis( const BoundingBox& bb )
	: bb_( bb )
	{
	}

	bool operator () ( OmniLightPtr& p1, OmniLightPtr& p2 )
	{
		if (p1->priority() != p2->priority())
		{
			return p1->priority() > p2->priority();
		}
		else
		{
			return p1->attenuation( bb_ ) > p2->attenuation( bb_ );
		}
	}

	BoundingBox bb_;
};

/**
 *	Helper class used to sort spot lights by their attenuation and cone.
 */
class SortSpots
{
public:
	SortSpots( const BoundingBox& bb )
	: bb_( bb )
	{
	}

	bool operator () ( SpotLightPtr& p1, SpotLightPtr& p2 )
	{
		if (p1->priority() != p2->priority())
		{
			return p1->priority() > p2->priority();
		}
		else
		{
			return p1->attenuation( bb_ ) > p2->attenuation( bb_ );
		}
	}

	BoundingBox bb_;
};


void LightContainer::assign( const LightContainerPtr & pLC )
{
	ambientColour_ = pLC->ambientColour_;
	spotLights_.assign( pLC->spotLights_.begin(), pLC->spotLights_.end() );
	omniLights_.assign( pLC->omniLights_.begin(), pLC->omniLights_.end() );
	directionalLights_.assign( pLC->directionalLights_.begin(), pLC->directionalLights_.end() );
}

void LightContainer::init( const LightContainerPtr & pLC, const BoundingBox& bb, bool limitToRenderable, bool dynamicOnly )
{
	BW_GUARD_PROFILER( LightContainer_init );

	if( pLC )
	{

		ambientColour_ = pLC->ambientColour_;

		directionalLights_ = pLC->directionalLights_;
		while (limitToRenderable && directionalLights_.size() > 2)
		{
			directionalLights_.pop_back();
		}

		omniLights_.clear();
		
//		for (uint32 i = 0; ( i < pLC->nOmnis() ) && !( limitToRenderable && omniLights_.size() >= 4 ) ; i++ )
		if (!dynamicOnly)
		{

			OmniLightVector::iterator it = pLC->omniLights_.begin();
			OmniLightVector::iterator end = pLC->omniLights_.end();

			while (it != end)
			{
				if ((*it)->intersects( bb ))
				{
					omniLights_.push_back( *it );
				}
				++it;
			}

/*			for (uint32 i = 0; ( i < pLC->nOmnis() ); i++ )
			{
				OmniLightPtr pOmni = pLC->omni( i );
				if( pOmni->intersects( bb ) )
				{
					omniLights_.push_back( pOmni );
				}
			}*/
		}
		else
		{
			OmniLightVector::iterator it = pLC->omniLights_.begin();
			OmniLightVector::iterator end = pLC->omniLights_.end();

			while (it != end)
			{
				if ((*it)->dynamic() && (*it)->intersects( bb ))
				{
					omniLights_.push_back( *it );
				}
				++it;
			}
/*			for (uint32 i = 0; ( i < pLC->nOmnis() ); i++ )
			{
				OmniLightPtr pOmni = pLC->omni( i );
				if( pOmni->dynamic() && pOmni->intersects( bb ) )
				{
					omniLights_.push_back( pOmni );
				}
			}*/
		}

		std::sort( omniLights_.begin(), omniLights_.end(), SortOmnis(bb) );
		if (limitToRenderable)
		{
			if (omniLights_.size() > 4 )
				omniLights_.resize( 4 );
		}

		spotLights_.clear();
		for (uint32 i = 0; i < pLC->nSpots(); i++)
		{
			SpotLightPtr pSpot = pLC->spot( i );
			if (dynamicOnly && !pSpot->dynamic())
				continue;

			if( pSpot->intersects( bb ) )
			{
				spotLights_.push_back( pSpot );
			}
		}

		std::sort( spotLights_.begin(), spotLights_.end(), SortSpots(bb) );
		if (limitToRenderable)
		{
			if (spotLights_.size() > 2 )
				spotLights_.resize( 2 );
		}
	}
	else
	{
		ambientColour_ = Colour( 0, 0, 0, 0 );
		directionalLights_.clear();
		omniLights_.clear();
		spotLights_.clear();
	}
}


/**
 *	Copy these lights into our own container.
 */
void LightContainer::addToSelf( const LightContainerPtr & pLC, bool addOmnis, bool addSpots, bool addDirectionals  )
{
	BW_GUARD;
	if (pLC)
	{
		if (addDirectionals)
		{
			for ( uint32 i = 0; i < pLC->nDirectionals(); i++ )
			{
				DirectionalLightPtr pDir = pLC->directional( i );
				if (std::find( this->directionalLights_.begin(),
							this->directionalLights_.end(), pDir ) == this->directionalLights_.end() )
				{
					directionalLights_.push_back( pDir );
				}
			}
		}

		if (addOmnis)
		{
			for ( uint32 i = 0; i < pLC->nOmnis(); i++ )
			{
				OmniLightPtr pOmni = pLC->omni( i );
				if (std::find( this->omniLights_.begin(),
							this->omniLights_.end(), pOmni ) == this->omniLights_.end() )
				{
					omniLights_.push_back( pOmni );
				}
			}
		}

		if (addSpots)
		{
			for ( uint32 i = 0; i < pLC->nSpots(); i++ )
			{
				SpotLightPtr pSpot = pLC->spot( i );
				if (std::find( this->spotLights_.begin(),
							this->spotLights_.end(), pSpot ) == this->spotLights_.end() )
				{
					spotLights_.push_back( pSpot );
				}
			}
		}
	}
}

/**
 *	Copy these lights into our own container.
 */
void LightContainer::addToSelf( const LightContainerPtr & pLC, const BoundingBox & bb,
	bool limitToRenderable, bool dynamicOnly )
{
	BW_GUARD;
	if (pLC)
	{
		/*
		ambientColour_.r = (ambientColour_.r + pLC->ambientColour_.r)/2;
		ambientColour_.g = (ambientColour_.g + pLC->ambientColour_.g)/2;
		ambientColour_.b = (ambientColour_.b + pLC->ambientColour_.b)/2;
		ambientColour_.a = (ambientColour_.a + pLC->ambientColour_.a)/2;
		*/
		/*
		if (pLC->ambientColour_ != Colour( 0, 0, 0, 0 ) )
		{
			ambientColour_ = ( ambientColour_ + pLC->ambientColour_ ) / 2.f;
		}
		*/

		for (uint32 i = 0; ( i < pLC->nDirectionals() ) && !( limitToRenderable && directionalLights_.size() > 2 ) ; i++ )
		{
			DirectionalLightPtr pDir = pLC->directional( i );
			bool good = true;
			for (uint32 j=0; j < this->nDirectionals(); j++)
			{
				if (this->directional(j) == pDir) good = false;
			}
			if (!good) continue;
			directionalLights_.push_back( pDir );
		}

		for (uint32 i = 0; i < pLC->nOmnis(); i++ )
		{
			OmniLightPtr pOmni = pLC->omni( i );

			if( pOmni->intersects( bb ) || pOmni->dynamic())
			{
				bool good = true;
				for (uint32 j=0; j < this->nOmnis(); j++)
				{
					if (this->omni(j) == pOmni) good = false;
				}
				if (!good) continue;
				if ( (dynamicOnly && pOmni->dynamic()) ||
					!dynamicOnly )
					omniLights_.push_back( pOmni );
			}
		}

		std::sort( omniLights_.begin(), omniLights_.end(), SortOmnis(bb) );
		if (limitToRenderable)
		{
			if (omniLights_.size() > 4 )
				omniLights_.resize( 4 );
		}

		for (uint32 i = 0; i < pLC->nSpots(); i++ )
		{
			SpotLightPtr pSpot = pLC->spot( i );
			if (dynamicOnly && !pSpot->dynamic())
				continue;

			if( pSpot->intersects( bb ) )
			{
				bool good = true;
				for (uint32 j=0; j < this->nSpots(); j++)
				{
					if (this->spot(j) == pSpot) good = false;
				}
				if (!good) continue;
				spotLights_.push_back( pSpot );
			}
		}

		std::sort( spotLights_.begin(), spotLights_.end(), SortSpots(bb) );
		if (limitToRenderable)
		{
			if (spotLights_.size() > 2 )
				spotLights_.resize( 2 );
		}
	}
}

// Helper function for adding vertex shader constants incrementally
static inline void addVertexShaderConstant( int& constantIndex, const float* constants, int count )
{
	BW_GUARD;
	Moo::rc().device()->SetVertexShaderConstantF(  constantIndex, constants,  count );
	constantIndex += count;
}


/**
 *	Adds a maximum of two omnis in world space to the space set aside for spot lights.
 */
void LightContainer::addExtraOmnisInWorldSpace( ) const
{
	BW_GUARD;
	uint8	nPointLights = 0;
	int constCount = 33;

	// iterate through and add omni lights.
	// TODO: cull them, probably not the place to do it though...

	for( OmniLightVector::const_iterator it = omniLights_.begin(); ( it != omniLights_.end() ) && ( nPointLights < 4 ); it++ )
	{
		const OmniLightPtr omni = *it;
//		if( omni->intersects( bb ) )
		{
			Vector4 position;
			addVertexShaderConstant( constCount, (const float*)&omni->worldPosition(),  1 );
			Colour c = omni->colour() * omni->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = omni->worldInnerRadius();
			float outerRadius = omni->worldOuterRadius();

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), 0, 0 ),  1 );

			nPointLights++;
		}
	}
}

/**
 *	Adds a maximum of two omnis in model space to the space set aside for spot lights.
 *
 *	@param invWorld the inverse world transform, used to transform lights to object space.
 */
void LightContainer::addExtraOmnisInModelSpace( const Matrix& invWorld ) const
{
	BW_GUARD;
	uint8	nPointLights = 0;
	int constCount = 33;

	float invWorldScale = XPVec3Length( &Vector3Base( invWorld ) );

	// iterate through, transform and add omni lights.
	// TODO: cull them, probably not the place to do it though...

	for( OmniLightVector::const_iterator it = omniLights_.begin(); ( it != omniLights_.end() ) && ( nPointLights < 2 ); it++ )
	{
		OmniLightPtr omni = *it;
//		if( omni->intersects( bb ) )
		{
			Vector4 position;
			addVertexShaderConstant( constCount, (const float*)XPVec3Transform( &position, &omni->worldPosition(), &invWorld ),  1 );
			Colour c = omni->colour() * omni->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = omni->worldInnerRadius() * invWorldScale;
			float outerRadius = omni->worldOuterRadius() * invWorldScale;

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), 0, 0 ),  1 );

			nPointLights++;
		}
	}
}


/**
 *	Adds the current lights to the shader constants, in object space.
 *
 *	@param invWorld the inverse world transform, used to transform lights to object space.
 */
void LightContainer::addLightsInModelSpace( const Matrix& invWorld )
{
	BW_GUARD;
	LightContainerPtr pLC = Moo::rc().lightContainer();

	int constCount = 16;

	// Ambient colour
	// TODO: move this outside.
	addVertexShaderConstant( constCount, (const float*)pLC->ambientColour(),  1 );
	float invWorldScale = XPVec3Length( &Vector3Base( invWorld ) );

	uint8	nDirectionalLights = 0;

	constCount = 17;

	// iterate through, transform and add directional lights.
	// TODO: find a way of culling them

	for( DirectionalLightVector::iterator it = pLC->directionals().begin(); ( it != pLC->directionals().end() ) && ( nDirectionalLights < 2 ); it++ )
	{
		DirectionalLightPtr directional = *it;
		Vector4 localDirection;
		XPVec3TransformNormal( (Vector3*)&localDirection, &directional->worldDirection(), &invWorld );
		XPVec3Normalize( (Vector3*)&localDirection, (Vector3*)&localDirection );

		Colour c = directional->colour() * directional->multiplier();
		addVertexShaderConstant( constCount, (const float*)&localDirection,  1 );	
		addVertexShaderConstant( constCount, (const float*)&c,  1 );

		nDirectionalLights++;
	}

	uint8	nPointLights = 0;
	constCount = 21;

	// iterate through, transform and add omni lights.
	// TODO: cull them, probably not the place to do it though...

	for( OmniLightVector::iterator it = pLC->omnis().begin(); ( it != pLC->omnis().end() ) && ( nPointLights < 4 ); it++ )
	{
		OmniLightPtr omni = *it;
//		if( omni->intersects( bb ) )
		{
			Vector4 position;
			addVertexShaderConstant( constCount, (const float*)XPVec3Transform( &position, &omni->worldPosition(), &invWorld ),  1 );
			Colour c = omni->colour() * omni->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = omni->worldInnerRadius() * invWorldScale;
			float outerRadius = omni->worldOuterRadius() * invWorldScale;

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), 0, 0 ),  1 );

			nPointLights++;
		}
	}

	uint8 nSpotLights = 0;
	constCount = 33;

	// iterate through, transform and add spot lights.
	// TODO: cull them, probably not here.

	for( SpotLightVector::iterator it = pLC->spots().begin(); ( it != pLC->spots().end() ) && ( nSpotLights < 2 ); it++ )
	{
		SpotLightPtr spot = *it;
//		if( spot->intersects( bb ) )
		{
			Vector4 position;
			addVertexShaderConstant( constCount, (const float*)XPVec3Transform( &position, &spot->worldPosition(), &invWorld ),  1 );
			Colour c = spot->colour() * spot->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = spot->worldInnerRadius() * invWorldScale;
			float outerRadius = spot->worldOuterRadius() * invWorldScale;

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), spot->cosConeAngle(), 0 ),  1 );

			Vector4 localDirection;
			XPVec3TransformNormal( (Vector3*)&localDirection, &spot->worldDirection(), &invWorld );
			XPVec3Normalize( (Vector3*)&localDirection, (Vector3*)&localDirection );
			addVertexShaderConstant( constCount, (const float*)&localDirection,  1 );	

			nSpotLights++;
		}
	}
}

/**
 *	Adds the current lights to the shader constants, in world space.
 *
 */
void LightContainer::addLightsInWorldSpace( )
{
	BW_GUARD;
	LightContainerPtr pLC = Moo::rc().lightContainer();

	int constCount = 16;

	// Ambient colour
	// TODO: move this outside.
	addVertexShaderConstant( constCount, (const float*)pLC->ambientColour(),  1 );

	uint8	nDirectionalLights = 0;

	constCount = 17;

	// iterate through and add directional lights.
	// TODO: find a way of culling them

	for( DirectionalLightVector::iterator it = pLC->directionals().begin(); ( it != pLC->directionals().end() ) && ( nDirectionalLights < 2 ); it++ )
	{
		DirectionalLightPtr directional = *it;

		Colour c = directional->colour() * directional->multiplier();
		addVertexShaderConstant( constCount, (const float*)&directional->worldDirection(),  1 );
		addVertexShaderConstant( constCount, (const float*)&c,  1 );

		nDirectionalLights++;
	}

	uint8	nPointLights = 0;
	constCount = 21;

	// iterate through and add omni lights.
	// TODO: cull them, probably not the place to do it though...

	for( OmniLightVector::iterator it = pLC->omnis().begin(); ( it != pLC->omnis().end() ) && ( nPointLights < 4 ); it++ )
	{
		OmniLightPtr omni = *it;
//		if( omni->intersects( bb ) )
		{
			addVertexShaderConstant( constCount, (const float*)&omni->worldPosition(),  1 );
			Colour c = omni->colour() * omni->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = omni->worldInnerRadius();
			float outerRadius = omni->worldOuterRadius();

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), 0, 0 ),  1 );

			nPointLights++;
		}
	}

	uint8 nSpotLights = 0;
	constCount = 33;

	// iterate through and add spot lights.
	// TODO: cull them, probably not here.

	for( SpotLightVector::iterator it = pLC->spots().begin(); ( it != pLC->spots().end() ) && ( nSpotLights < 2 ); it++ )
	{
		SpotLightPtr spot = *it;
//		if( spot->intersects( bb ) )
		{
			addVertexShaderConstant( constCount, (const float*)&spot->worldPosition(),  1 );
			Colour c = spot->colour() * spot->multiplier();
			addVertexShaderConstant( constCount, (const float*)&c,  1 );

			float innerRadius = spot->worldInnerRadius();
			float outerRadius = spot->worldOuterRadius();

			addVertexShaderConstant( constCount, (const float*)Vector4( outerRadius, 1.f / ( outerRadius - innerRadius ), spot->cosConeAngle(), 0 ),  1 );
			addVertexShaderConstant( constCount, (const float*)&spot->worldDirection(),  1 );	

			nSpotLights++;
		}
	}

}


void LightContainer::commitToFixedFunctionPipeline()
{
	BW_GUARD;
	DX::Material d3dmaterial;
	d3dmaterial.Diffuse = Moo::Colour( 1, 1, 1, 1 );
	d3dmaterial.Ambient = Moo::Colour( 1, 1, 1, 1 );
	d3dmaterial.Specular = Moo::Colour( 0, 0, 0, 0 );
	d3dmaterial.Emissive = ambientColour();
	d3dmaterial.Power = 0;
	rc().device()->SetMaterial( &d3dmaterial );
    rc().setRenderState( D3DRS_LIGHTING, TRUE );

	int lightIndex = 0;

	//Add directional lights
	uint numDirectionals = directionals().size();
	uint i;
	for ( i = 0; i < numDirectionals; i++ )
	{
		DX::Light light;

		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse = directionals()[i]->colour() * directionals()[i]->multiplier();
		light.Specular = Colour( 0, 0, 0, 0 );
		light.Ambient = Colour( 0, 0, 0, 0 );
		light.Position = Vector3( 0, 0, 0 );
		light.Direction = directionals()[i]->worldDirection();

		light.Attenuation0 = 1;
		light.Attenuation1 = 0;
		light.Attenuation2 = 0;
		light.Range = 0;

		rc().device()->SetLight( lightIndex, &light );
		rc().device()->LightEnable( lightIndex++, TRUE );
	}

	//Add spotlights
	uint numSpots = spots().size();
	for ( i = 0; i < numSpots; i++ )
	{
		DX::Light light;

		light.Type = D3DLIGHT_SPOT;
		light.Diffuse = spots()[i]->colour() * spots()[i]->multiplier();
		light.Specular = Colour( 0, 0, 0, 0 );
		light.Ambient = Colour( 0, 0, 0, 0 );
		light.Position = spots()[i]->worldPosition();
		light.Direction = spots()[i]->worldDirection();

		light.Attenuation0 = 1;
		light.Attenuation1 = 0;
		light.Attenuation2 = 0;
		light.Range = 0;
		light.Theta = acosf( spots()[i]->cosConeAngle() ) * 2.f;
		light.Phi = acosf( spots()[i]->cosConeAngle() ) * 2.f;

		rc().device()->SetLight( lightIndex, &light );
		rc().device()->LightEnable( lightIndex++, TRUE );
	}

	//Add omnis
	uint numOmnis = omnis().size();
	for ( i = 0; i < numOmnis; i++ )
	{
		DX::Light light;

		light.Type = D3DLIGHT_POINT;
		light.Diffuse = omnis()[i]->colour() * omnis()[i]->multiplier();
		light.Specular = Colour( 0, 0, 0, 0 );
		light.Ambient = Colour( 0, 0, 0, 0 );
		light.Position = omnis()[i]->worldPosition();
		light.Direction = Vector3( 0, 0, 0 );
		light.Attenuation0 = 1;
		light.Attenuation1 = 0;
		light.Attenuation2 = 0;
		light.Range = omnis()[i]->outerRadius();

		rc().device()->SetLight( lightIndex, &light );
		rc().device()->LightEnable( lightIndex++, TRUE );
	}

	//disable the next light, and all lights following.
	rc().device()->LightEnable( lightIndex, FALSE );
}


/**
 *	Destructor
 */
LightContainer::~LightContainer()
{
}


}

// light_container.cpp
