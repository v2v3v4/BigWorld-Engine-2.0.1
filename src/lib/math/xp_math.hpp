/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef XP_MATH_HPP
#define XP_MATH_HPP

// This file contains some cross platform math related stuff.

#include <math.h>
#include "cstdmf/stdmf.hpp"

#if defined(_WIN32)

#include "cstdmf/aligned.hpp"

#ifndef USE_XG_MATH
#include "d3dx9math.h"
#define DIRECTX_MATH
#define EXT_MATH

#include <emmintrin.h>

typedef D3DXMATRIX MatrixBase;
typedef D3DXQUATERNION QuaternionBase;
typedef D3DXVECTOR2 Vector2Base;
typedef D3DXVECTOR3 Vector3Base;
typedef D3DXVECTOR4 Vector4Base;

#define XPVec2Length D3DXVec2Length
#define XPVec2LengthSq D3DXVec2LengthSq
#define XPVec2Normalize D3DXVec2Normalize
#define XPVec2Dot D3DXVec2Dot
#define XPVec3Length D3DXVec3Length
#define XPVec3LengthSq D3DXVec3LengthSq
#define XPVec3Dot D3DXVec3Dot
#define XPVec3Cross D3DXVec3Cross
#define XPVec3Normalize D3DXVec3Normalize
#define XPVec3Lerp D3DXVec3Lerp
#define XPVec3Transform D3DXVec3Transform
#define XPVec3TransformCoord D3DXVec3TransformCoord
#define XPVec3TransformNormal D3DXVec3TransformNormal
#define XPVec4Transform D3DXVec4Transform
#define XPVec4Length D3DXVec4Length
#define XPVec4LengthSq D3DXVec4LengthSq
#define XPVec4Normalize D3DXVec4Normalize
#define XPVec4Lerp D3DXVec4Lerp
#define XPVec4Dot D3DXVec4Dot
#define XPMatrixIdentity D3DXMatrixIdentity
#define XPMatrixInverse D3DXMatrixInverse
#define XPMatrixRotationQuaternion D3DXMatrixRotationQuaternion
#define XPMatrixTranspose D3DXMatrixTranspose
#define XPMatrixfDeterminant D3DXMatrixDeterminant
#define XPMatrixOrthoLH D3DXMatrixOrthoLH
#define XPMatrixLookAtLH D3DXMatrixLookAtLH
#define XPMatrixMultiply D3DXMatrixMultiply
#define XPMatrixPerspectiveFovLH D3DXMatrixPerspectiveFovLH
#define XPMatrixRotationX D3DXMatrixRotationX
#define XPMatrixRotationY D3DXMatrixRotationY
#define XPMatrixRotationZ D3DXMatrixRotationZ
#define XPMatrixScaling D3DXMatrixScaling
#define XPMatrixTranslation D3DXMatrixTranslation
#define XPMatrixRotationYawPitchRoll D3DXMatrixRotationYawPitchRoll
#define XPQuaternionDot D3DXQuaternionDot
#define XPQuaternionNormalize D3DXQuaternionNormalize
#define XPQuaternionRotationMatrix D3DXQuaternionRotationMatrix
#define XPQuaternionSlerp D3DXQuaternionSlerp
#define XPQuaternionRotationAxis D3DXQuaternionRotationAxis
#define XPQuaternionMultiply D3DXQuaternionMultiply
#define XPQuaternionInverse D3DXQuaternionInverse

#else

#include <xgmath.h>
#define XG_MATH
#define EXT_MATH

#include "cstdmf/debug.hpp"

/**
 *	This class is a wrapper of the XGMatrix class, which is the Matrix
 *	implementation of the XGMath library.
 */
class MatrixBase : public XGMATRIX, public Aligned
{
public:
	MatrixBase()
	{
		MF_ASSERT( (((int) this) & 0x0F) == 0 );
	}
	MatrixBase( const MatrixBase & other ) : XGMATRIX( other )
	{
		MF_ASSERT( (((int) this) & 0x0F) == 0 );
	}
};

typedef XGQUATERNION QuaternionBase;
typedef XGVECTOR2 Vector2Base;
typedef XGVECTOR3 Vector3Base;
typedef XGVECTOR4 Vector4Base;


#define XPVec2Length XGVec2Length
#define XPVec2LengthSq XGVec2LengthSq
#define XPVec2Normalize XGVec2Normalize
#define XPVec2Dot XGVec2Dot
#define XPVec3Length XGVec3Length
#define XPVec3LengthSq XGVec3LengthSq
#define XPVec3Dot XGVec3Dot
#define XPVec3Cross XGVec3Cross
#define XPVec3Normalize XGVec3Normalize
#define XPVec3Lerp XGVec3Lerp
#define XPVec3Transform XGVec3Transform
#define XPVec3TransformCoord XGVec3TransformCoord
#define XPVec3TransformNormal XGVec3TransformNormal
#define XPVec4Transform XGVec4Transform
#define XPVec4Length XGVec4Length
#define XPVec4LengthSq XGVec4LengthSq
#define XPVec4Normalize XGVec4Normalize
#define XPVec4Lerp XGVec4Lerp
#define XPVec4Dot XGVec4Dot
#define XPMatrixIdentity XGMatrixIdentity
#define XPMatrixInverse XGMatrixInverse
#define XPMatrixRotationQuaternion XGMatrixRotationQuaternion
#define XPMatrixTranspose XGMatrixTranspose
#define XPMatrixfDeterminant XGMatrixfDeterminant
#define XPMatrixOrthoLH XGMatrixOrthoLH
#define XPMatrixLookAtLH XGMatrixLookAtLH
#define XPMatrixMultiply XGMatrixMultiply
#define XPMatrixPerspectiveFovLH XGMatrixPerspectiveFovLH
#define XPMatrixRotationX XGMatrixRotationX
#define XPMatrixRotationY XGMatrixRotationY
#define XPMatrixRotationZ XGMatrixRotationZ
#define XPMatrixScaling XGMatrixScaling
#define XPMatrixTranslation XGMatrixTranslation
#define XPMatrixRotationYawPitchRoll XGMatrixRotationYawPitchRoll
#define XPQuaternionDot XGQuaternionDot
#define XPQuaternionNormalize XGQuaternionNormalize
#define XPQuaternionRotationMatrix XGQuaternionRotationMatrix
#define XPQuaternionSlerp XGQuaternionSlerp
#define XPQuaternionRotationAxis XGQuaternionRotationAxis
#define XPQuaternionMultiply XGQuaternionMultiply
#define XPQuaternionInverse XGQuaternionInverse

#endif


#else // not _WIN32
/**
 *	This class implements a vector of 2 floats for non-windows platforms.
 */
struct Vector2Base
{
	Vector2Base() {};
	Vector2Base( float _x, float _y ) : x( _x ), y( _y )
	{
	}

	operator float *()				{ return (float *)&x; }
	operator const float *() const	{ return (float *)&x; }

	float x, y;
};

/**
 * This class implements a vector of 3 floats for non-windows platforms.
 */
struct Vector3Base
{
	Vector3Base() {};
	Vector3Base( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z )
	{
	}

	operator float *()				{ return (float *)&x; }
	operator const float *() const	{ return (float *)&x; }

	float x, y, z;
};

/**
 * This class implements a vector of 4 floats for non-windows platforms.
 */
struct Vector4Base
{
	Vector4Base() {};
	Vector4Base( float _x, float _y, float _z, float _w ) :
		x( _x ), y( _y ), z( _z ), w( _w )
	{
	}

	operator float *()				{ return (float *)&x; }
	operator const float *() const	{ return (float *)&x; }

	float x, y, z, w;
};

/**
 * This class implements a 4x4 Matrix for non-windows platforms.
 */
struct MatrixBase
{
	operator float *()				{ return (float *)&m00; }
	operator const float *() const	{ return (float *)&m00; }

	union
	{
		float m[4][4];
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
	};
};

/**
 * This class implements a Quaternion for non-windows platforms.
 */
struct QuaternionBase
{
	QuaternionBase() {};
	QuaternionBase( float _x, float _y, float _z, float _w ) :
		x( _x ), y( _y ), z( _z ), w( _w )
	{
	}

	operator float *()				{ return (float *)&x; }
	operator const float *() const	{ return (float *)&x; }

	float x, y, z, w;
};

#define XPVec3Length(v) (v)->length()
#define XPVec3Cross(out,a,b) (out)->crossProduct(*(a),*(b))
#define XPVec3Dot(a,b) (a)->dotProduct(*(b))
#define XPVec3Lerp(out,a,b,s) (out)->lerp(*(a),*(b),(s))
#define XPQuaternionRotationMatrix(out,in) (out)->fromMatrix(*(in))
#define XPQuaternionSlerp(out,a,b,s) (out)->slerp(*(a),*(b),(s))

#endif // _WIN32

#endif // XP_MATH_HPP
