/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOOK_UP_ENTITY_HANDLER_HPP
#define LOOK_UP_ENTITY_HANDLER_HPP

#include "get_entity_handler.hpp"
#include "network/bundle.hpp"


/**
 *	This class processes a request to retrieve the base mailbox of a
 *	checked-out entity from the database.
 */
class LookUpEntityHandler : public GetEntityHandler
{
public:
	LookUpEntityHandler( const Mercury::Address& srcAddr,
			Mercury::ReplyID replyID, bool offChannel );
	virtual ~LookUpEntityHandler() {}

	void lookUpEntity( EntityTypeID typeID, const std::string & name )
	{
		EntityDBKey entityKey( typeID, 0, name );
		this->lookUpEntity( entityKey );
	}

	void lookUpEntity( EntityTypeID typeID, DatabaseID databaseID )
	{
		EntityDBKey entityKey( typeID, databaseID );
		this->lookUpEntity( entityKey );
	}

	// IDatabase::IGetEntityHandler/GetEntityHandler overrides
	virtual void onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation );

private:
	void lookUpEntity( const EntityDBKey & entityKey );
	void sendReply();

	Mercury::Bundle		replyBundle_;
	Mercury::Address	srcAddr_;
	bool				offChannel_;
};

#endif // LOOK_UP_ENTITY_HANDLER_HPP
