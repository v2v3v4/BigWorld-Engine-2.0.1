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

#include "cstdmf/config.hpp"
#include "doc_watcher.hpp"

#if ENABLE_WATCHERS

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT( 0 )

namespace WatcherDoc
{
/**
 *	This is a helper function that initialise watcher documentation from
 *	reading the watchdoc/{cellapp,baseapp... etc}.xml
 *
 *	@param appName			The name of the application
 *
 */

	DataSectionPtr	s_pDocSection = NULL;

	void initWatcherDoc( const std::string & appName )
	{
		std::string watchdoc = "watcherdoc/" + appName + ".xml";
		s_pDocSection = BWResource::instance().openSection( watchdoc );
		// TODO: Add this back when we start using it.
		/*
		if (!s_pDocSection)
		{
			WARNING_MSG( "WatcherDoc::initWatcherDoc: unable to load %s\n",
				watchdoc.c_str() );
		}
		*/
	}

	void setWatcherDoc( WatcherPtr pWatcher, const std::string & path )
	{
		if (s_pDocSection && pWatcher)
		{
			std::string comment = s_pDocSection->readString( path );
			if (!comment.empty())
				pWatcher->setComment( comment );
		}
	}
}


#endif // ENABLE_WATCHERS
