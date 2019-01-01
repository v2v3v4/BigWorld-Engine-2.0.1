/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SETUP_ITEMS_TASK_HPP
#define SETUP_ITEMS_TASK_HPP


#include "world/item_info_db.hpp"
#include "list_group_states.hpp"


/**
 *	This class processes list items to set them in groups, filter by search,
 *	gather statistics, etc.
 */
class SetupItemsTask
{
public:
	typedef std::vector< ItemInfoDB::ItemPtr > Items;
	typedef std::set< ItemInfoDB::Type > Types;

	SetupItemsTask( const std::string & search,
		const ListGroupStates & groupStates, const Types & allowedTypes,
		ItemInfoDB::ComparerPtr pComparer );

	virtual ~SetupItemsTask() {}

	virtual void execute();

	virtual void results( Items & retItems, int & retNumItems,
									int & retNumTris, int & retNumPrimitives );

private:
	// input params
	const ListGroupStates & groupStates_;
	Types allowedTypes_;
	ItemInfoDB::ComparerPtr pComparer_;

	// return params
	Items retItems_;
	int retNumItems_;
	int retNumTris_;
	int retNumPrimitives_;

	// Other members
	typedef std::pair< std::string, bool > SearchWord;
	typedef std::vector< SearchWord > SearchWords;
	SearchWords searchWords_;
	bool executed_;

	bool itemInSearch( ItemInfoDB::Item * pItem ) const;

	void splitSearchInWords( const std::string & search );
};


/**
 *	This class helps in processing items in a separate thread.
 */
class SetupItemsTaskThread : public SetupItemsTask
{
public:
	SetupItemsTaskThread( const std::string & search,
		const ListGroupStates & groupStates, const Types & allowedTypes,
		ItemInfoDB::ComparerPtr pComparer );

	~SetupItemsTaskThread();
	
	void stop();

	bool finished() { return finished_; }

	void execute();

	void results( Items & retItems, int & retNumItems,
									int & retNumTris, int & retNumPrimitives );

private:
	SimpleThread * pThread_;

	bool finished_;

	static void s_startThread( void * threadData );

	void executeInternal();
};



#endif // SETUP_ITEMS_TASK_HPP
