/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCAN_VISION_CONTROLLER_HPP
#define SCAN_VISION_CONTROLLER_HPP

#include "vision_controller.hpp"

/**
 *  This class is the controller for an entity's vision.
 */
class ScanVisionController : public VisionController
{
	DECLARE_CONTROLLER_TYPE( ScanVisionController )

public:
	 // period in ticks. defaults to 10 ticks (1 second)
	ScanVisionController( float visionAngle = 1.f,
						  float visionRange = 20.f,
						  float seeingHeight = 2.f,
						  float amplitude = 0.f,
						  float scanPeriod = 0.f,
						  float timeOffset = 0.f,
						  int updatePeriod = 10 );

	virtual float getYawOffset();

	void	writeRealToStream( BinaryOStream & stream );
	bool 	readRealFromStream( BinaryIStream & stream );

private:
	float 	amplitude_;
	float	scanPeriod_;
	float   timeOffset_;
};

#endif // VISION_CONTROLLER_HPP
