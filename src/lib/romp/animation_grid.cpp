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
#include "animation_grid.hpp"
#include "math/perlin_noise.hpp"
#include "flora_vertex_type.hpp"
#include "cstdmf/debug.hpp"
#include "romp/enviro_minder.hpp"
#include "romp/weather.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

//Multplies the wind by the animation noise grid to get the final
//wind animation value.
static float s_noiseFactor = 0.3f;


FloraAnimationGrid::FloraAnimationGrid( int width, int height ):
	width_( width ),
	height_( height ),
	tTime_( 0.0 )
{
	nConstants_ = width_*height_;
	constants_ = new Vector4[nConstants_];
	noise_ = new float[nConstants_];
	direction_ = new Vector4[nConstants_];
	pAnimGridSetter_ = new AnimationGridSetter(constants_,nConstants_);

	//Create the watchers once only
	static bool s_createdWatchers = false;
	if (!s_createdWatchers)
	{
		MF_WATCH( "Client Settings/Flora/Noise Factor",
			s_noiseFactor,
			Watcher::WT_READ_WRITE,
			"Multiplier for flora noise animation grid.  Affects overall flora movement." );		
		s_createdWatchers = true;
	}
}


FloraAnimationGrid::~FloraAnimationGrid()
{
	if( *Moo::EffectConstantValue::get( "FloraAnimationGrid" ) == pAnimGridSetter_.get() )
	{
		*Moo::EffectConstantValue::get( "FloraAnimationGrid" ) = NULL;
	}
	
	pAnimGridSetter_ = NULL;
	delete[] constants_;
	delete[] noise_;
	delete[] direction_;
}


void FloraAnimationGrid::set( int constantBase )
{
	Moo::rc().device()->SetVertexShaderConstantF( constantBase, Vector4( 0.f, 0.f, 255.f, 0.f ), 1 );
	Moo::rc().device()->SetVertexShaderConstantF( constantBase+1, (float*)constants_, nConstants_ );
}


void FloraAnimationGrid::update( float dTime, EnviroMinder& enviro )
{
	*Moo::EffectConstantValue::get( "FloraAnimationGrid" ) = pAnimGridSetter_;

	float windX = enviro.weather()->wind().x;
	float windZ = enviro.weather()->wind().y;

	tTime_ += (double)dTime;  
	float animationTime = (float)( fmod( tTime_, 1000.0 ) );

	//TODO : this function just animates the x components for now.
	for ( int z=0; z<height_; z++ )
	{
		//remember - 4 values per x value ( a constant is a vector4 )
		for ( int x=0; x<width_; x++ )
		{
			int idx = x+(z*width_);

			Vector4* pVec = &constants_[x+(z*width_)];
			noise_[idx] = perlin_.noise3( Vector3((float)x,(float)z,animationTime) );
			noise_[idx] *= (-1.f * s_noiseFactor);
			pVec->x = noise_[idx] * windX;
			pVec->y = 0.f;
			pVec->z = noise_[idx] * windZ;
			pVec->w = 0.f;
		}
	}
}