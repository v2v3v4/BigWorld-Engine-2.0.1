/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __PYSCRIPT_PY_CALLBACK_HPP__
#define __PYSCRIPT_PY_CALLBACK_HPP__

#ifndef MF_SERVER
// Not available on server yet, pending refactoring.

namespace Script
{
	// Timer handles
	typedef uint32 TimerHandle;
	const TimerHandle INVALID_TIMER_HANDLE = 0;
	const TimerHandle MAX_TIMER_HANDLE = 0xffffffff;
	TimerHandle getHandle();

	// Game time
	typedef double (*TotalGameTimeFn)();
	void	setTotalGameTimeFn( TotalGameTimeFn fn );
	double	getTotalGameTime();

	// Callbacks
	void clearTimers();
	void tick( double timeNow );
	void callNextFrame( PyObject * fn, PyObject * args, const char * reason,
		double age = 0.0 );
}

#endif

#endif // __PYSCRIPT_PY_CALLBACK_HPP__
