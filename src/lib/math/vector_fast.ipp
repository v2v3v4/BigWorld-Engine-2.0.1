/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@file
 *
 *	@ingroup Math
 */

// -----------------------------------------------------------------------------

/**
 *
 */
inline
VectorFastBase::VectorFastBase()
{
}


/**
 *
 */
inline
VectorFastBase::VectorFastBase( const VectorFastBase& v )
:	v4( v.v4 )
{
}


/**
 *
 */
inline
VectorFastBase::VectorFastBase( __m128 v )
:	v4( v )
{
}


/**
 *
 */
inline
VectorFastBase::VectorFastBase( float a, float b, float c, float d )
:	v4( _mm_set_ps( d, c, b, a ) )
{
}


/**
 *
 */
inline
VectorFastBase::VectorFastBase( const Vector3& v )
:	v4( _mm_loadu_ps( &v.x ) )
{
}


/**
 *
 */
inline
VectorFastBase::VectorFastBase( const Vector4& v )
:	v4( _mm_loadu_ps( &v.x ) )
{
}


/**
 *
 */
inline
void VectorFastBase::getVector3( Vector3& v ) const
{
	__m128 vy = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	__m128 vz = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 2, 2, 2, 2 ) );

	_mm_store_ss( &v.x, v4 );
	_mm_store_ss( &v.y, vy );
	_mm_store_ss( &v.z, vz );
}



/**
 *
 */
/*
inline
void VectorFastBase::getVector4( Vector4& v) const
{
	_mm_storeu_ps( &v.x, v4 );
}
*/

	
/**
 *
 */
inline
void VectorFastBase::castToInt( int* i ) const
{
	__m128 vy = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	__m128 vz = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 2, 2, 2, 2 ) );
	__m128 vw = _mm_shuffle_ps( v4, v4, _MM_SHUFFLE( 3, 3, 3, 3 ) );

	i[0] = _mm_cvttss_si32( v4 );
	i[1] = _mm_cvttss_si32( vy );
	i[2] = _mm_cvttss_si32( vz );
	i[3] = _mm_cvttss_si32( vw );
}


/**
 *
 */
inline
void VectorFastBase::setZero()
{
	v4 = _mm_setzero_ps();
}


/**
 *
 */
inline
void VectorFastBase::saturate()
{
	v4 = _mm_min_ps( _mm_max_ps( v4, _mm_setzero_ps() ), _mm_set1_ps( 1.f ) );
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator = ( const VectorFastBase& v )
{
	v4 = v.v4;
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator = ( __m128 v )
{
	v4 = v;
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator += ( const VectorFastBase& v )
{
	v4 = _mm_add_ps( v4, v.v4 );
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator -= ( const VectorFastBase& v )
{
	v4 = _mm_sub_ps( v4, v.v4 );
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator *= ( const VectorFastBase& v )
{
	v4 = _mm_mul_ps( v4, v.v4 );
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator *= ( float s )
{
	v4 = _mm_mul_ps( v4, _mm_set1_ps( s ) );
	return *this;
}


/**
 *
 */
inline
VectorFastBase& VectorFastBase::operator /= ( float s )
{
	v4 = _mm_div_ps( v4, _mm_set1_ps( s ) );
	return *this;
}


/**
 *
 */
inline
VectorFastBase VectorFastBase::operator - () const
{
	return VectorFastBase( _mm_sub_ps( _mm_setzero_ps(), v4 ) );
}


// -----------------------------------------------------------------------------


/**
 *
 */
inline
VectorFastBase operator +( const VectorFastBase& v1, const VectorFastBase& v2 )
{
	return VectorFastBase( _mm_add_ps( v1.v4, v2.v4 ) );
}


/**
 *
 */
inline
VectorFastBase operator -( const VectorFastBase& v1, const VectorFastBase& v2 )
{
	return VectorFastBase( _mm_sub_ps( v1.v4, v2.v4 ) );
}


/**
 *
 */
inline
VectorFastBase operator *( const VectorFastBase& v, float s )
{
	return VectorFastBase( _mm_mul_ps( v.v4, _mm_set1_ps( s ) ) );
}


/**
 *
 */
inline
VectorFastBase operator *( float s, const VectorFastBase& v )
{
	return VectorFastBase( _mm_mul_ps( v.v4, _mm_set1_ps( s ) ) );
}


/**
 *
 */
inline
VectorFastBase operator *( const VectorFastBase& v1, const VectorFastBase& v2 )
{
	return VectorFastBase( _mm_mul_ps( v1.v4, v2.v4 ) );
}


/**
 *
 */
inline
VectorFastBase operator /( const VectorFastBase& v, float s )
{
	return VectorFastBase( _mm_div_ps( v.v4, _mm_set1_ps( s ) ) );
}


bool operator ==( const VectorFastBase& v1, const VectorFastBase& v2 );
bool operator !=( const VectorFastBase& v1, const VectorFastBase& v2 );
bool operator < ( const VectorFastBase& v1, const VectorFastBase& v2 );

// -----------------------------------------------------------------------------

inline
bool operator   > ( const VectorFastBase& v1, const VectorFastBase& v2 ) { return v2 < v1; }
inline
bool operator   >=( const VectorFastBase& v1, const VectorFastBase& v2 ) { return !(v1<v2); }
inline
bool operator   <=( const VectorFastBase& v1, const VectorFastBase& v2 ) { return !(v2<v1); }
