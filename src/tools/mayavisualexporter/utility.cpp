/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#pragma warning( disable : 4530 )

#include "utility.hpp"

std::string toLower( const std::string &s )
{
	std::string newString = s;

	for( uint32 i = 0; i < newString.length(); i++ )
	{
		if( newString[ i ] >= 'A' && newString[ i ] <= 'Z' )
		{
			newString[ i ] = newString[ i ] + 'a' - 'A';
		}
	}
	return newString;
}

std::string trimWhitespaces(const std::string& s)
{
	int f = s.find_first_not_of( ' ' );
	int l = s.find_last_not_of( ' ' );
	return s.substr( f, l - f + 1 );
}

std::string unifySlashes(const std::string& s)
{
	std::string ret = s;
	int n;
	while ((n = ret.find_first_of( "\\" )) > 0)
	{
		ret[n] = '/';
	}
	return ret;
}

matrix4<float> rearrangeMatrix( const matrix4<float>& m )
{
	matrix4<float> mm = m;
	matrix4<float> mn = mm;
	
	return mn;
}

bool isMirrored( const matrix3<float> &m )
{
	vector3<float> row0( m.v11, m.v12, m.v13 );
	vector3<float> row1( m.v21, m.v22, m.v23 );
	vector3<float> row2( m.v31, m.v32, m.v33 );
	
	return row0.cross( row1 ).dot( row2 ) < 0;
}

matrix4<float> normaliseMatrix( const matrix4<float> &m )
{
	vector3<float> row0( m.v11, m.v12, m.v13 );
	vector3<float> row1( m.v21, m.v22, m.v23 );
	vector3<float> row2( m.v31, m.v32, m.v33 );

	row0.normalise();
	row1.normalise();
	row2.normalise();
	
	matrix4<float> m2( row0.x, row0.y, row0.z, m.v14, row1.x, row1.y, row1.z, m.v24, row2.x, row2.y, row2.z, m.v23, m.v41, m.v42, m.v43, m.v44 );

	return m2;
}

bool trailingLeadingWhitespaces( const std::string& s )
{
	bool ret = false;
	if (s.length())
		ret = (s[0] == ' ') || (s[s.length()-1] == ' ');
	return ret;
}

float snapValue( float v, float snapSize )
{
	if ( snapSize > 0.f )
	{
		float halfSnap = snapSize / 2.f;

		if (v > 0.f)
		{
			v += halfSnap;

			v -= ( fmodf( v, snapSize ) );
		}
		else
		{
			v -= halfSnap;

			v += ( fmodf( -v, snapSize ) ); 
		}
	}

	return v;
}

Point3 snapPoint3( const Point3& pt, float snapSize )
{
	Point3 p = pt;

	p.x = snapValue( p.x, snapSize );
	p.y = snapValue( p.y, snapSize );
	p.z = snapValue( p.z, snapSize );

	return p;
}
