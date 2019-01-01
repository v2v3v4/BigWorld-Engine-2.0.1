/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FOOT_PRINT_RENDERER_HPP
#define FOOT_PRINT_RENDERER_HPP

#include "math/vector3.hpp"

typedef uint32 ChunkSpaceID;

/**
 *	Stores and renders foot prints.
 */
class FootPrintRenderer
{
public:
	FootPrintRenderer(ChunkSpaceID spaceID);
	~FootPrintRenderer();

	void addFootPrint(const Vector3 * vertices);
	void draw();

	static void init();
	static void fini();
	
	static void setFootPrintOption(int selectedOption);
	
private:
	int spaceID_;
};

#endif // FOOT_PRINT_RENDERER_HPP
