/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
    #define INLINE    inline
#else
 	/// INLINE macro.
    #define INLINE
#endif


/**
 *	This constructor sets this vector to the zero vector.
 */
INLINE
Vector3::Vector3()
{
}


/**
 *	This constructor sets the elements of the vector to the input values.
 *
 *	@param a	The value that element 0 is set to.
 *	@param b	The value that element 1 is set to.
 *	@param c	The value that element 2 is set to.
 */
INLINE
Vector3::Vector3( float a, float b, float c ) : Vector3Base( a, b, c )
{
}


/**
 *	This method constructs this Vector3 from Vector3Base. If we are using
 *	DirectX, this is D3DXVECTOR3.
 */
INLINE
Vector3::Vector3( const Vector3Base & v3 )
{
	*static_cast< Vector3Base * >( this ) = v3;
}


/**
 *	This method constructs this Vector3 from an SSE register
 */
#ifdef _WIN32
INLINE
Vector3::Vector3( __m128 v4 )
{
	__m128 vy = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	__m128 vz = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 2, 2, 2, 2 ) );
	_mm_store_ss( &x, v4 );
	_mm_store_ss( &y, vy );
	_mm_store_ss( &z, vz );
}
#endif


/**
 *	This method sets all elements of the vector to 0.
 */
INLINE
void Vector3::setZero()
{
	x = y = z = 0;
}


/**
 *	This method sets the elements of the vector to the input values.
 *
 *	@param a	The value that element x is set to.
 *	@param b	The value that element y is set to.
 *	@param c	The value that element z is set to.
 */
INLINE
void Vector3::set( float a, float b, float c )
{
	x = a;
	y = b;
	z = c;
};


#if 0
/**
 *	This method sets this vector to the input vector scaled by the input float.
 */
INLINE
void Vector3::scale( const Vector3& v, float s )
{
	x = v.x * s;
	y = v.y * s;
	z = v.z * s;
};
#endif


/**
 *	This method returns the dot product of this vector and the input vector.
 *
 *	@param v	The vector to perform the dot product with.
 *
 *	@return The dot product of this vector and the input vector.
 */
INLINE
float Vector3::dotProduct( const Vector3& v ) const
{
#ifdef EXT_MATH
	return XPVec3Dot( this, &v );
#else
	return x * v.x + y * v.y + z * v.z;
#endif
};


/**
 *	This method sets this vector to the cross product of the input vectors.
 */
INLINE
void Vector3::crossProduct( const Vector3& v1, const Vector3& v2 )
{
#ifdef EXT_MATH
	XPVec3Cross( this, &v1, &v2 );
#else
	// TODO: This is wrong if &v1 or &v2 is this.
	x = (v1.y * v2.z) - (v1.z * v2.y);
	y = (v1.z * v2.x) - (v1.x * v2.z);
	z = (v1.x * v2.y) - (v1.y * v2.x);
#endif
}


/**
 * Porduces a linear interpolation of v1 an v2 based on alpha. 
 *   alpha = 0.0    result = v1
 *   alpha = 0.5    result = (v1+v2)/2 
 *   alpha = 1.0    result = v2 
 *
 *	@param v1		
 *	@param v2		
 *	@param alpha	
 */
INLINE
void Vector3::lerp( const Vector3 & v1, const Vector3 & v2, float alpha )
{
	// result = ( v1 * ( 1 - alpha ) ) + ( v2 * alpha )
	*this = v2;
	*this -= v1;
	*this *= alpha;
	*this += v1;
}


/**
 *	Component wise clamp.
 *
 *	@param lower		
 *	@param upper		
 */
INLINE
void Vector3::clamp( const Vector3 & lower, const Vector3 & upper )
{
	x = Math::clamp( lower.x, x, upper.x );
	y = Math::clamp( lower.y, y, upper.y );
	z = Math::clamp( lower.z, z, upper.z );
}


/**
 *	This method returns the cross product of this vector with the input vector.
 */
INLINE
Vector3 Vector3::crossProduct(const Vector3 & v) const
{
	Vector3 result;
	result.crossProduct( *this, v );

	return result;
}


/**
 *	This method sets this vector to vector1 projected onto vector2.
 */
INLINE
void Vector3::projectOnto( const Vector3& v1, const Vector3& v2 )
{
	*this = v2 * ( v1.dotProduct( v2 ) / v2.lengthSquared() );
}


/**
 *	This method returns this vector projected onto the input vector.
 */
INLINE
Vector3 Vector3::projectOnto(const Vector3 & v) const
{
	Vector3 result;
	result.projectOnto( *this, v );

	return result;
}


/**
 *	This method returns the length of this vector.
 */
INLINE
float Vector3::length() const
{
#ifdef EXT_MATH
	return XPVec3Length( this );
#else

#ifdef __BORLANDC__
	return sqrt((x * x) + (y * y) + (z * z));
#else
	return sqrtf((x * x) + (y * y) + (z * z));
#endif // __BORLANDC__

#endif // EXT_MATH
}


/**
 *	This method returns the length of this vector squared.
 */
INLINE
float Vector3::lengthSquared() const
{
#ifdef EXT_MATH
	return XPVec3LengthSq( this );
#else
	return this->dotProduct( *this );
#endif
}


/**
 *	This method normalises this vector. That is, the direction of the vector
 *	will stay the same and its length will become 1.
 */
INLINE
void Vector3::normalise()
{
#ifdef EXT_MATH
	XPVec3Normalize( this, this );
#else
	const float length = this->length();

	if (!almostZero( length, 0.00000001f ) )
	{
		float rcp = 1.f / length;
		*this *= rcp;
	}
#endif
}


/**
 *	This function returns a copy of this vector that has been normalised to
 *	unit length.
 *
 *	@return		A copy of this vector normalised to unit length.
 */
INLINE
Vector3 Vector3::unitVector() const
{
	Vector3 result( *this );
	result.normalise();
	return result;
}


/**
 *	This method adds the input vector to this vector.
 */
INLINE
void Vector3::operator +=( const Vector3& v )
{
	x += v.x;
	y += v.y;
	z += v.z;
}


/**
 *	This method subtracts the input vector from this vector.
 */
INLINE
void Vector3::operator -=( const Vector3& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}


/**
 *	This method scales this vector by the input value.
 */
INLINE
void Vector3::operator *=( float s )
{
	x *= s;
	y *= s;
	z *= s;
}


/**
 *	This method divides the vector by the input value.
 */
INLINE
void Vector3::operator /=( float s )
{
	float divisor = 1.f / s;
	x *= divisor;
	y *= divisor;
	z *= divisor;
}

INLINE
Vector3 Vector3::operator-() const
{
	Vector3 that;
	that.x = -x;
	that.y = -y;
	that.z = -z;
	return that;
}

/**
 *	This function returns the sum of the two input vectors.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator +( const Vector3& v1, const Vector3& v2 )
{
	return Vector3( v1.x+v2.x, v1.y+v2.y, v1.z+v2.z );
}


/**
 *	This function returns the result of subtracting v2 from v1.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator -( const Vector3& v1, const Vector3& v2 )
{
	return Vector3( v1.x-v2.x, v1.y-v2.y, v1.z-v2.z );
}


/**
 *	This function returns the input vector scaled by the input float.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator *( const Vector3& v, float s )
{
//	Vector3 result( v );
//	result *= s;

//	return result;
	return Vector3( v.x * s, v.y * s, v.z * s );
}


/**
 *	This function returns the input vector scaled by the input float.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator *( float s, const Vector3& v )
{
	return Vector3( v.x * s, v.y * s, v.z * s );
}


/**
 *	This function returns the input vector scaled down by the input float.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator /( const Vector3& v, float s )
{
	float oos = 1.f / s;
	return Vector3( v[0] * oos, v[1] * oos, v[2] * oos );
}


/**
 *	This function returns a vector whose elements are the product of the
 *	corresponding elements of the input vector.
 *
 *	@relates Vector3
 */
INLINE
Vector3 operator *( const Vector3& a, const Vector3& b )
{
	return Vector3( a.x*b.x, a.y*b.y, a.z*b.z );
}

/**
 *	This function returns whether or not two vectors are equal. Two vectors are
 *	considered equal if all of their corresponding elements are equal.
 *
 *	@return True if the input vectors are equal, otherwise false.
 */
INLINE
bool operator   ==( const Vector3& v1, const Vector3& v2 )
{
	return ( v1[0] == v2[0] ) & ( v1[1] == v2[1] ) & ( v1[2] == v2[2] );
}


/**
 *	This function returns whether or not two vectors are not equal. Two vectors
 *	are considered equal if all of their corresponding elements are equal.
 *
 *	@return True if the input vectors are not equal, otherwise false.
 */
INLINE
bool operator   !=( const Vector3& v1, const Vector3& v2 )
{
	return !(( v1[0] == v2[0] ) && ( v1[1] == v2[1] ) && ( v1[2] == v2[2] ));
}


/**
 *	This function returns whether or not the vector on the left is less than
 *	the vector on the right. A vector is considered less than another if
 *	its x element is less than the other. Or if the x elements are equal,
 *	then the y elements are compared, and so on.
 *
 *	@return True if the input vectors are not equal, otherwise false.
 */
INLINE bool operator < ( const Vector3& v1, const Vector3& v2 )
{
	if (v1.x < v2.x) return true;
	if (v1.x > v2.x) return false;
	if (v1.y < v2.y) return true;
	if (v1.y > v2.y) return false;
	return (v1.z < v2.z);
}

/**
 *	This method sets this vector to the direction vector with the input pitch
 *	and yaw. The vector will have unit length.
 */
INLINE void Vector3::setPitchYaw( float pitchInRadians, float yawInRadians )
{
	double cosPitch = cos( pitchInRadians );
	double sinPitch = sin( -pitchInRadians );

	double cosYaw = cos( yawInRadians );
	double sinYaw = sin( yawInRadians );

	x = float(cosPitch * sinYaw);
	y = float(sinPitch);
	z = float(cosPitch * cosYaw);
}


/**
 *	This method returns the pitch of this vector when it is considered as a
 *	direction vector.
 */
INLINE float Vector3::pitch() const
{
	return -atan2f( y, sqrtf( x*x + z*z ) );
}


/**
 *	This method returns the yaw of this vector when it is considered as a
 *	direction vector.
 */
INLINE float Vector3::yaw() const
{
	return atan2f( x, z );
}

// vector3.ipp
