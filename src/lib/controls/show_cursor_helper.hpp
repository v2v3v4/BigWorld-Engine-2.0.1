/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHOW_CURSOR_HELPER_HPP
#define SHOW_CURSOR_HELPER_HPP


/**
 *	This class changes the visibility of the cursor on constryction and
 *	restores it on destruction.
 */
class ShowCursorHelper
{
public:
	ShowCursorHelper( bool show );
	~ShowCursorHelper();

	// calling this method will prevent this object from restoring the
	// previous cursor state
	void dontRestore();

	// restore's previous cursor state early
	void restoreNow();

	// static helper method to now if the cursor is visible or not
	static bool visible();

private:
	bool show_;
	int showCursorCount_;
};


#endif // SHOW_CURSOR_HELPER_HPP
