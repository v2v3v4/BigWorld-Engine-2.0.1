/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATH_EXTRA_HPP
#define MATH_EXTRA_HPP

#include "cstdmf/locale.hpp"

#include "math_namespace.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "rectt.hpp"
#include <sstream>
#include <iomanip>
#include <float.h>

BEGIN_BW_MATH


inline
bool watcherStringToValue( const char * valueStr, BW::Rect & rect )
{
	char comma;
	std::istringstream istr( valueStr );
	istr.imbue( Locale::standardC() );
	istr >> rect.xMin_ >> comma >> rect.yMin_ >> comma >>
			rect.xMax_ >> comma >> rect.yMax_;
	return !istr.fail() && istr.eof();
}


inline
void streamFloat( std::ostringstream & ostr, float v )
{
	if (v == FLT_MAX)
	{
		ostr << "FLT_MAX";
	}
	else if (v == -FLT_MAX)
	{
		ostr << "-FLT_MAX";
	}
	else
	{
		ostr << v;
	}
}


inline
std::string watcherValueToString( const BW::Rect & rect )
{
	std::ostringstream ostr;
	ostr.imbue( Locale::standardC() );
	ostr << std::setprecision( 3 ) << std::fixed;

	streamFloat( ostr, rect.xMin_ );
	ostr << ", ";
	streamFloat( ostr, rect.yMin_ );
	ostr << ", ";
	streamFloat( ostr, rect.xMax_ );
	ostr << ", ";
	streamFloat( ostr, rect.yMax_ );

	return ostr.str();
}


void orthogonalise(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
				 Vector3 &e1, Vector3 &e2, Vector3 &e3);


uint32 largerPow2( uint32 number );

uint32 smallerPow2( uint32 number );

END_BW_MATH


#endif // MATH_EXTRA_HPP
