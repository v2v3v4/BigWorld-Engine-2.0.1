/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WATCHER_FORWARDING_HPP
#define WATCHER_FORWARDING_HPP

#include "cstdmf/watcher.hpp"

#include "watcher_forwarding_collector.hpp"
#include "watcher_forwarding_types.hpp"


class WatcherPathRequestV2;

/**
 * This class implements a special Watcher for component managers. Watcher
 * requests sent to the watcher path associated with this watcher can be
 * sent via Mercury to components being mangaged by the manager.
 */
class ForwardingWatcher : public Watcher
{
public:

	/**
	 * Indicators for watcher paths that are sent via the forwarding
	 * watcher as to which components being managed that should receive
	 * the watcher request.
	 */
	enum ExposeHints
	{
		WITH_ENTITY = 0, //!< The component owns a specific entity.
		ALL, //!< All components owned by the manager.
		WITH_SPACE, //!< All components with a specific space.
		LEAST_LOADED //!< The least loaded component.
	};

	/**
	 * Create a new ForwardingCollector, initialised with all neccessary
	 * information, and store it in our list of collectors.
	 *
	 * @param pathRequest The WatcherPathRequest the collector should report
	 *                    back to.
	 * @param destWatcher The watcher path to be used on the receiving
	 *                    component.
	 * @param targetInfo  The target information extracted from the watcher
	 *                    path to be used in determining which components
	 *                    should be forwarded to.
	 *
	 * @returns A pointer to a new ForwardingCollector on success,
	 *          NULL on error.
	 */
	virtual ForwardingCollector *newCollector(
		WatcherPathRequestV2 & pathRequest,
		const std::string & destWatcher,
		const std::string & targetInfo ) = 0;


	/*
	 * Overridden from Watcher
	 */

	/**
	 * The Forwarding Watcher doesn't support the version 1 protocol.
	 *
	 * @returns Always returns false.
	 */
	virtual bool getAsString( const void * base, const char * path,
		std::string & result, std::string & desc, Mode & mode ) const
	{
		return false;
	}

	/**
	 * The Forwarding Watcher doesn't support the version 1 protocol.
	 *
	 * @returns Always returns false.
	 */
	virtual bool setFromString( void * base, const char * path,
		const char * valueStr )
	{
		return false;
	}


	virtual bool setFromStream( void * base, const char * path,
		WatcherPathRequestV2 & pathRequest );

	virtual bool getAsStream( const void * base, const char * path,
		WatcherPathRequestV2 & pathRequest ) const;

protected:
	static const std::string TARGET_LEAST_LOADED;
	static const std::string TARGET_ALL;

	ComponentIDList getComponentIDList( const std::string & targetInfo );
};

#endif
