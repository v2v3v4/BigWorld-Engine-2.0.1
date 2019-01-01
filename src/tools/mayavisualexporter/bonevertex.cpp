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

#include "bonevertex.hpp"

BoneVertex::BoneVertex( const Point3& position, int index1, int index2, int index3, float weight1, float weight2, float weight3 ) :
	position( position ), index1( index1 ), index2( index2 ), index3( index3 ), weight1( weight1 ), weight2( weight2 ), weight3( weight3 )
{
}
