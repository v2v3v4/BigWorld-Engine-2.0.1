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

#include "time_queue.hpp"

/**
 *	This method cancels the timer associated with this handle. It is safe to
 *	call when the TimerHandle has not been set.
 */
void TimerHandle::cancel()
{
	if (pNode_ != NULL)
	{
		TimeQueueNode* pNode = pNode_;

		pNode_ = NULL;
		pNode->cancel();
	}
}

// time_queue.cpp
