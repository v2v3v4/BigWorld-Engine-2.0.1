/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __material_hpp__
#define __material_hpp__

struct material
{
	std::string name;		// identifier
	std::string mapFile;	// texture or mfm file
	std::string fxFile;		// .fx file for shader
	
	int mapIdMeaning;	// 0 = none, 1 = bitmap, 2 = mfm
};

#endif // __material_hpp__