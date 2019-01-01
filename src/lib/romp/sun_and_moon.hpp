/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SUN_AND_MOON_HPP
#define SUN_AND_MOON_HPP

#pragma warning ( disable: 4503 )

#include <iostream>
#include "custom_mesh.hpp"
#include "lens_effect.hpp"
#include "moo/effect_material.hpp"

class TimeOfDay;
typedef SmartPointer< class ShaderSet > ShaderSetPtr;

/**
 * TODO: to be documented.
 */
class SunAndMoon
{
public:
	SunAndMoon();
	~SunAndMoon();

	void				timeOfDay( TimeOfDay* timeOfDay )		{ timeOfDay_ = timeOfDay; }

	void				create( void );
	void				destroy( void );

	void				draw();	

private:
	void				createMoon( void );
	void				createSun( void );

	void drawSquareMesh( const Matrix & world, CustomMesh<Moo::VertexXYZNDUV> * pMesh, Moo::EffectMaterialPtr pMat );

	SunAndMoon(const SunAndMoon&);
	SunAndMoon& operator=(const SunAndMoon&);

	friend std::ostream& operator<<(std::ostream&, const SunAndMoon&);
	
	CustomMesh<Moo::VertexXYZNDUV>*	moon_;
	CustomMesh<Moo::VertexXYZNDUV>*	sun_;
	
	Moo::EffectMaterialPtr	moonMat_;
	Moo::EffectMaterialPtr	sunMat_;	

	TimeOfDay*			timeOfDay_;

	LensEffect			sunLensEffect_;	
};

#ifdef CODE_INLINE
#include "sun_and_moon.ipp"
#endif




#endif
/*sun_and_moon.hpp*/
