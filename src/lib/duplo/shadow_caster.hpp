/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHADOW_CASTER_HPP
#define SHADOW_CASTER_HPP

#include "moo/render_target.hpp"
#include "math/boundbox.hpp"

/**
 *	This class implements the ShadowCaster object, the shadow caster
 *	implements the shadow buffer and handles rendering to and rendering
 *	with the shadow buffer. The shadow buffer is implemented as a 
 *	floating point texture.
 */

namespace Moo
{
	class EffectMaterial;
}

class ShadowCasterCommon;
typedef SmartPointer<class ChunkItem> ChunkItemPtr;

class ShadowCaster : public ReferenceCount, public Aligned
{
public:
	ShadowCaster();

	bool init( ShadowCasterCommon* pCommon, bool halfRes, uint32 idx, Moo::RenderTarget* pDepthParent );
	void begin( const BoundingBox& worldSpaceBB, const Vector3& lightDirection );
	void end();

	void beginReceive( bool useTerrain = true );
	void endReceive();

	void setupMatrices( const BoundingBox& worldSpaceBB, const Vector3& lightDirection );

	void angleIntensity( float angleIntensity ) { angleIntensity_ = angleIntensity; }

	const BoundingBox& aaShadowVolume() const	{ return aaShadowVolume_; }

	Moo::RenderTarget* shadowTarget() { return pShadowTarget_.getObject(); }

	void setupConstants( Moo::EffectMaterial& effect );
private:
	void pickReceivers();	
	Matrix	view_;
	Matrix	projection_;
	Matrix	viewport_;
	Matrix	shadowProjection_;
	Matrix	viewProjection_;
	BoundingBox aaShadowVolume_;

	RECT	scissorRect_;

	ShadowCasterCommon* pCommon_;

	typedef std::vector<ChunkItemPtr> ShadowItems;
	ShadowItems shadowItems_;

	Moo::RenderTargetPtr pShadowTarget_;

	float	angleIntensity_;

	uint32 shadowBufferSize_;

	ShadowCaster( const ShadowCaster& );
	ShadowCaster& operator=( const ShadowCaster& );
};

typedef SmartPointer<ShadowCaster> ShadowCasterPtr;

#endif // SHADOW_CASTER_HPP
