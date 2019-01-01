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
#include "setup_items_task.hpp"
#include "group_item.hpp"
#include <shlwapi.h>


///////////////////////////////////////////////////////////////////////////////
// Section: SetupItemsTask
///////////////////////////////////////////////////////////////////////////////

/**
 *	Constructor
 *
 *	@param search		Search expresion to filter items with
 *	@param groupStates	Class for managing groups' expanded/collapsed states.
 *	@param pComparer	Comparer class to use, or NULL for default sorting.
 */
SetupItemsTask::SetupItemsTask( const std::string & search,
		const ListGroupStates & groupStates, const Types & allowedTypes,
		ItemInfoDB::ComparerPtr pComparer ) :
	groupStates_( groupStates ),
	allowedTypes_( allowedTypes ),
	pComparer_( pComparer ),
	retNumItems_( 0 ),
	retNumTris_( 0 ),
	retNumPrimitives_( 0 ),
	executed_( false )
{
	BW_GUARD;

	splitSearchInWords( search );
}


/**
 *	This method sets up the items (time-consuming operation).
 */
void SetupItemsTask::execute()
{
	BW_GUARD;

	ItemInfoDB::ItemList items;
	ItemInfoDB::instance().items( items );

	ItemInfoDB::ComparerPtr pComparer = pComparer_;
	if (!pComparer)
	{
		pComparer = ItemInfoDB::Type::builtinType(
						ItemInfoDB::Type::TYPEID_FILEPATH ).comparer( true );
	}
	if (pComparer)
	{
		items.sort( *pComparer );
	}

	executed_ = true;

	retItems_.clear();

	ItemInfoDB::Item * pPrevItem = NULL;
	GroupItem * pLastGroup = NULL;
	retNumItems_ = 0;
	retNumTris_ = 0;
	retNumPrimitives_ = 0;
	int lastGroupNumItems = 0;
	int lastGroupNumTris = 0;
	int lastGroupNumPrims = 0;
	for (ItemInfoDB::ItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
		ItemInfoDB::Item * pItem = (*it).get();
		if (searchWords_.empty() || itemInSearch( pItem ))
		{
			bool addItem = true;
			ItemInfoDB::Type groupByType = groupStates_.groupByType();
			if (groupByType.valueType().isValid())
			{
				std::string group = pItem->propertyAsString( groupByType );
				if (!pPrevItem ||
					pPrevItem->propertyAsString( groupByType ) != group)
				{
					// It's group start, add statistics to the last group...
					if (pLastGroup)
					{
						pLastGroup->numItems( lastGroupNumItems );
						pLastGroup->numTris( lastGroupNumTris );
						pLastGroup->numPrimitives( lastGroupNumPrims );
					}

					// ...and insert a new group.
					lastGroupNumItems = 0;
					lastGroupNumTris = 0;
					lastGroupNumPrims = 0;
					pLastGroup = new GroupItem( group, groupByType );
					retItems_.push_back( pLastGroup );
				}

				// check to see if the group is expanded or not.
				if (!groupStates_.groupExpanded( group ))
				{
					addItem = false;
				}
			}
			if (addItem)
			{
				retItems_.push_back( pItem );
			}
			
			lastGroupNumItems++;
			lastGroupNumTris += pItem->numTris();
			lastGroupNumPrims += pItem->numPrimitives();
			
			pPrevItem = pItem;

			retNumItems_++;
			retNumTris_ += pItem->numTris();
			retNumPrimitives_ += pItem->numPrimitives();
		}
	}

	// It's group start, add statistics to the last group...
	if (pLastGroup)
	{
		pLastGroup->numItems( lastGroupNumItems );
		pLastGroup->numTris( lastGroupNumTris );
		pLastGroup->numPrimitives( lastGroupNumPrims );
	}
}


/**
 *	This method returns the result of setting up the items.
 *
 *	@param retItems		Receives the resulting items.
 *	@param retNumItems	Receives the resulting number of items.
 *	@param retNumTris	Receives the resulting number of triangles.
 *	@param retNumPrimitives	Receives the resulting number of primitives.
 */
void SetupItemsTask::results( Items & retItems, int & retNumItems,
									int & retNumTris, int & retNumPrimitives )
{
	BW_GUARD;

	MF_ASSERT( executed_ );

	retItems = retItems_;
	retNumItems = retNumItems_;
	retNumTris = retNumTris_;
	retNumPrimitives = retNumPrimitives_;
}


/**
 *	This method returns whether or not an item matches the search criteria.
 *
 *	@param pItem	Item to match against the search criteria
 *	@return		Whether or not an item matches the search criteria.
 */
bool SetupItemsTask::itemInSearch( ItemInfoDB::Item * pItem ) const
{
	BW_GUARD;

	bool inSearch = true;
	for (SearchWords::const_iterator it = searchWords_.begin();
		inSearch && it != searchWords_.end(); ++it)
	{
		inSearch = false;
		pItem->startProperties();
		if ((*it).second)
		{
			while (!pItem->isPropertiesEnd())
			{
				const ItemInfoDB::Item::PropertyPair prop =
														pItem->nextProperty();
				const ItemInfoDB::Type & type = prop.first;
				const std::string & propStr = *(prop.second);
				if (!propStr.empty() &&
					(allowedTypes_.empty() ||
						allowedTypes_.find( type ) != allowedTypes_.end()) &&
					PathMatchSpecA( propStr.c_str(), (*it).first.c_str() ))
				{
					inSearch = true;
					break;
				}
			}
		}
		else
		{
			while (!pItem->isPropertiesEnd())
			{
				const ItemInfoDB::Item::PropertyPair prop =
														pItem->nextProperty();
				const ItemInfoDB::Type & type = prop.first;
				const std::string & propStr = *(prop.second);
				if (!propStr.empty() &&
					(allowedTypes_.empty() ||
						allowedTypes_.find( type ) != allowedTypes_.end()))
				{
					std::string propStrLwr = propStr;
					std::transform( propStrLwr.begin(), propStrLwr.end(),
												propStrLwr.begin(), tolower );

					if (propStrLwr.find( (*it).first ) != std::string::npos)
					{
						inSearch = true;
						break;
					}
				}
			}
		}
	}
	return inSearch;
}


/**
 *	This method splits the search string in words, allowing for a search using
 *	multiple keywords.
 *
 *	@param search	Search line as typed by the user.
 */
void SetupItemsTask::splitSearchInWords( const std::string & search )
{
	BW_GUARD;

	searchWords_.clear();
	int spacePos = 0;
	int nextSpacePos = 0;
	while (nextSpacePos != std::string::npos)
	{
		if (spacePos < (int)search.length())
		{
			nextSpacePos = search.find_first_of( ' ', spacePos );
		}
		else
		{
			nextSpacePos = std::string::npos;
		}

		int wordLen = nextSpacePos - spacePos;

		std::string word;
		if (nextSpacePos == std::string::npos)
		{
			word = search.substr( spacePos );
		}
		else if (wordLen > 0)
		{
			word = search.substr( spacePos, wordLen );
		}

		spacePos = nextSpacePos + 1;

		if (!word.empty())
		{
			bool useWildcards = word.find( '*' ) != std::string::npos ||
								word.find( '?' ) != std::string::npos;

			searchWords_.push_back( SearchWord( word, useWildcards ) );
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Section: SetupItemsTaskThread
///////////////////////////////////////////////////////////////////////////////

/**
 *	Constructor
 *
 *	@param search		Search expresion to filter items with
 *	@param groupStates	Class for managing groups' expanded/collapsed states.
 *	@param pComparer	Comparer class to use, or NULL for default sorting.
 */
SetupItemsTaskThread::SetupItemsTaskThread( const std::string & search,
		const ListGroupStates & groupStates, const Types & allowedTypes,
		ItemInfoDB::ComparerPtr pComparer ) :
	SetupItemsTask( search, groupStates, allowedTypes, pComparer ),
	pThread_( NULL ),
	finished_( false )
{
	BW_GUARD;

	pThread_ = new SimpleThread( s_startThread, this );
}


/**
 *	Destructor
 */
SetupItemsTaskThread::~SetupItemsTaskThread()
{
	BW_GUARD;

	stop();
}


/**
 *	This method stops the thread if it's running.
 */
void SetupItemsTaskThread::stop()
{
	BW_GUARD;

	delete pThread_;
	pThread_ = NULL;
}


/**
 *	This method should not be called in this class.
 */
void SetupItemsTaskThread::execute()
{
	BW_GUARD;

	MF_ASSERT( 0 && "This method is not supported in this class" );
}


/**
 *	This method should be called if "finished()" returns true only.
 */
void SetupItemsTaskThread::results( Items & retItems, int & retNumItems,
								int & retNumTris, int & retNumPrimitives )
{
	BW_GUARD;

	MF_ASSERT( finished_ );

	SetupItemsTask::results( retItems, retNumItems, retNumTris,
															retNumPrimitives );
}


/**
 *	This static method is the entry point for the thread, and executes the
 *	task.
 *
 *	@param threadData	Pointer to the thread data, a SetupItemsTaskThread *.
 */
/*static*/
void SetupItemsTaskThread::s_startThread( void * threadData )
{
	BW_GUARD;

	SetupItemsTaskThread * pTask =
						static_cast< SetupItemsTaskThread * >( threadData );

	pTask->executeInternal();

	pTask->finished_ = true;
}


/**
 *	This method calls the base class' execute.
 */
void SetupItemsTaskThread::executeInternal()
{
	BW_GUARD;

	SetupItemsTask::execute();
}
