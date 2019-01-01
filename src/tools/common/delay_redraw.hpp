/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DELAY_REDRAW
#define DELAY_REDRAW

/**
 * A helper class that allows scoped disabling of updates
 */
class DelayRedraw
{
public:
	DelayRedraw( CWnd* wnd );
	~DelayRedraw();
private:
	CWnd* wnd_;
	static std::map< CWnd*, int > s_counter_;
};

#endif // DELAY_REDRAW