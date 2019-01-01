/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_MATH_HPP
#define MOO_MATH_HPP

#include <cstdmf/stdmf.hpp>
#include <cstdmf/debug.hpp>
#include <vector>
#include <list>

#ifndef MF_SERVER
#include "d3dx9math.h"
#endif

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "math/matrix.hpp"
#include "math/quat.hpp"

class Quaternion;
#include "math/angle.hpp"

#ifndef MF_SERVER
namespace Moo
{
	typedef D3DXCOLOR				Colour;
	typedef D3DCOLOR				PackedColour;

	/**
	 * Unpack compressed normal into a three float vector.
	 */
	inline Vector3 unpackNormal( uint32 packed )
	{
		int32 z = int32(packed) >> 22;
		int32 y = int32( packed << 10 ) >> 21;
		int32 x = int32( packed << 21 ) >> 21;

		return Vector3( float( x ) / 1023.f, float( y ) / 1023.f, float( z ) / 511.f );
	}

	/**
	 * Pack three float normal (each component clamped to [-1,1]) into a single
	 * unsigned 32bit word ( 11 bits x, 11 bits y, 10 bits z )
	 */
	inline uint32 packNormal( const Vector3& nn )
	{
		Vector3 n = nn;
		n.normalise();

		n.x = Math::clamp(-1.f, n.x, 1.f);
		n.y = Math::clamp(-1.f, n.y, 1.f);
		n.z = Math::clamp(-1.f, n.z, 1.f);


		return	( ( ( (uint32)(n.z * 511.0f) )  & 0x3ff ) << 22L ) |
				( ( ( (uint32)(n.y * 1023.0f) ) & 0x7ff ) << 11L ) |
				( ( ( (uint32)(n.x * 1023.0f) ) & 0x7ff ) <<  0L );
	}
} // namespace Moo
#endif



#endif
