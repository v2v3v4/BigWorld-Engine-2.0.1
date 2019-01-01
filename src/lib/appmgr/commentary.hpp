/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COMMENTARY_HPP
#define COMMENTARY_HPP

#include <iostream>
#include <vector>

/**
 *	This class provides the global interface for commentary.
 *	The commentary class is also a useful hub for runtime
 *	debugging information.
 */
class Commentary
{
public:
	~Commentary();

	//Accessors
	static Commentary & instance();

	void logFile( std::ostream* logFile ) { logFile_ = logFile; }
	std::ostream* logFile() { return logFile_; }

	enum LevelId
	{
		COMMENT = 0,
		CRITICAL = 1,
		ERROR_LEVEL = 2,
		WARNING = 3,
		HACK = 4,
		SCRIPT_ERROR = 5,
		NUM_LEVELS
	};

	void	addMsg( const std::string & msg, int id = COMMENT );
	void	addMsg( const std::wstring & msg, int id = COMMENT );

	//callbacks
	class View
	{
	public:
		virtual void onAddMsg( const std::wstring & msg, int id ) = 0;
	};

	void	addView( Commentary::View * view );
	void	delView( Commentary::View * view );

private:
	Commentary();

	Commentary( const Commentary& );
	Commentary& operator=( const Commentary& );

	typedef std::vector< View* > Views;
	Views	views_;

	std::ostream* logFile_;

	friend std::ostream& operator<<( std::ostream&, const Commentary& );
};

#ifdef CODE_INLINE
#include "commentary.ipp"
#endif

#endif // COMMENTARY_HPP
