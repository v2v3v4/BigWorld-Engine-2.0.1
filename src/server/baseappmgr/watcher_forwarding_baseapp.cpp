/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "watcher_forwarding_baseapp.hpp"
#include "cstdmf/watcher_path_request.hpp"
#include "server/watcher_forwarding_collector.hpp"

#include "baseappmgr.hpp"
#include "baseapp.hpp"
#include "baseapp/baseapp_int_interface.hpp"


// Overridden from ForwardingCollector
ForwardingCollector *BAForwardingWatcher::newCollector(
	WatcherPathRequestV2 & pathRequest,
	const std::string & destWatcher,
	const std::string & targetInfo )
{
	ForwardingCollector *collector =
		new ForwardingCollector( pathRequest, destWatcher );
	if (collector == NULL)
	{
		ERROR_MSG( "newCollector: Failed to create new watcher "
					"ForwardingCollector.\n" );
		return NULL;
	}

	AddressList *targetList = this->getComponentAddressList( targetInfo );

	if (!collector->init( BaseAppIntInterface::callWatcher,
		BaseAppMgr::instance().interface(), targetList ))
	{
		ERROR_MSG( "BAForwardingWatcher::newCollector: Failed to initialise "
					"ForwardingCollector.\n" );
		delete collector;
		collector = NULL;

		if (targetList)
			delete targetList;
	}

	return collector;
}


/**
 * Generate a list of BaseApp(s) (Component ID, Address pair) as determined by
 * the target information for the forwarding watchers.
 * 
 * @param targetInfo A string describing which component(s) to add to the list.
 *
 * @returns A list of AddressPair's to forward a watcher request to.
 */
AddressList * BAForwardingWatcher::getComponentAddressList(
	const std::string & targetInfo )
{
	AddressList * list = new AddressList;
	if (list == NULL)
	{
		ERROR_MSG( "BAForwardingWatcher::getComponentList: Failed to "
					"generate component list.\n" );
		return list;
	}

	BaseAppMgr::BaseApps & baseApps = BaseAppMgr::instance().baseApps();

	if (targetInfo == TARGET_LEAST_LOADED)
	{
		BaseApp *baseApp = BaseAppMgr::instance().findBestBaseApp();
		if (baseApp)
		{
			list->push_back(
					AddressPair( baseApp->addr(), baseApp->id() ) );
		}

	}
	else if (targetInfo == TARGET_ALL)
	{
		BaseAppMgr::BaseApps::iterator iter = baseApps.begin();
		while (iter != baseApps.end())
		{
			list->push_back(
				AddressPair( (*iter).second->addr(), (*iter).second->id() ) );
			iter++;
		}

	}
	else
	{
		ERROR_MSG( "BaseAppMgr ForwardingWatcher::getComponentAddressList: "
					"Invalid forwarding destination '%s'.\n",
					targetInfo.c_str() );
		if (list)
			delete list;
		list = NULL;
	}
//	else
//	{
//		// TODO: possible optimsiation here 
//		ComponentIDList targetList = this->getComponentIDList( targetInfo );
//		BaseAppMgr::BaseApps::iterator baseAppIter = baseApps.begin();
//		while (baseAppIter != baseApps.end())
//		{
//			BaseApp *baseApp = (*baseAppIter).second;
//			ComponentIDList::iterator targetIter = targetList.begin();
//			while (targetIter != targetList.end())
//			{
//				// The baseapp matches a baseapp in our list
//				if (*targetIter == baseApp->id())
//				{
//					list->push_back(
//						AddressPair( baseApp->addr(), baseApp->id() ) );
//				}
//
//				targetIter++;
//			}
//
//			baseAppIter++;
//		}
//	}

	return list;
}
