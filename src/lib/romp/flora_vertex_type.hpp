/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLORA_VERTEX_TYPE_HPP
#define FLORA_VERTEX_TYPE_HPP

#define PACK_UV	16383.f
#define PACK_FLEX 255.f
typedef short POS_TYPE;
typedef short UV_TYPE;
typedef short ANIM_TYPE;

#pragma pack(push, 1 )

/**
 * TODO: to be documented.
 */
struct FloraVertex
{
	Vector3 pos_;
	short uv_[2];
	short animation_[2];	//flex, animationIdx	

	void set( const Moo::VertexXYZNUV& v )
	{		
		pos_ = v.pos_;
		uv_[0] = (UV_TYPE)(v.uv_[0] * PACK_UV);
		uv_[1] = (UV_TYPE)(v.uv_[1] * PACK_UV);
	}

	void set( const FloraVertex& v, const Matrix* pTransform )
	{		
		pos_ = pTransform->applyPoint( v.pos_ );
		uv_[0] = v.uv_[0];
		uv_[1] = v.uv_[1];
	}

	void flex( float f )
	{
		animation_[0] = (ANIM_TYPE)(PACK_FLEX * f);
	}

	void uv( const Vector2& uv )
	{
		uv_[0] = (UV_TYPE)(uv[0] * PACK_UV);
		uv_[1] = (UV_TYPE)(uv[1] * PACK_UV);
	}
};

#pragma pack( pop )

#endif
