/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP

#include <iostream>
#include <vector>

#include <cstdmf/stdmf.hpp>
#include <cstdmf/smartpointer.hpp>
#include "cstdmf/vectornodest.hpp"

#include "moo_math.hpp"
namespace Moo {

class DirectionalLight;
typedef SmartPointer< DirectionalLight > DirectionalLightPtr;

/**
 * A directional light source (ala sunlight).
 */
class DirectionalLight : public SafeReferenceCount
{
public:
	DirectionalLight();
	DirectionalLight( const Colour& colour, const Vector3& direction_ );
	~DirectionalLight();

	const Vector3&		direction( ) const;
	void				direction( const Vector3& direction );

	const Colour&		colour( ) const;
	void				colour( const Colour& colour );

#ifdef EDITOR_ENABLED
	float				multiplier() const { return multiplier_; }
	void				multiplier( float m ) { multiplier_ = m; }
#else//EDITOR_ENABLED
	float				multiplier() const { return 1.0f; }
#endif


	void				worldTransform( const Matrix& transform );

	const Vector3&		worldDirection( ) const;
private:

	Vector3				direction_;
	Vector3				worldDirection_;

	Colour				colour_;

#ifdef EDITOR_ENABLED
	float				multiplier_;
#endif


// Allow the default copy constructor and assignment operator
//	DirectionalLight(const DirectionalLight&);
//	DirectionalLight& operator=(const DirectionalLight&);

	friend std::ostream& operator<<(std::ostream&, const DirectionalLight&); 
};
}
//typedef std::vector< Moo::DirectionalLightPtr > DirectionalLightVector;
typedef VectorNoDestructor< Moo::DirectionalLightPtr > DirectionalLightVector;

#ifdef CODE_INLINE
#include "directional_light.ipp"
#endif




#endif // DIRECTIONAL_LIGHT_HPP
