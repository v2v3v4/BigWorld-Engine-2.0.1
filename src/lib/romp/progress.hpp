/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROGRESS_HPP
#define PROGRESS_HPP

#include <vector>

#include "font.hpp"

#include "cstdmf/stdmf.hpp"


/**
 *	This class keeps track of the progress of a single task
 */
class ProgressTask
{
public:
	ProgressTask( class ProgressDisplay * pOwner,
		const std::string & name,
		float length = 0 );				// length <=0 means indeterminate time

	virtual ~ProgressTask();


	bool step( float progress = 1 );	// add this amount to done

	bool set( float done = 0 );			// set to this much done

	void length( float length )		{ length_ = length; }

private:
	class ProgressDisplay	 * pOwner_;	// the ProgressDisplay we belong to

	float			done_;				// work done so far
	float			length_;			// total work to do

	void			detach()		{ pOwner_ = NULL; }

	// for indeterminate time progress tasks, length_ is always done_ + 1.f

	friend class ProgressDisplay;
	friend class SuperModelProgressDisplay;
	friend class GUIProgressDisplay;
};


/**
 *	This class manages and displays a group of ProgressTasks.
 */
class ProgressDisplay
{
public:
	typedef bool (*ProgressCallback)();

	ProgressDisplay( FontPtr pFont = NULL,
		ProgressCallback pCallback = NULL,
		uint32 colour = 0xFF26D1C7 );

	virtual ~ProgressDisplay();

	virtual void		add( const std::string & str );
	virtual void		append( const std::string & str );


	// do what we were designed to - display ourselves
	virtual bool		draw( bool force = false );

protected:

	FontPtr				pFont_;
	ProgressCallback	pCallback_;
	uint32				colour_;

	void				drawBar( int row, float fraction );

	// called by progress tasks
	virtual void		add( ProgressTask & task, const std::string & name );
	virtual void		del( ProgressTask & task );

	/**
	 * TODO: to be documented.
	 */
	struct ProgressNode
	{
		ProgressTask*		task;
		std::string			name;
		std::vector<int>	children;
		int					level;
	};

	std::vector<ProgressNode> tasks_;

	int					deepestNode_;
	std::vector<int>	roots_;

	uint64				lastDrawn_;
	uint64				minRedrawTime_;

	float				rowHeight_;
	float				rowSep_;

	std::vector<std::string>	messages_;

	friend class ProgressTask;
};


#endif
/*progress.hpp*/
