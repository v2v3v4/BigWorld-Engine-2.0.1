/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RAIN_HPP
#define RAIN_HPP

#include "chunk/chunk_manager.hpp"
#include "moo/material.hpp"
#include "moo/node.hpp"
#include "moo/vertex_formats.hpp"
#include "math/vector3.hpp"
#include "fog_controller.hpp"

#include <iostream>
#include <vector>

class Weather;
class PyMetaParticleSystem;
class SourcePSA;
class TintShaderPSA;
//class PAColour;

/**
 * TODO: to be documented.
 */
class Rain
{
public:
	Rain();
	~Rain();

	void activate( const class EnviroMinder&, DataSectionPtr pSpaceSettings );
	void deactivate( const class EnviroMinder& );

	void		draw();
	void		drawInside( ChunkBoundary::Portal*, Chunk* );
	void		tick( float dt );

	void		amount( float a );
	float		amount() const;

	void		update( const Weather& w, bool outside );

	void		addAttachments( class PlayerAttachments & pa );

	//called by enviro minder
	void		addFogEmitter();
	void		remFogEmitter();

	static void	disable( bool state );

private:
	Rain(const Rain&);
	Rain& operator=(const Rain&);

	void				buildMaterial( void );
	void				createRainSplashes( float density );
	//void				createRainSplashesForAvatar( float density );

	Moo::Material		material_;
	int					numRainBits_;
	float				sphereSize_;
	int					minPseudoDepth_;
	int					maxPseudoDepth_;
	int					baseIllumination_;
	float				velocitySensitivity_;
	Vector3				wind_;
	Vector3				lastCameraPos_;
	float				amount_;
	float				maxFarFog_;
	float				maxNearFog_;
	PyMetaParticleSystem*	rainSplashes_;
	//ParticleSystem *	headRainSplashes_;
	//ParticleSystem *	leftShoulderRainSplashes_;
	//ParticleSystem *	rightShoulderRainSplashes_;
	std::vector<SourcePSA*> rainSplashSources_;
	std::vector<float>	maxRates_;
	std::vector<TintShaderPSA*> rainSplashTints_;
	std::vector<float>	maxTintAlphas_;
	//PAColour *			headRainColour_;
	//PAColour *			leftShoulderRainColour_;
	//PAColour *			rightShoulderRainColour_;
	Moo::NodePtr		cameraNode_;
	FogController::Emitter emitter_;
	int					emitterID_;
	bool				outside_;	
	bool				transiting_;
	float				transitionFactor_;
	Vector3				randomOffset();

	static bool			s_disable;

	Moo::VertexXYZDUV verts_[4];


	friend std::ostream& operator<<(std::ostream&, const Rain&);
};

#ifdef CODE_INLINE
#include "rain.ipp"
#endif




#endif
/*rain.hpp*/
