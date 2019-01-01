/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIGHT_CONTAINER_HPP
#define LIGHT_CONTAINER_HPP

#include <iostream>

#include "moo_math.hpp"
#include "directional_light.hpp"
#include "omni_light.hpp"
#include "spot_light.hpp"

#include "cstdmf/smartpointer.hpp"

namespace Moo
{

typedef SmartPointer< class LightContainer > LightContainerPtr;

/**
 *	A bucket of lights grouped to light regions of a world.
 *	Contains helper methods to sort, setup and transform the lights for 
 *	use.
 */
class LightContainer : public SafeReferenceCount
{
public:
	LightContainer( const LightContainerPtr & pLC, const BoundingBox& bb, bool limitToRenderable = false, bool dynamicOnly = false );
	LightContainer();
	~LightContainer();

	void							assign( const LightContainerPtr & pLC );

	void							init( const LightContainerPtr & pLC, const BoundingBox& bb, 
		bool limitToRenderable = false, bool dynamicOnly = false );

	void							addToSelf( const LightContainerPtr & pLC, bool addOmnis=true, bool addSpots=true, bool addDirectionals=true  );
	void							addToSelf( const LightContainerPtr & pLC, const BoundingBox& bb,
		bool limitToRenderable = false, bool dynamicOnly = false );

	const Colour&					ambientColour( ) const;
	void							ambientColour( const Colour& colour );

	const DirectionalLightVector&	directionals( ) const;
	DirectionalLightVector&			directionals( );
	bool							addDirectional( const DirectionalLightPtr & pDirectional, bool checkExisting=false );
	bool							delDirectional( const DirectionalLightPtr & pDirectional );
	uint32							nDirectionals( ) const;
	DirectionalLightPtr				directional( uint32 i ) const;

	const OmniLightVector&			omnis( ) const;
	OmniLightVector&				omnis( );
	bool							addOmni( const OmniLightPtr & pOmni, bool checkExisting=false );
	bool							delOmni( const OmniLightPtr & pOmni );
	uint32							nOmnis( ) const;
	OmniLightPtr					omni( uint32 i ) const;

	const SpotLightVector&			spots( ) const;
	SpotLightVector&				spots( );
	bool							addSpot( const SpotLightPtr & pSpot, bool checkExisting=false );
	bool							delSpot( const SpotLightPtr & pSpot );
	uint32							nSpots( ) const;
	SpotLightPtr					spot( uint32 i ) const;

	bool							addLight( const OmniLightPtr & pOmni, bool checkExisting=false );
	bool							delLight( const OmniLightPtr & pOmni );
	bool							addLight( const SpotLightPtr & pSpot, bool checkExisting=false );
	bool							delLight( const SpotLightPtr & pSpot );
	bool							addLight( const DirectionalLightPtr & pDir, bool checkExisting=false );
	bool							delLight( const DirectionalLightPtr & pDir );

	void							addExtraOmnisInWorldSpace( ) const;
	void							addExtraOmnisInModelSpace( const Matrix& invWorld ) const;

	static void						addLightsInWorldSpace( );
	static void						addLightsInModelSpace( const Matrix& invWorld );

	void							commitToFixedFunctionPipeline();

	void							operator=( const LightContainerPtr& pLC );

private:

	Colour					ambientColour_;

	DirectionalLightVector	directionalLights_;
	OmniLightVector			omniLights_;
	SpotLightVector			spotLights_;

/*	LightContainer(const LightContainer&);
	LightContainer& operator=(const LightContainer&);*/

	friend std::ostream& operator<<(std::ostream&, const LightContainer&);
};

}

#ifdef CODE_INLINE
#include "light_container.ipp"
#endif




#endif // LIGHT_CONTAINER_HPP
