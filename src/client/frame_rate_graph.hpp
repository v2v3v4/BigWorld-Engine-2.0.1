/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FRAME_RATE_GRAPH_HPP
#define FRAME_RATE_GRAPH_HPP


#include "moo/device_callback.hpp"


/**
 *	This helper class displays a graph of the framerate.
 *	It is drawn even when the engine statistics console is not up.
 */
class FrameRateGraph : public Moo::DeviceCallback
{
public:
	FrameRateGraph();
	~FrameRateGraph();

	void graph( float t );

private:
	void createUnmanagedObjects();

    uint valueIndex_;
	float values_[100];
	Moo::VertexTL verts_[100];
	Moo::Material mat_;
	Moo::VertexTL measuringLines_[6];
	static bool s_display_;
};


#endif // FRAME_RATE_GRAPH_HPP


// frame_rate_graph.hpp
