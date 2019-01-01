/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DPRINTF_HPP
#define DPRINTF_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/config.hpp"

#include <stdarg.h>

#include <vector>

/**
 *	Definition for the critical message callback functor
 */
struct CriticalMessageCallback
{
	virtual void handleCritical( const char * msg ) = 0;
	virtual ~CriticalMessageCallback() {};
};

/**
 *	Definition for the message callback functor. If the
 *  function returns true, the default behaviour for
 *  displaying messages is ignored.
 */
struct DebugMessageCallback
{
	virtual bool handleMessage( int componentPriority,
		int messagePriority, const char * format, va_list argPtr ) = 0;
	virtual ~DebugMessageCallback() {};
};

/**
 *	This class is used to help filter debug messages.
 */
class DebugFilter
{
public:
	DebugFilter() :
		filterThreshold_( 0 ),
		hasDevelopmentAssertions_( true )
	{
	}

	/**
	 *	This method returns the singleton instance of this class.
	 */
	static DebugFilter & instance()
	{
		if (s_instance_ == NULL)
		{
			s_instance_ = new DebugFilter();
		}
		return *s_instance_;
	}

	static void fini()
	{
		if (s_instance_)
		{
			delete s_instance_;
			s_instance_ = NULL;
		}
	}

	/**
	 *	This method returns whether or not a message with the input priorities
	 *	should be accepted.
	 */
	static bool shouldAccept( int componentPriority, int messagePriority )
	{
		return (messagePriority >=
			std::max( DebugFilter::instance().filterThreshold(),
				componentPriority ));
	}

	static bool shouldWriteTimePrefix()
		{ return s_shouldWriteTimePrefix_; }

	static void shouldWriteTimePrefix( bool value )
		{ s_shouldWriteTimePrefix_ = value; }

	static bool shouldWriteToConsole()
		{ return s_shouldWriteToConsole_; }
	
	static void shouldWriteToConsole( bool value )
		{ s_shouldWriteToConsole_ = value; }

	/**
	 *	This method	returns the threshold for the filter. Only messages with
	 *	a message priority greater than or equal to this limit will be
	 *	displayed.
	 */
	int filterThreshold() const				{ return filterThreshold_; }

	/**
	 *	This method sets the threshold for the filter.
	 *
	 *	@see filterThreshold
	 */
	void filterThreshold( int value )			{ filterThreshold_ = value; }

	/**
	 *	This method	returns whether or not development time assertions should
	 *	cause a real assertion.
	 */
	bool hasDevelopmentAssertions() const	{ return hasDevelopmentAssertions_; }

	/**
	 *	This method	sets whether or not development time assertions should
	 *	cause a real assertion.
	 */
	void hasDevelopmentAssertions( bool value )	{ hasDevelopmentAssertions_ = value; }

	/**
	 *	This method returns the callbacks associated with a critical message.
	 */
	typedef std::vector<CriticalMessageCallback *> CriticalCallbacks;
	const CriticalCallbacks & getCriticalCallbacks( ) const
		{ return pCriticalCallbacks_; }

	/**
	 *	This method sets a callback associated with a critical message. DebugFilter
	 *  now owns the object pointed to by pCallback.
	 */
	void addCriticalCallback( CriticalMessageCallback * pCallback );
	void deleteCriticalCallback( CriticalMessageCallback *pCallback );

	/**
	 *	This method returns the callbacks associated with debug messages.
	 */
	typedef std::vector<DebugMessageCallback *> DebugCallbacks;
	const DebugCallbacks & getMessageCallbacks( ) const
		{ return pMessageCallbacks_; }

	/**
	 *	This method sets a callback associated with a critical message. If the
	 *	callback is set and returns true, the default behaviour for displaying
	 *	messages is not done. DebugFilter now owns the object pointed to by
	 *  pCallback.
	 */
	void addMessageCallback( DebugMessageCallback * pCallback );
	void deleteMessageCallback( DebugMessageCallback * pCallback );


private:
	int filterThreshold_;
	bool hasDevelopmentAssertions_;

	CriticalCallbacks pCriticalCallbacks_;
	DebugCallbacks pMessageCallbacks_;

	static DebugFilter* s_instance_;

	static bool s_shouldWriteTimePrefix_;
	static bool s_shouldWriteToConsole_;
};




#if ENABLE_DPRINTF

// This function prints a debug message.
#ifndef _WIN32
    void dprintf( const char * format, ... )
        __attribute__ ( (format (printf, 1, 2 ) ) );
    void dprintf( int componentPriority, int messagePriority,
            const char * format, ...)
        __attribute__ ( (format (printf, 3, 4 ) ) );
#else
	void dprintf( const char * format, ... );
	void dprintf( int componentPriority, int messagePriority,
			const char * format, ...);
#endif

	/// This function prints a debug message.
	void vdprintf( const char * format, va_list argPtr,
				const char * prefix = NULL );
	void vdprintf( int componentPriority, int messagePriority,
				const char * format, va_list argPtr, const char * prefix = NULL );
#else
	/// This function prints a debug message.
	inline void dprintf( const char * format, ... ) {}
	inline void dprintf( int componentPriority, int messagePriority,
							const char * format, ...) {}

	/// This function prints a debug message.
	inline void vdprintf( const char * format, va_list argPtr,
							const char * prefix = NULL ) {}
	inline void vdprintf( int componentPriority, int messagePriority,
							const char * format, va_list argPtr, const char * prefix = NULL ) {}
#endif


#endif // DPRINTF_HPP
