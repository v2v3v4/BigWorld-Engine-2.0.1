/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __face_hpp__
#define __face_hpp__

struct Face
{
	Face();
	
	int positionIndex[3];
	int normalIndex[3];
	int uvIndex[3];
	int uvIndex2[3];
	int colourIndex[3];
	int materialIndex;
};

#endif // __face_hpp__