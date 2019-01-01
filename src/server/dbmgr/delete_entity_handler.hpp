/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DELETE_ENTITY_HANDLER_HPP
#define DELETE_ENTITY_HANDLER_HPP

#include "get_entity_handler.hpp"

#include "network/bundle.hpp"


/**
 *	This class processes a request to delete an entity from the database.
 */
class DeleteEntityHandler : public GetEntityHandler,
							public IDatabase::IDelEntityHandler
{
public:
	DeleteEntityHandler( EntityTypeID typeID, DatabaseID dbID,
		const Mercury::Address& srcAddr, Mercury::ReplyID replyID );
	DeleteEntityHandler( EntityTypeID typeID, const std::string& name,
		const Mercury::Address& srcAddr, Mercury::ReplyID replyID );
	virtual ~DeleteEntityHandler() {}

	void deleteEntity();

	// GetEntityHandler override
	virtual void onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation );

	// IDatabase::IDelEntityHandler override
	virtual void onDelEntityComplete( bool isOK );

private:
	void sendReply();
	void addFailure();

	Mercury::Bundle		replyBundle_;
	Mercury::Address	srcAddr_;
	EntityDBKey			ekey_;
};

#endif // DELETE_ENTITY_HANDLER_HPP
