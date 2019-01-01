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
#include "commentary.hpp"
#include "cstdmf/string_utils.hpp"
#include "resmgr/string_provider.hpp"
#include <algorithm>

#ifndef CODE_INLINE
#include "commentary.ipp"
#endif

#include <windows.h>

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Commentary::Commentary():
	logFile_( NULL )
{
}


/**
 *	Destructor.
 */
Commentary::~Commentary()
{
}


/**
 *	This static method returns the singleton instance of this class.
 */
Commentary & Commentary::instance()
{
	static Commentary instance_;

	return instance_;
}

//Please ensure this matches the entries in LevelId:
const char* LevelName[Commentary::NUM_LEVELS] =
{
	"ACTION",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"HACK",
	"SCRIPT ERROR"
};


/**
 *	This method invokes a commentary message.  All commentary
 *	views are told about the message.
 *
 *	@param	msg the commentary message.
 *	@param	id	an id indicating the type of message.  Can be
 *			one of the family of Commentary::ERROR,
 *			Commentary::CRITICAL etc.
 */
void Commentary::addMsg( const std::string & msg, int id )
{
	std::wstring wmsg;

	if (!msg.empty() && msg[0] == '`')
	{
		bw_utf8tow( LocaliseUTF8( msg.c_str() + 1 ), wmsg );
	}
	else
	{
		bw_utf8tow( msg, wmsg );
	}

	addMsg( wmsg, id );
}


/**
 *	This method invokes a commentary message.  All commentary
 *	views are told about the message.
 *
 *	@param	msg the commentary message.
 *	@param	id	an id indicating the type of message.  Can be
 *			one of the family of Commentary::ERROR,
 *			Commentary::CRITICAL etc.
 */
void Commentary::addMsg( const std::wstring & msg, int id )
{
	Views::iterator it = views_.begin();
	Views::iterator end = views_.end();

	while ( it != end )
	{
		(*it++)->onAddMsg( msg, id );
	}

	if (logFile_)
	{
		*logFile_ << LevelName[id] << ": " << msg.c_str() << std::endl;
	}
}


/**
 *	This method adds a view on the commentary.
 *	Views might have be called listeners, except it depends
 *	on your perspective, and view is a shorter word.
 *
 *	@param view the view to add.
 */
void Commentary::addView( Commentary::View * view )
{
	Views::iterator it = std::find(
			views_.begin(), views_.end(), view );

	if ( it == views_.end() )
	{
		views_.push_back( view );
	}
}


/**
 *	This method removes a view on the commentary.
 *
 *	@param view	the view to remove.
 */
void Commentary::delView( Commentary::View * view )
{
	Views::iterator it = std::find(
			views_.begin(), views_.end(), view );

	if ( it != views_.end() )
	{
		views_.erase( it );
	}
}

extern int ClosedCaptions_token;
int commentaryTokens = ClosedCaptions_token;



// -----------------------------------------------------------------------------
// Section: Streaming
// -----------------------------------------------------------------------------

/**
 *	Output streaming operator for Commentary.
 */
std::ostream& operator<<(std::ostream& o, const Commentary& t)
{
	o << "Commentary\n";
	return o;
}

// commentary.cpp
