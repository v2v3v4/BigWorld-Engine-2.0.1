/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _BWTRACER_HEADER
#define _BWTRACER_HEADER

class BWTracer;

namespace Mercury
{
class NetworkInterface;
}


/**
 *	This simple class is used to initialise and manage the lifespan of the
 *	bwtracer.
 */
class BWTracerHolder
{
public:
	BWTracerHolder();
	~BWTracerHolder();

	void init( Mercury::NetworkInterface & networkInterface );

private:
	BWTracer * pImpl_;
};


#endif
