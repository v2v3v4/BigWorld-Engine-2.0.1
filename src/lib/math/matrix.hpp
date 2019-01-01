/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <math.h>
#include <string.h>

#include "xp_math.hpp"

#include "cstdmf/stdmf.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

class Quaternion;

/**
 *	This class provides a 4x4 matrix and associated operations. This class is
 *	most often used to represent a linear transformation from one 3D space to
 *	another.
 *
 *	@ingroup Math
 */
class Matrix : public MatrixBase
{

public:
	Matrix();
	
	INLINE
	Matrix( const Vector4& v0,
			const Vector4& v1,
			const Vector4& v2,
			const Vector4& v3 );

	INLINE void	setZero();
	INLINE void	setIdentity();

	void	setScale( const float x, const float y, const float z );
	void	setScale( const Vector3 & scale );

	void	setTranslate( const float x, const float y, const float z );
	void	setTranslate( const Vector3 & pos );

	void	setRotateX( const float angle );
	void	setRotateY( const float angle );
	void	setRotateZ( const float angle );
	void	setRotate( const Quaternion & q );

	void	setRotate( float yaw, float pitch, float roll );
	void	setRotateInverse( float yaw, float pitch, float roll );

	void	multiply( const Matrix& m1, const Matrix& m2 );
	void	preMultiply( const Matrix& m );
	void	postMultiply( const Matrix& m );

	void	invertOrthonormal( const Matrix& m );
	void	invertOrthonormal();
	bool	invert( const Matrix& m );
	bool	invert();
	float	getDeterminant() const;

	void	transpose( const Matrix & m );
	void	transpose();

	void	lookAt( const Vector3& position,
				const Vector3& direction, const Vector3& up );

	float&	operator ()( uint32 column, uint32 row );
	float	operator ()( uint32 column, uint32 row ) const;

	Vector3	applyPoint( const Vector3& v2 ) const;
	void	applyPoint( Vector3&v1, const Vector3& v2) const;
	void	applyPoint( Vector4&v1, const Vector3& v2) const;
	void	applyPoint( Vector4&v1, const Vector4& v2) const;

	INLINE Vector3	applyVector( const Vector3& v2 ) const;
	INLINE void		applyVector( Vector3& v1, const Vector3& v2 ) const;

	const Vector3 & applyToUnitAxisVector( int axis ) const;
	const Vector3 & applyToOrigin() const;

	INLINE Vector3 & operator []( int i );
	INLINE const Vector3 & operator []( int i ) const;

	INLINE void				row( int i, const Vector4 & value );
	INLINE const Vector4&	row( int i ) const;

	Vector4 column( int i ) const;
	void column( int i, const Vector4 & v );

	void preRotateX(const float angle);
	void preRotateY(const float angle);
	void preRotateZ(const float angle);
	void preTranslateBy(const Vector3 & v);

	void postRotateX(const float angle);
	void postRotateY(const float angle);
	void postRotateZ(const float angle);
	void postTranslateBy(const Vector3 & v);

	bool isMirrored() const;

	void orthogonalProjection( float w, float h, float zn, float zf );
	void perspectiveProjection( float fov, float aspectRatio, 
		float nearPlane, float farPlane );

	void translation( const Vector3& v );

	float yaw() const;
	float pitch() const;
	float roll() const;

	static const Matrix identity;

private:
};

typedef Matrix Matrix34;
typedef Matrix Matrix44;

#ifdef CODE_INLINE
	#include "matrix.ipp"
#endif

#endif

// matrix.hpp
