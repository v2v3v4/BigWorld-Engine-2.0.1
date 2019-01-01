/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONCURRENCY_HPP
#define CONCURRENCY_HPP

#include "debug.hpp"

#if defined( PLAYSTATION3 )
#include <cell/atomic.h>
#endif

void memTrackerThreadFinish();

typedef void (* SimpleThreadFunc)(void*);
/**
 * Helper structure for SimpleThread on both windows and linux.
 */
struct ThreadTrampolineInfo
{
	ThreadTrampolineInfo( SimpleThreadFunc func, void * arg )
	{
		this->func = func;
		this->arg = arg;
	}
	SimpleThreadFunc func;
	void * arg;
};


/**
 *  Dummy mutex class that doesn't actually do anything.
 */
class DummyMutex
{
public:
	void grab() {}
	bool grabTry() { return true; }
	void give() {}
};

#ifdef _WIN32

#ifndef _XBOX360
#include "windows.h"
#else
#include <xtl.h>
#endif

#include <process.h>

inline unsigned long OurThreadID()
{
	return (unsigned long)(GetCurrentThreadId());
}

inline uint getNumCores()
{
	static uint count;

	if ( !count )
	{
		DWORD processAffinity;
		DWORD systemAffinity;
		HRESULT res = GetProcessAffinityMask( GetCurrentProcess(), &processAffinity, &systemAffinity );
		if ( !SUCCEEDED( res ) )
		{
			ERROR_MSG( "Could not get process affinity mask\n" );
			return 1;
		}

		for ( uint i = 0; i < 32; i++ )
		{
			count += ( processAffinity & ( 1 << i ) ) ? 1 : 0;
		}
	}

	return count;
}


/**
 *	This class is a simple mutex
 */
class SimpleMutex
{
public:
	SimpleMutex() : gone_( false ) { InitializeCriticalSection( &mutex_ ); }
	~SimpleMutex()	{ DeleteCriticalSection( &mutex_ ); }

	void grab()		{ EnterCriticalSection( &mutex_ ); MF_ASSERT( !gone_ ); gone_ = true; }
	void give()		{ gone_ = false; LeaveCriticalSection( &mutex_ ); }
	bool grabTry()
	{
		if (TryEnterCriticalSection( &mutex_ ))
		{
			MF_ASSERT( !gone_ );
			gone_ = true;
			return true;
		}
		return false;
	};

private:
	CRITICAL_SECTION	mutex_;
	bool				gone_;
};


/**
 *	This class is a simple semaphore
 */
class SimpleSemaphore
{
public:
	SimpleSemaphore() : sema_( CreateSemaphore( NULL, 0, 32767, NULL ) ) { }
	~SimpleSemaphore()	{ CloseHandle( sema_ ); }

	void push()			{ ReleaseSemaphore( sema_, 1, NULL ); }
	void pull()			{ WaitForSingleObject( sema_, INFINITE ); }
	bool pullTry()		{ return WaitForSingleObject( sema_, 0 ) == WAIT_OBJECT_0; }

private:
	HANDLE sema_;
};


/**
 *  This class is a simple thread
 */
class SimpleThread
{
public:
	SimpleThread()
		: thread_( NULL )
	{}

	SimpleThread( SimpleThreadFunc threadfunc, void * arg )
	{
		init( threadfunc, arg );
	}

	~SimpleThread()
	{
		WaitForSingleObject( thread_, INFINITE );
		CloseHandle( thread_ );
	}

	void init( SimpleThreadFunc threadfunc, void * arg )
	{
		ThreadTrampolineInfo * info = new ThreadTrampolineInfo( threadfunc, arg );
		unsigned threadId;
		thread_ = HANDLE( _beginthreadex( 0, 0, trampoline, info, 0, &threadId ) );
	}

	struct AutoHandle	// Ugly!!
	{
		AutoHandle( HANDLE h ) { HANDLE p = GetCurrentProcess();
			DuplicateHandle( p, h, p, &real_, 0, TRUE, 0 ); }
		~AutoHandle() { CloseHandle( real_ ); }
		operator HANDLE() const { return real_; }
		HANDLE real_;
	};

	HANDLE handle() const	{ return thread_; }	// exposed for more complex ops
	static AutoHandle current()	{ return AutoHandle( GetCurrentThread() ); }

private:
	HANDLE thread_;

	/*
	 * this trampoline function is present so that we can hide the fact that windows
	 * and linux expect different function signatures for thread functions
	 */

	static unsigned __stdcall trampoline( void * arg )
	{
		ThreadTrampolineInfo * info = static_cast<ThreadTrampolineInfo*>(arg);

		CallWithExceptionFilter( info->func, info->arg );
		memTrackerThreadFinish();

		delete info;
		return 0;
	}
};


/**
 *	Swaps dst for newVal if it is still curVal. Returns true if swapped.
 */
#ifndef _XBOX360
inline char atomic_swap( void *& dst, void * curVal, void * newVal )
{
	__asm mov eax, curVal
	__asm mov edx, newVal
	__asm mov ecx, dst
	__asm lock cmpxchg [ecx], edx
	__asm setz al
}
#else
inline char atomic_swap( void *& dst, void * curVal, void * newVal )
{
	InterlockedExchange( (volatile long *)&dst, (long)newVal );
	return 1;
}
#endif


#if BWCLIENT_AS_PYTHON_MODULE

int tlsGetOffSet(int size);
char * tlsGetData();
void tlsIncRef();
void tlsDecRef();

const char   c_initFalse   = 0;
const char   c_initTrue    = 1;

/**
 *	You are not allowed to use __declspec( thread ) when building
 *	a DLL. The TLS API is the way to go if you require thread local
 *	storage. The ThreadLocal template class encapsulates the
 *	underlying TLS implementation and can be used in a way very
 *	similar to __declspec( thread ).
 */
template<typename T>
class ThreadLocal
{
public:
	ThreadLocal();
	ThreadLocal(const T & value);

	~ThreadLocal();

	T & get();
	const T & get() const;

	void set(const T & value);

	operator T & ()
	{
		return get();
	}

	T & operator -> ()
	{
		return get();
	}

	T* operator&()
	{
		return &(get());
	}

	const T & operator = ( const T & other )
	{
		this->set(other);
		return this->get();
	}

	bool operator == ( const T & other )
	{
		return this->get() == other;
	}

	bool operator != ( const T & other )
	{
		return this->get() != other;
	}

	const T & operator ++ ()
	{
		return ++this->get();
	}

	T operator ++ (int)
	{
		return this->get()++;
	}

	const T & operator--()
	{
		return --this->get();
	}

	T operator--(int)
	{
		return this->get()--;
	}

	const T & operator+=( const T & inc )
	{
		return this->get() += inc;
	}

	const T & operator-=( const T & dec )
	{
		return this->get() -= dec;
	}

private:
	int dataOffset_;
	T   initValue_;
};

template<typename T>
ThreadLocal<T>::ThreadLocal()
{
	this->dataOffset_ = tlsGetOffSet(sizeof(T) + 1);
	tlsIncRef();
}

template<typename T>
ThreadLocal<T>::ThreadLocal(const T & value)
{
	char * tlsData    = tlsGetData();
	this->dataOffset_ = tlsGetOffSet(sizeof(T) + 1);
	this->initValue_  = value;
	this->get();
	tlsIncRef();
}

template<typename T>
ThreadLocal<T>::~ThreadLocal()
{
	tlsDecRef();
}

template<typename T>
T & ThreadLocal<T>::get()
{
	char * tlsChar = tlsGetData();
	char * initialised = tlsChar + this->dataOffset_;
	T & value = *(T*)(tlsChar + this->dataOffset_ + 1);
	if (*initialised == c_initFalse)
	{
		value = this->initValue_;
		*initialised = c_initTrue;
	}
	return value;
}

template<typename T>
const T & ThreadLocal<T>::get() const
{
	return const_cast<ThreadLocal<T>*>(this)->get();
}

template<typename T>
void ThreadLocal<T>::set(const T & value)
{
	T & data = this->get();
	data = value;
}

// Using "ThreadLocal<type>" as opposed to "__declspec( thread ) type" occurs
// in approximatly a 20% performance penalty. But because thread local variables
// are rarely used in time critical code this shouldn't be a problem. Anyway,
// "ThreadLocal<type>" is only used when building <bwclient>.dll so this will
// never be a problem in a production release.
#define THREADLOCAL(type) ThreadLocal<type>

#else // BWCLIENT_AS_PYTHON_MODULE

/// thread local declaration - ie "static THREADLOCAL(type) blah"
#if !defined(MF_SINGLE_THREADED)
	#define THREADLOCAL(type) __declspec( thread ) type
#else
	#define THREADLOCAL(type) type
#endif

#endif // BWCLIENT_AS_PYTHON_MODULE


#else // !_WIN32

#ifndef MF_SINGLE_THREADED
#include <pthread.h>
#include <errno.h>

/**
 *	This class is a simple mutex
 */
class SimpleMutex
{
public:
	SimpleMutex()	{ pthread_mutex_init( &mutex_, NULL); }
	~SimpleMutex()	{ pthread_mutex_destroy( &mutex_ ); }

	void grab()		{ pthread_mutex_lock( &mutex_ ); }
	bool grabTry()	{ return (pthread_mutex_trylock( &mutex_ ) == 0); }
	void give()		{ pthread_mutex_unlock( &mutex_ ); }

private:
	pthread_mutex_t mutex_;
};


/**
 *	This class is a simple semaphore, implemented with a condition variable
 */
class SimpleSemaphore
{
public:
	SimpleSemaphore() :
		value_( 0 )
	{
		pthread_mutex_init( &lock_, NULL );
		pthread_cond_init( &wait_, NULL );
	}
	~SimpleSemaphore()
	{
		pthread_cond_destroy( &wait_ );
		pthread_mutex_destroy( &lock_ );
	}

	void pull()
	{
		pthread_mutex_lock( &lock_ );
		value_--;
		if (value_ < 0)
			pthread_cond_wait( &wait_, &lock_ );
		pthread_mutex_unlock( &lock_ );
	}
	bool pullTry()
	{
		bool gotit = true;
		pthread_mutex_lock( &lock_ );
		if (value_ <= 0)
			gotit = false;
		else
			value_--;
		pthread_mutex_unlock( &lock_ );
		return gotit;
	}
	void push()
	{
		pthread_mutex_lock( &lock_ );
		value_++;
		if (value_ <= 0)
			pthread_cond_signal( &wait_ );
		pthread_mutex_unlock( &lock_ );
	}

private:
	pthread_mutex_t	lock_;
	pthread_cond_t	wait_;
	int			value_;
};

inline unsigned long OurThreadID()
{
	return (unsigned long)(pthread_self());
}

/**
 *  This class is a simple thread
 */
class SimpleThread
{
public:
	SimpleThread() : thread_( 0 )
	{
	}

	SimpleThread( SimpleThreadFunc threadfunc, void * arg )
	{
		this->init( threadfunc, arg );
	}

	void init( SimpleThreadFunc threadfunc, void * arg )
	{
		ThreadTrampolineInfo * info = new ThreadTrampolineInfo( threadfunc, arg );
		pthread_create( &thread_, NULL, trampoline, info );
	}
	~SimpleThread()
	{
		pthread_join( thread_, NULL );
	}

	pthread_t handle() const	{ return thread_; }
	static pthread_t current()	{ return pthread_self(); }

private:
	pthread_t thread_;

	/*
	 * this trampoline function is present so that we can hide the fact that windows
	 * and linux expect different function signatures for thread functions
	 */

	static void * trampoline( void * arg )
	{
		ThreadTrampolineInfo * info = static_cast<ThreadTrampolineInfo*>(arg);
		info->func( info->arg );
		delete info;
		return NULL;
	}
};
#else // MF_SINGLE_THREADED

/**
 *	No-op version of SimpleMutex.
 */
class SimpleMutex
{
public:
	SimpleMutex() {}
	~SimpleMutex() {}

	void grab() {}
	void give() {}
	bool grabTry() { return true; }
};

/**
 *	No-op version of SimpleSemaphore.
 */
class SimpleSemaphore
{
public:
	SimpleSemaphore() {}
	~SimpleSemaphore() {}

	void push() {}
	void pull() {}
	bool pullTry() { return true; }
};

inline unsigned long OurThreadID()
{
	return 0;
}

/**
 *	No-op version of SimpleThread.
 */
class SimpleThread
{
public:
	SimpleThread() {}
	SimpleThread( SimpleThreadFunc threadfunc, void * arg )
	{
		this->init( threadfunc, arg );
	}

	void init( SimpleThreadFunc threadfunc, void * arg )
	{
		CRITICAL_MSG( "SimpleThread being initialised when "
			"MF_SINGLE_THREADED!" );
		// threadfunc( arg );
	}


	~SimpleThread(){}

	int handle() const { return 0; }
	static int current() { return 0; }

};
#endif // MF_SINGLE_THREADED

/// thread local declaration - ie "static THREADLOCAL(type) blah"
#ifndef MF_SINGLE_THREADED
# define THREADLOCAL(type) __thread type
#else
# define THREADLOCAL(type) type
#endif

/**
 *	Swaps dst for newVal if it is still curVal. Returns true if swapped.
 */

#if defined( PLAYSTATION3 )

inline bool atomic_swap( void *& dst, void * curVal, void * newVal )
{
	uint32 ret = cellAtomicCompareAndSwap32(
		(uint32_t *)&dst, (uint32_t)curVal, (uint32_t)newVal );
	return ( ret == (uint32_t)curVal );
}

#else

inline bool atomic_swap( void *& dst, void * curVal, void * newVal )
{
	char ret;

	__asm__ volatile (
			"lock cmpxchg %2, %1\n\t"	// (atomically) Compare and Exchange
			"setz %0\n"
		:	"=r"	(ret)		// %0 is ret on output
		:	"m"		(dst),		// %1 is dst on input
			"r"		(newVal),	// %2 is newVal on input
			"a"		(curVal)	// eax is curVal on input
		: "memory" );			// memory is modified
	return ret;
}

#endif

#endif // _WIN32

/**
 *	This class grabs and holds a mutex for as long as it is around
 */
class SimpleMutexHolder
{
public:
	SimpleMutexHolder( SimpleMutex & sm ) : sm_( sm )	{ sm_.grab(); }
	~SimpleMutexHolder()								{ sm_.give(); }
private:
	SimpleMutex & sm_;
};

// Temporary fix to enable compilation under linux for pedantic compilers 
#ifdef _WIN32
 #define ARGTYPE  uint64
#else
 #define ARGTYPE  intptr_t
#endif

/**
 *	This class is a matrix mutex holder
 */
class MatrixMutexHolder
{
	static const int MUTEX_MATRIX_SIZE = 271;
	static SimpleMutex mutex_[MUTEX_MATRIX_SIZE];
	static const unsigned long INVALID_THREAD_ID = 0;
	static volatile unsigned long threadId_[MUTEX_MATRIX_SIZE];
	void grab( ARGTYPE hashInput )
	{
#ifdef _WIN32
		uint64 ui = hashInput;
#else
		uint64 ui = (int)hashInput;
#endif
		index_ = ((ui * ui) >> 3) % MUTEX_MATRIX_SIZE;
		grab_ = threadId_[ index_ ] != OurThreadID();
		if( grab_ )
		{
			mutex_[ index_ ].grab();
			threadId_[ index_ ] = OurThreadID();
		}
	}
	ARGTYPE index_;
	bool grab_;
public:
	MatrixMutexHolder( const void* p )
	{
		grab( (ARGTYPE)p );
	}
	MatrixMutexHolder( unsigned int i )
	{
		grab( i );
	}
	~MatrixMutexHolder()
	{
		if( grab_ )
		{
			threadId_[ index_ ] = INVALID_THREAD_ID;
			mutex_[ index_ ].give();
		}
	}
};

#undef ARGTYPE

extern void (*pMainThreadIdleStartFunc)();
extern void (*pMainThreadIdleEndFunc)();

namespace BWConcurrency
{
inline void startMainThreadIdling()
{
	(*pMainThreadIdleStartFunc)();
}

inline void endMainThreadIdling()
{
	(*pMainThreadIdleEndFunc)();
}

inline
void setMainThreadIdleFunctions( void (*pStartFunc)(), void (*pEndFunc)() )
{
	pMainThreadIdleStartFunc = pStartFunc;
	pMainThreadIdleEndFunc = pEndFunc;
}

}

#endif // CONCURRENCY_HPP
