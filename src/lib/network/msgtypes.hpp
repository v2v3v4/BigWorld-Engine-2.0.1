/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MSGTYPES_HPP
#define MSGTYPES_HPP

#include <float.h>
#include <math.h>

#include "basictypes.hpp"
#include "math/mathdef.hpp"
#include "cstdmf/binary_stream.hpp"

// -----------------------------------------------------------------------------
// Section: Coordinate
// -----------------------------------------------------------------------------

/**
 * 	This class is used to pack a 3d vector for network transmission.
 *
 * 	@ingroup network
 */
class Coord
{
//	typedef Float<3> Storage;
	typedef float Storage;

	public:
		/// Construct from x,y,z.
		Coord(float x, float y, float z) : x_(x), y_(y), z_(z)
		{}

		/// Construct from a Vector3.
		explicit Coord( const Vector3 & v ) : x_(v.x), y_(v.y), z_(v.z)
		{}

		/// Default constructor.
		Coord()
		{}

		bool operator==( const Coord & oth ) const
		{ return x_ == oth.x_ && y_ == oth.y_ && z_ == oth.z_; }

	Storage x_; 	///< The x component of the coordinate.
	Storage y_;		///< The y component of the coordinate.
	Storage z_;		///< The z component of the coordinate.
};

inline BinaryIStream& operator>>( BinaryIStream &is, Coord &c )
{
	return is >> c.x_ >> c.y_ >> c.z_;
}

inline BinaryOStream& operator<<( BinaryOStream &os, const Coord &c )
{
	return os << c.x_ << c.y_ << c.z_;
}


/**
 *	This method is used to convert an angle in the range [-pi, pi) to an int8.
 *
 *	@see int8ToAngle
 */
inline int8 angleToInt8( float angle )
{
	return (int8)floorf( (angle * 128.f) / MATH_PI + 0.5f );
}


/**
 *	This method is used to convert an compressed angle to an angle in the range
 *	[-pi, pi).
 *
 *	@see angleToInt8
 */
inline float int8ToAngle( int8 angle )
{
	return float(angle) * (MATH_PI / 128.f);
}


/**
 *	This method is used to convert an angle in the range [-pi/2, pi/2) to an
 *	int8.
 *
 *	@see int8ToHalfAngle
 */
inline int8 halfAngleToInt8( float angle )
{
	return (int8)
		Math::clamp( -128.f, floorf( (angle * 254.f) / MATH_PI + 0.5f ),
				127.f );
}


/**
 *	This method is used to convert an compressed angle to an angle in the range
 *	[-pi/2, pi/2).
 *
 *	@see halfAngleToInt8
 */
inline float int8ToHalfAngle( int8 angle )
{
	return float(angle) * (MATH_PI / 254.f);
}


/**
 *	This class is used to pack a yaw and pitch value for network transmission.
 *
 *	@ingroup network
 */
class YawPitch
{
public:
	YawPitch( float yaw, float pitch )
	{
		this->set( yaw, pitch );
	}

	YawPitch() {};

	void set( float yaw, float pitch )
	{
		yaw_   = angleToInt8( yaw );
		pitch_ = angleToInt8( pitch );
	}

	void get( float & yaw, float & pitch ) const
	{
		yaw   = int8ToAngle( yaw_ );
		pitch = int8ToAngle( pitch_ );
	}

	friend BinaryIStream& operator>>( BinaryIStream& is, YawPitch &yp );
	friend BinaryOStream& operator<<( BinaryOStream& os, const YawPitch &yp );

private:
	uint8	yaw_;
	uint8	pitch_;
};

// Streaming operators
inline BinaryIStream& operator>>( BinaryIStream& is, YawPitch &yp )
{
	return is >> yp.yaw_ >> yp.pitch_;
}


inline BinaryOStream& operator<<( BinaryOStream& os, const YawPitch &yp )
{
	return os << yp.yaw_ << yp.pitch_;
}



/**
 *	This class is used to pack a yaw, pitch and roll value for network
 *	transmission.
 *
 *	@ingroup network
 */
class YawPitchRoll
{
public:
	YawPitchRoll( float yaw, float pitch, float roll )
	{
		this->set( yaw, pitch, roll );
	}

	YawPitchRoll() {};

	bool operator==( const YawPitchRoll & oth ) const
	{
		return yaw_ == oth.yaw_ && pitch_ == oth.pitch_ && roll_ == oth.roll_;
	}

	void set( float yaw, float pitch, float roll )
	{
		yaw_   = angleToInt8( yaw );
		pitch_ = halfAngleToInt8( pitch );
		roll_ = angleToInt8( roll );
	}

	void get( float & yaw, float & pitch, float & roll ) const
	{
		yaw   = int8ToAngle( yaw_ );
		pitch = int8ToHalfAngle( pitch_ );
		roll = int8ToAngle( roll_ );
	}

	bool nearTo( const YawPitchRoll & oth ) const
	{
		return uint( (yaw_-oth.yaw_+1) | (pitch_-oth.pitch_+1) |
			(roll_-oth.roll_+1) ) <= 3;
	}

	friend BinaryIStream& operator>>( BinaryIStream& is, YawPitchRoll &ypr );
	friend BinaryOStream& operator<<( BinaryOStream& os,
		const YawPitchRoll &ypr );

private:
	uint8	yaw_;
	uint8	pitch_;
	uint8	roll_;
};

inline BinaryIStream& operator>>( BinaryIStream &is, YawPitchRoll &ypr )
{
	return is >> ypr.yaw_ >> ypr.pitch_ >> ypr.roll_;
}

inline BinaryOStream& operator<<( BinaryOStream &os, const YawPitchRoll &ypr )
{
	return os << ypr.yaw_ << ypr.pitch_ << ypr.roll_;
}

/**
 *	This class is used to store a packed x and z coordinate.
 */
class PackedXZ
{
protected:
	/** @internal */
	union MultiType
	{
		float	asFloat;
		uint	asUint;
		int		asInt;
	};

public:
	PackedXZ() {};
	PackedXZ( float x, float z );

	inline void packXZ( float x, float z );
	inline void unpackXZ( float & x, float & z ) const;

	inline void getXZError( float & xError, float & zError ) const;

	friend BinaryIStream& operator>>( BinaryIStream& is, PackedXZ &xz );
	friend BinaryOStream& operator<<( BinaryOStream& os, const PackedXZ &xz );

private:
	unsigned char data_[3];
};


/**
 *	Constructor.
 */
inline PackedXZ::PackedXZ( float x, float z )
{
	this->packXZ( x, z );
}


/**
 *	This function compresses the two input floats into the low 3 bytes of the
 *	return value. It does this as follows:
 *
 *	Bits 12-23 can be thought of as a 12-bit float storing the x-value.
 *	Bits 0-11 can be thought of as a 12-bit float storing the z-value.
 *	Bits 0-7 are the mantissa, bits 8-10 are the unsigned exponent and bit 11 is
 *	the sign bit.
 *
 *	The input values must be in the range (-510, 510).
 */
inline void PackedXZ::packXZ( float xValue, float zValue )
{
	const float addValues[] = { 2.f, -2.f };
	const uint32 xCeilingValues[] = { 0, 0x7ff000 };
	const uint32 zCeilingValues[] = { 0, 0x0007ff };

	PackedXZ::MultiType x; x.asFloat = xValue;
	PackedXZ::MultiType z; z.asFloat = zValue;

	// We want the value to be in the range (-512, -2], [2, 512). Take 2 from
	// negative numbers and add 2 to positive numbers. This is to make the
	// exponent be in the range [2, 9] which for the exponent of the float in
	// IEEE format is between 10000000 and 10000111.

	x.asFloat += addValues[ x.asInt < 0 ];
	z.asFloat += addValues[ z.asInt < 0 ];

	uint result = 0;

	// Here we check if the input values are out of range. The exponent is meant
	// to be between 10000000 and 10000111. We check that the top 5 bits are
	// 10000.
	// We also need to check for the case that the rounding causes an overflow.
	// This occurs when the bits of the exponent and mantissa we are interested
	// in are all 1 and the next significant bit in the mantissa is 1.
	// If an overflow occurs, we set the result to the maximum result.
	result |= xCeilingValues[((x.asUint & 0x7c000000) != 0x40000000) ||
										((x.asUint & 0x3ffc000) == 0x3ffc000)];
	result |= zCeilingValues[((z.asUint & 0x7c000000) != 0x40000000) ||
										((z.asUint & 0x3ffc000) == 0x3ffc000)];


	// Copy the low 3 bits of the exponent and the high 8 bits of the mantissa.
	// These are the bits 15 - 25. We then add one to this value if the high bit
	// of the remainder of the mantissa is set. It magically works that if the
	// mantissa wraps around, the exponent is incremented by one as required.
	result |= ((x.asUint >>  3) & 0x7ff000) + ((x.asUint & 0x4000) >> 2);
	result |= ((z.asUint >> 15) & 0x0007ff) + ((z.asUint & 0x4000) >> 14);

	// We only need this for values in the range [509.5, 510.0). For these
	// values, the above addition overflows to the sign bit.
	result &= 0x7ff7ff;

	// Copy the sign bit (the high bit) from the values to the result.
	result |=  (x.asUint >>  8) & 0x800000;
	result |=  (z.asUint >> 20) & 0x000800;

	BW_PACK3( (char*)data_, result );
}



/**
 *	This function uncompresses the values that were created by pack.
 */
inline void PackedXZ::unpackXZ( float & xarg, float & zarg ) const
{
	uint data = BW_UNPACK3( (const char*)data_ );

	MultiType & xu = (MultiType&)xarg;
	MultiType & zu = (MultiType&)zarg;

	// The high 5 bits of the exponent are 10000. The low 17 bits of the
	// mantissa are all clear.
	xu.asUint = 0x40000000;
	zu.asUint = 0x40000000;

	// Copy the low 3 bits of the exponent and the high 8 bits of the mantissa
	// into the results.
	xu.asUint |= (data & 0x7ff000) << 3;
	zu.asUint |= (data & 0x0007ff) << 15;

	// The produced floats are in the range (-512, -2], [2, 512). Change this
	// back to the range (-510, 510). [Note: the sign bit is not on yet.]
	xu.asFloat -= 2.0f;
	zu.asFloat -= 2.0f;

	// Copy the sign bit across.
	xu.asUint |= (data & 0x800000) << 8;
	zu.asUint |= (data & 0x000800) << 20;
}


/**
 *	This method returns the maximum error in the x and z components caused by
 *	compression.
 */
inline void PackedXZ::getXZError( float & xError, float & zError ) const
{
	uint32 data = BW_UNPACK3( (const char*)data_ );
	//
	// The exponents are the 3 bits from bit 8.
	int xExp = (data >> 20) & 0x7;
	int zExp = (data >>  8) & 0x7;

	// The first range is 2 metres and the mantissa is 8 bits. This makes each
	// step 2/(2**8). The x and z compression rounds and so the error can be
	// up to half step size. The error doubles for each additional range.
	xError = (1.f/256.f) * (1 << xExp);
	zError = (1.f/256.f) * (1 << zExp);
}

// Streaming operators
inline BinaryIStream& operator>>( BinaryIStream& is, PackedXZ &xz )
{
	MF_ASSERT( sizeof( xz.data_ ) == 3 );
	const void *src = is.retrieve( sizeof( xz.data_ ) );
	BW_NTOH3_ASSIGN( reinterpret_cast<char *>( xz.data_ ),
		reinterpret_cast<const char*>( src ) );
	return is;
}


inline BinaryOStream& operator<<( BinaryOStream& os, const PackedXZ &xz )
{
	MF_ASSERT( sizeof( xz.data_ ) == 3 );
	void * dest = os.reserve( sizeof( xz.data_ ) );
	BW_HTON3_ASSIGN( reinterpret_cast< char * >( dest ),
		reinterpret_cast< const char* >( xz.data_ ) );
	return os;
}


// -----------------------------------------------------------------------------
// Section: class PackedXHZ
// -----------------------------------------------------------------------------

/**
 *  This is just a placeholder at the moment. The idea is that we send down
 *  a message that identifies the chunk that the entity is in and the approx.
 *  height within that chunk so that the client can find the correct level.
 */
class PackedXHZ : public PackedXZ
{
public:
	PackedXHZ() : height_( 0 ) {};
private:
	int8 height_;
};


// -----------------------------------------------------------------------------
// Section: class PackedXYZ
// -----------------------------------------------------------------------------

/**
 *	This class is used to store a packed x, y, z coordinate. At the moment, it
 *	stores the y value as a 16 bit float with a 4 bit exponent and 11 bit
 *	mantissa. This is a bit of overkill. It can handle values in the range
 *	(-131070.0, 131070.0)
 */
class PackedXYZ : public PackedXZ
{
public:
	PackedXYZ();
	PackedXYZ( float x, float y, float z );

	inline void packXYZ( float x, float y, float z );
	inline void unpackXYZ( float & x, float & y, float & z ) const;
	inline void getXYZError( float & xErr, float & yErr, float & zErr ) const;

	inline void y( float value );
	inline float y() const;
	inline float yError() const;

	friend BinaryIStream& operator>>( BinaryIStream& is, PackedXYZ &xyz );
	friend BinaryOStream& operator<<( BinaryOStream& is, const PackedXYZ &xyz );

private:
	unsigned char yData_[2];
};


/**
 *	Constructor.
 */
inline PackedXYZ::PackedXYZ()
{
}


/**
 *	Constructor.
 */
inline PackedXYZ::PackedXYZ( float x, float y, float z )
{
	this->packXZ( x, z );
	this->y( y );
}


/**
 *	This method packs the three input floats into this object.
 */
inline void PackedXYZ::packXYZ( float x, float y, float z )
{
	this->y( y );
	this->packXZ( x, z );
}


/**
 *	This method unpacks this object into the three input floats.
 */
inline void PackedXYZ::unpackXYZ( float & x, float & y, float & z ) const
{
	y = this->y();
	this->unpackXZ( x, z );
}


/**
 *	This method returns the maximum error in the x, y and z components caused by
 *	compression.
 */
inline void PackedXYZ::getXYZError(
		float & xErr, float & yErr, float & zErr ) const
{
	this->getXZError( xErr, zErr );
	yErr = this->yError();
}


/**
 *	This method sets the packed y value from a float.
 */
inline void PackedXYZ::y( float yValue )
{
	// TODO: no range checking is done on the input value or rounding of the
	// result.

	// Add bias to the value to force the floating point exponent to be in the
	// range [2, 15], which translates to an IEEE754 exponent in the range
	// 10000000 to 1000FFFF (which contains a +127 bias).
	// Thus only the least significant 4 bits of the exponent need to be
	// stored.

	const float addValues[] = { 2.f, -2.f };
	MultiType y; y.asFloat = yValue;
	y.asFloat += addValues[ y.asInt < 0 ];

	uint16& yDataInt = *(uint16*)yData_;

	// Extract the lower 4 bits of the exponent, and the 11 most significant
	// bits of the mantissa (15 bits all up).
	yDataInt = (y.asUint >> 12) & 0x7fff;

	// Transfer the sign bit.
 	yDataInt |= ((y.asUint >> 16) & 0x8000);

}


/**
 *	This method returns the packed y value as a float.
 */
inline float PackedXYZ::y() const
{
	// Preload output value with 0(sign) 10000000(exponent) 00..(mantissa).
	MultiType y;
	y.asUint = 0x40000000;

	uint16& yDataInt = *(uint16*)yData_;

	// Copy the 4-bit lower 4 bits of the exponent and the
	// 11 most significant bits of the mantissa.
	y.asUint |= (yDataInt & 0x7fff) << 12;

	// Remove value bias.
	y.asFloat -= 2.f;

	// Copy the sign bit.
	y.asUint |= (yDataInt & 0x8000) << 16;

	return y.asFloat;
}


/**
 *	This method returns the maximum error for the y value caused by compression.
 */
inline float PackedXYZ::yError() const
{
	// TODO: Is this endian safe?
	uint16& yDataInt = *(uint16*)yData_;

	// The exponent is the 4 bits from bit 11.
	int yExp = (yDataInt >> 11) & 0xf;

	// The first range is 2 metres and the mantissa is 11 bits. This makes each
	// step 2/(2**11). The y compression only rounds and so the maximum error
	// is the step size. This doubles for each additional range.
	return (2.f/2048.f) * (1 << yExp);
}


// Streaming operators
inline BinaryIStream& operator>>( BinaryIStream& is, PackedXYZ &xyz )
{
	MF_ASSERT( sizeof( xyz.yData_ ) == sizeof( uint16 ) );
	return is >> (PackedXZ&)xyz >> *(uint16*)xyz.yData_;
}


inline BinaryOStream& operator<<( BinaryOStream& os, const PackedXYZ &xyz )
{
	MF_ASSERT( sizeof( xyz.yData_ ) == sizeof( uint16 ) );
	return os << (PackedXZ&)xyz << *(uint16*)xyz.yData_;
}


typedef uint8 IDAlias;


// -----------------------------------------------------------------------------
// Section: ReferencePosition
// -----------------------------------------------------------------------------


/**
 *	This method returns the reference position associated with the input
 *	position.
 *
 *	The reason that it is important to round the reference position is that if
 *	the reference position changes by less than the least accurate offset from
 *	this position, entities that are meant to be stationary will move as the
 *	reference position moves.
 */
inline Vector3 calculateReferencePosition( const Vector3 & pos )
{
	return Vector3( BW_ROUNDF( pos.x ), BW_ROUNDF( pos.y ), BW_ROUNDF( pos.z ) );
}

#endif // MSGTYPES_HPP
