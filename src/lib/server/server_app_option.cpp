/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server_app_option.hpp"

// -----------------------------------------------------------------------------
// Section: ServerAppOptionIniter
// -----------------------------------------------------------------------------

ServerAppOptionIniter::Container * ServerAppOptionIniter::s_pContainer_;

/**
 *	Constructor.
 *
 *	@param configPath The setting to read from bw.xml
 *	@param watcherPath The path to the watcher value
 *	@param watcherMode Indicates whether the watcher should be read-only.
 */
ServerAppOptionIniter::ServerAppOptionIniter( const char * configPath,
			const char * watcherPath,
			Watcher::Mode watcherMode ) :
		IntrusiveObject< ServerAppOptionIniter >( s_pContainer_ ),
		configPath_( configPath ),
		watcherPath_( watcherPath ),
		watcherMode_( watcherMode )
{
}


/**
 *	This method initialises all of the configuration options. This includes
 *	reading the value from bw.xml and creating watcher entries, if appropriate.
 */
void ServerAppOptionIniter::initAll()
{
	if (s_pContainer_ == NULL)
	{
		return;
	}

	Container::iterator iter = s_pContainer_->begin();

	while (iter != s_pContainer_->end())
	{
		(*iter)->init();

		++iter;
	}
}


/**
 *	This method prints the value of all configuration options.
 */
void ServerAppOptionIniter::printAll()
{
	if (s_pContainer_ == NULL)
	{
		return;
	}

	INFO_MSG( "Configuration options (from server/bw.xml):\n" );

	Container::iterator iter = s_pContainer_->begin();

	while (iter != s_pContainer_->end())
	{
		(*iter)->print();

		++iter;
	}
}


/**
 *	This method cleans up the objects associated with initialising configuration
 *	options.
 */
void ServerAppOptionIniter::deleteAll()
{
	while (s_pContainer_ != NULL)
	{
		delete s_pContainer_->back();
	}
}

// server_app_option.cpp
