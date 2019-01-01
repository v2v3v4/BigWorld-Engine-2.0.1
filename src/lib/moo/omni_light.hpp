/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef OMNI_LIGHT_HPP
#define OMNI_LIGHT_HPP

#include <iostream>
#include "moo_math.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/vectornodest.hpp"
#include "math/boundbox.hpp"


class BoundingBox;
namespace Moo {

class OmniLight;
typedef SmartPointer< OmniLight > OmniLightPtr;

/**
 * This class represents an omni-directional light with a colour, position and
 * an attenuation range.
 */
class OmniLight : public SafeReferenceCount
{
public:
	OmniLight();
	OmniLight( const D3DXCOLOR& colour, const Vector3& position, float innerRadius, float outerRadius );
	~OmniLight();

	const Vector3&		position( ) const;
	void				position( const Vector3& position );
	float				innerRadius( ) const;
	void				innerRadius( float innerRadius );
	float				outerRadius( ) const;
	void				outerRadius( float outerRadius );

	const Colour&		colour( ) const;
	void				colour( const Colour& colour );

	void				worldTransform( const Matrix& transform );

	const Vector3&		worldPosition( void ) const;
	float				worldInnerRadius( void ) const;
	float				worldOuterRadius( void ) const;

	bool				intersects( const BoundingBox& worldSpaceBB ) const;

	Vector4*		getTerrainLight( uint32 timestamp, float lightScale );

	float				attenuation( const BoundingBox& worldSpaceBB ) const;

	bool				dynamic() const { return dynamic_; };
	void				dynamic( bool state ) { dynamic_ = state; };

#ifdef EDITOR_ENABLED
	float				multiplier() const { return multiplier_; }
	void				multiplier( float m ) { multiplier_ = m; }
#else//EDITOR_ENABLED
	float				multiplier() const { return 1.0f; }
#endif

	int					priority() const { return priority_; }
	void				priority(int b) { priority_ = b; }


private:
	void				createTerrainLight( float lightScale );

	Vector3				position_;
	float				innerRadius_;
	float				outerRadius_;
	Colour				colour_;

	Vector3				worldPosition_;
	float				worldInnerRadius_;
	float				worldOuterRadius_;
	bool				dynamic_;

	uint32				terrainTimestamp_;
	Vector4				terrainLight_[3];

	int					priority_;

#ifdef EDITOR_ENABLED
	float				multiplier_;
#endif

// Allow the default copy constructor and assignment operator
//	OmniLight(const OmniLight&);
//	OmniLight& operator=(const OmniLight&);

	friend std::ostream& operator<<(std::ostream&, const OmniLight&);
};
}
//typedef std::vector< Moo::OmniLightPtr > OmniLightVector;
typedef VectorNoDestructor< Moo::OmniLightPtr > OmniLightVector;

#ifdef CODE_INLINE
#include "omni_light.ipp"
#endif




#endif // OMNI_LIGHT_HPP
