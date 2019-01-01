/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FREQUENT_TASKS_HPP
#define FREQUENT_TASKS_HPP

#include <vector>

namespace Mercury
{

/**
 *	Interface for a task to be added to the FrequentTasks collection.
 */
class FrequentTask
{
public:
	virtual ~FrequentTask() {}

	virtual void doTask() = 0;
};



/**
 *	This class is used to maintain the collection of frequent tasks.
 */
class FrequentTasks
{
public:
	FrequentTasks();
	~FrequentTasks();

	void add( FrequentTask * pTask );

	bool cancel( FrequentTask * pTask );

	void process();

private:
	
	typedef std::vector< Mercury::FrequentTask * > Container;
	Container container_;

	bool isDirty_;

	bool * pGotDestroyed_;
};

} // namespace Mercury

#endif // FREQUENT_TASKS_HPP
