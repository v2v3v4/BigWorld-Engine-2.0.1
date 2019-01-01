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
#include "processor_affinity.hpp"


DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 )

#ifndef _XBOX360

namespace
{
	uint32 s_processor = 0;

	// this function returns an available processor number
	// it favours processorHint, but if this one is not
	// available 
	uint32 availableProcessor( uint32 processorHint )
	{
		uint32 ret = processorHint;

		// First get the available processors
		DWORD processAffinity = 0;
		DWORD systemAffinity = 0;
		HRESULT hr = GetProcessAffinityMask( GetCurrentProcess(), 
			&processAffinity, &systemAffinity );

		// If hint is available, use it, otherwise find the first
		// available processor
		if (SUCCEEDED(hr) &&
			!(processAffinity & (1 << processorHint)) )
		{
			for (uint32 i = 0; i < 32; i++)
			{
				if (processAffinity & (1 << i))
				{
					ret = i;
					break;
				}
			}
		}

		return ret;
	}
}

/**
 *  This sets the affinity for the app to the given processors.
 *	@param processor the processor to set the affinity to, please 
 *		note that this is only a hint, if the processor is not 
 *		available the first available processor will be chosen
 */
void ProcessorAffinity::set(uint32 processorHint)
{
	// get an available processor
    s_processor = availableProcessor(processorHint);

    SetThreadAffinityMask(GetCurrentThread(), 1 << s_processor );

	if (s_processor != processorHint)
	{
		INFO_MSG( "ProcessorAffinity::set - unable to set processor "
			"affinity to %d setting to %d in stead\n", processorHint, s_processor );
	}
}


/**
 *  This gets the processor mask that the application is running on.
 */
uint32 ProcessorAffinity::get()
{
    return s_processor;
}


/**
 *  This makes sure that the process is running on the right processor.
 */
void ProcessorAffinity::update()
{
    set(s_processor);
}


#endif
