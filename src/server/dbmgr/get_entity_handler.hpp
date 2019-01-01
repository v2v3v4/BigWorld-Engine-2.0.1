/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_ENTITY_HANDLER_HPP
#define GET_ENTITY_HANDLER_HPP

#include "dbmgr_lib/idatabase.hpp"

/**
 *	This class is meant to replace IDatabase::IGetEntityHandler as the base
 * 	class for IDatabase::getEntity() callbacks. It allows us to muck around
 * 	with the results before passing them on to the actual handler.
 */
class GetEntityHandler : public IDatabase::IGetEntityHandler
{
public:
	// Intercepts the result callback. Derived classes should NOT implement
	// this method.
	virtual void onGetEntityComplete( bool isOK,
				const EntityDBKey & entityKey,
				const EntityMailBoxRef * pBaseEntityLocation );

	// Derived classes should implement this method instead of
	// onGetEntityComplete() - note the extra 'd'.
	virtual void onGetEntityCompleted( bool isOK,
				const EntityDBKey & entityKey,
				const EntityMailBoxRef * pBaseEntityLocation ) = 0;

	static bool isActiveMailBox( const EntityMailBoxRef * pBaseRef );
};

#endif // GET_ENTITY_HANDLER_HPP
