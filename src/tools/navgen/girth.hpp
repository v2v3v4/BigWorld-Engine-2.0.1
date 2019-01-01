/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GIRTH_H
#define GIRTH_H

class girth
{

public:
	
	girth()
		: modelWidth_(-1.0f), modelHeight_(-1.0f), modelDepth_(-1.0f)
	{}

	girth( float width, float height, float depth )
		: modelWidth_(width), modelHeight_(height), modelDepth_(depth)
	{}

	//operator float() { return value_; }
	
	float getWidth() { return modelWidth_; }
	float getHeight() { return modelHeight_; }
	float getDepth() { return modelDepth_; }

private:
	
	float modelWidth_;
	float modelHeight_;
	float modelDepth_ ;

};

#endif