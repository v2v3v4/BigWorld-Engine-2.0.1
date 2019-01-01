/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FRAME_LOGGER_HPP
#define FRAME_LOGGER_HPP

#include "cstdmf/main_loop_task.hpp"

/** 
 *	This class records per-frame statistics and writes them to a log file.
 *	It can be activated and customised through the Debug/FrameLogger watchers.
 */
class FrameLogger
{
public:	
	static void init();
};

#endif FRAME_LOGGER_HPP