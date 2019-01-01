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

#include "math/math_extra.hpp"

uint32 my_largerPow2( uint32 number )
{
	uint32 i = 1;
	uint prevI = 0;
	while (i < number && i > prevI) 
	{
		prevI = i;
		i  *= 2;
	}
	if (i < prevI) 
	{
		return prevI;
	}
	return i;
}

uint32 my_smallerPow2( uint32 number )
{
	return my_largerPow2(number) /2;
}

TEST( POW2 )
{	
	for (uint32 i = 0 ; i < (2 << 15); i ++) 
	{
		CHECK( my_smallerPow2(i) == BW::smallerPow2(i));
		CHECK( my_largerPow2(i) == BW::largerPow2(i));
	}
	for (uint32 i = 0 ; ;i = (uint32)(i * 2/1.5) + 1)
	{
		CHECK( my_smallerPow2(i) == BW::smallerPow2(i));
		CHECK( my_largerPow2(i) == BW::largerPow2(i));
		if (i > powf(2,31))
		{
			break;
		}
	}
}
