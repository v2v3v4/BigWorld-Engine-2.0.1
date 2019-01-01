/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MAIN_LOOP_TASK_HPP
#define MAIN_LOOP_TASK_HPP

#include <vector>
#include "stringmap.hpp"


/**
 *	This class is an interface that can be implemented by tasks that want
 *	to run in the main loop. It is designed for large tasks, and so it is
 *	required that each task have a unique name. Tasks can be ordered by
 *	using groups (the task name is like a path) and by naming other tasks
 *	that must occur before or after the task considered.
 */
class MainLoopTask
{
public:
	MainLoopTask() : enableDraw(true) {	}
	virtual ~MainLoopTask() { }

	virtual bool init() { return true; }
	virtual void fini()	{ }

	virtual void tick( float dTime ) { }
	virtual void draw() { }
	virtual void inactiveTick( float dTime ) {}

	bool	enableDraw;
};


/**
 *	This class is a group of tasks with a similar function.
 *	It is not a singleton, but there is one global root group.
 */
class MainLoopTasks : public MainLoopTask
{
public:
	MainLoopTasks();
	virtual ~MainLoopTasks();

	virtual bool init();
	virtual void fini();

	virtual void tick( float dTime );
	virtual void draw();

	// this gets called rather than tick when the application is minimised
	virtual void inactiveTick( float dTime );

	// rules are strings like ">TaskA", "<TaskB"
	void add( MainLoopTask * pTask, const char * name, ... ); // rules, NULL@end
	void del( MainLoopTask * pTask, const char * name );

	void outputOrder();

	MainLoopTask * getMainLoopTask( const char * name );

	static MainLoopTasks & root();

	bool initted()							{ return initted_; }

	static void finiAll();
private:
	MainLoopTasks( const MainLoopTasks& );
	MainLoopTasks& operator=( const MainLoopTasks& );

	void sort();

	typedef StringMap<MainLoopTask*>	TaskMap;
	typedef std::vector<int>			OrderList;
	typedef std::vector<const char*>	RulesList;

	TaskMap			tasks_;
	OrderList		order_;
	RulesList		rules_;

	bool			initted_;
	bool			finished_;

	static std::vector<MainLoopTask*> *s_orphans_;
};

#endif // MAIN_LOOP_TASK_HPP
