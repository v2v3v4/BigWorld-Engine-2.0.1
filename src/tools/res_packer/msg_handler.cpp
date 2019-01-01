/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#include "cstdmf/concurrency.hpp"
#include "cstdmf/debug.hpp"
#include "msg_handler.hpp"
#include <iostream>


MsgHandler::MsgHandler() :
	errorsOccurred_( false )
{
	DebugFilter::instance().addMessageCallback( this );
	DebugFilter::instance().addCriticalCallback( this );
}

MsgHandler::~MsgHandler()
{
	DebugFilter::instance().deleteCriticalCallback( this );
	DebugFilter::instance().deleteMessageCallback( this );
}

bool MsgHandler::handleMessage( int componentPriority, int messagePriority, const char* format, va_list argPtr )
{
	//Anything that is not an error, warning or notice should be filtered with info
	if ((messagePriority != MESSAGE_PRIORITY_CRITICAL) &&
		(messagePriority != MESSAGE_PRIORITY_ERROR) &&
		(messagePriority != MESSAGE_PRIORITY_WARNING) &&
		(messagePriority != MESSAGE_PRIORITY_NOTICE))
	{
		return true;
	}

	errorsOccurred_ = (messagePriority >= MESSAGE_PRIORITY_ERROR);

	const char * priorityStr = messagePrefix( (DebugMessagePriority)messagePriority );

	class TempString
	{
	public:
		TempString( int size )
		{
			buffer_ = new char[ size + 1 ];
		}
		~TempString()
		{
			delete[] buffer_;
		}
	    char* buffer_;
	private:
		// unimplemented, hidden methods
		TempString( const TempString& other ) {};
		TempString& operator=( const TempString& other ) { return *this; };
	};

#ifdef _WIN32
	int size = _vscprintf( format, argPtr );
#else
	int size = 1024;
#endif

	TempString tempString( size );

#ifdef _WIN32
    size = _vsnprintf( tempString.buffer_, size, format, argPtr );
#else
    size = vsnprintf( tempString.buffer_, size, format, argPtr );
#endif

	if ( tempString.buffer_[ size - 1 ] == '\n' ) // If the message ends with a newline...
		tempString.buffer_[ size - 1 ] = 0;		// remove it
	else
		tempString.buffer_[ size ] = 0;

	std::cout << priorityStr << ": " << tempString.buffer_ << std::endl;

	return true;
}


void MsgHandler::handleCritical( const char * msg )
{
	// exit with failure. The message was already printed in handleMessage
	std::cout << "...failed" << std::endl;
	exit( EXIT_FAILURE );
}


bool MsgHandler::errorsOccurred() const
{
	return errorsOccurred_;
}
