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

#include "device_callback.hpp"

#ifndef CODE_INLINE
#include "device_callback.ipp"
#endif


#include <algorithm>
#include "cstdmf/concurrency.hpp"
#include "cstdmf/debug.hpp"


namespace Moo
{

DeviceCallback::CallbackList* DeviceCallback::callbacks_ = NULL;

// we assume that there will not be thread contention for the
// creation of the first DeviceCallback and the deletion of the last.
static SimpleMutex * callbacksLock_ = NULL;
static THREADLOCAL( bool ) s_locked;
static DeviceCallback* s_curCallback;

/**
 *	Check to see the callback objects have all been destructed properly.
 */
void DeviceCallback::fini()
{
	BW_GUARD;
	// check leaking
	if (callbacks_)
	{
		callbacksLock_->grab();
		uint count = callbacks_->size();
		WARNING_MSG("%d DeviceCallback object(s) NOT DELETED\n", count);
		CallbackList::iterator it = callbacks_->begin();
		for (; it!=callbacks_->end(); it++)
		{
			WARNING_MSG("DeviceCallback: NOT DELETED : %lx\n", (long)(*it) );
		}
		callbacksLock_->give();
	}
}


/**
 *	Constructor.
 */
DeviceCallback::DeviceCallback()
{
	BW_GUARD;

	if (!callbacks_)	// assumed to be unthreaded
	{
    	callbacks_ = new DeviceCallback::CallbackList;
		callbacksLock_ = new SimpleMutex;
	}

	if ( !s_locked )
	{
		callbacksLock_->grab();
	}
	else
	{
		MF_ASSERT( this != s_curCallback );
	}

	callbacks_->push_back( this );

	if ( !s_locked )
	{
		callbacksLock_->give();
	}
}


/**
 *	Destructor.
 */
DeviceCallback::~DeviceCallback()
{
	BW_GUARD;

	if (callbacks_)
    {
		if ( !s_locked )
		{
			callbacksLock_->grab();
		}
		else
		{
			MF_ASSERT( this != s_curCallback );
		}

        CallbackList::iterator it = std::find( callbacks_->begin(), callbacks_->end(), this );
        if( it != callbacks_->end() )
        {
            callbacks_->erase( it );
        }

	    bool wasEmpty = callbacks_->empty();

		if ( !s_locked )
		{
			callbacksLock_->give();
		}

		if (wasEmpty)	// assumed to be unthreaded
        {
	    	delete callbacks_;
			callbacks_ = NULL;
			delete callbacksLock_;
			callbacksLock_ = NULL;
        }
    }
}

void DeviceCallback::deleteUnmanagedObjects( )
{

}

void DeviceCallback::createUnmanagedObjects( )
{

}

void DeviceCallback::deleteManagedObjects( )
{

}

void DeviceCallback::createManagedObjects( )
{

}

void DeviceCallback::deleteAllUnmanaged( )
{
	BW_GUARD;

	if ( callbacks_ )
    {
		callbacksLock_->grab();
		s_locked = true;

        CallbackList::iterator it = callbacks_->begin();
        CallbackList::iterator end = callbacks_->end();

        while( it != end )
        {
			s_curCallback = *it;
			(*it)->deleteUnmanagedObjects();
            it++;
        }

		s_locked = false;
		callbacksLock_->give();
    }
}

void DeviceCallback::createAllUnmanaged( )
{
	BW_GUARD;

	if ( callbacks_ )
    {
		callbacksLock_->grab();
		s_locked = true;

        CallbackList::iterator it = callbacks_->begin();
        CallbackList::iterator end = callbacks_->end();

        while( it != end )
        {
			s_curCallback = *it;
            (*it)->createUnmanagedObjects();
            it++;
        }

		s_locked = false;
		callbacksLock_->give();
    }
}

void DeviceCallback::deleteAllManaged( )
{
	BW_GUARD;

	if ( callbacks_ )
    {
		callbacksLock_->grab();
		s_locked = true;

        CallbackList::iterator it = callbacks_->begin();
        CallbackList::iterator end = callbacks_->end();

        while( it != end )
        {
			s_curCallback = *it;
			(*it)->deleteManagedObjects();
            it++;
        }

		s_locked = false;
		callbacksLock_->give();
    }
}

void DeviceCallback::createAllManaged( )
{
	BW_GUARD;

	if ( callbacks_ )
    {
		callbacksLock_->grab();
		s_locked = true;

        CallbackList::iterator it = callbacks_->begin();
        CallbackList::iterator end = callbacks_->end();

        while( it != end )
        {
			s_curCallback = *it;
            (*it)->createManagedObjects();
            it++;
        }

		s_locked = false;
		callbacksLock_->give();
    }
}

GenericUnmanagedCallback::GenericUnmanagedCallback( Function* createFunction, Function* destructFunction  )
: createFunction_( createFunction ),
  destructFunction_( destructFunction )
{
}

GenericUnmanagedCallback::~GenericUnmanagedCallback( )
{
}

void GenericUnmanagedCallback::deleteUnmanagedObjects( )
{
	destructFunction_( );
}

void GenericUnmanagedCallback::createUnmanagedObjects( )
{
	createFunction_( );
}

}

// device_callback.cpp
