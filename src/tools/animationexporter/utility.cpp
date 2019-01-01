/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#pragma warning( disable : 4530 )

#include "utility.hpp"

#include "expsets.hpp"

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

Matrix3 rearrangeMatrix( const Matrix3& m )
{

	Matrix3 mm;

	mm.SetColumn( 0, m.GetColumn(0) );
	mm.SetColumn( 1, m.GetColumn(2) );
	mm.SetColumn( 2, m.GetColumn(1) );

	Point3 r2 = mm.GetRow( 2 );
	mm.SetRow( 2, mm.GetRow( 1 ) );
	mm.SetRow( 1, r2 );

	if (ExportSettings::instance().useLegacyOrientation())
	{
		mm.SetRow( 2, -mm.GetRow(2) );
		mm.SetRow( 0, -mm.GetRow(0) );
		mm.SetColumn( 2, -mm.GetColumn(2) );
		mm.SetColumn( 0, -mm.GetColumn(0) );
	}

	return mm;
}

bool isMirrored( const Matrix3 &m )
{
	return ( DotProd( CrossProd( m.GetRow( 0 ), m.GetRow( 1 ) ), m.GetRow( 2 ) ) < 0.0 );
}

Matrix3 normaliseMatrix( const Matrix3 &m )
{
	Matrix3 m2;
	Point3 p = m.GetRow( 0 );
	p = Normalize( p );
	m2.SetRow( 0, p );
	p = m.GetRow( 1 );
	p = Normalize( p );
	m2.SetRow( 1, p );
	p = m.GetRow( 2 );
	p = Normalize( p );
	m2.SetRow( 2, p );
	m2.SetRow( 3, m.GetRow( 3 ) );

	return m2;
}

bool trailingLeadingWhitespaces( const std::string& s )
{
	bool ret = false;
	if (s.length())
		ret = (s[0] == ' ') || (s[s.length()-1] == ' ');
	return ret;
}