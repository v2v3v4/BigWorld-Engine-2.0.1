/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MARKER_GIRTH_INFO_HPP
#define MARKER_GIRTH_INFO_HPP

struct MarkerGirthInfo
{
	MarkerGirthInfo()
		: girthValue( -1.0 ), generateRange( -1.0 )
	{}

	MarkerGirthInfo( float girthValue_, float generateRange_ )
		: girthValue( girthValue_), generateRange( generateRange_ )
	{}

	float girthValue;
	float generateRange;
};

#endif
