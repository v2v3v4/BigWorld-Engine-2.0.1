/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOOK_UP_DBID_HANDLER_HPP
#define LOOK_UP_DBID_HANDLER_HPP

#include "get_entity_handler.hpp"
#include "network/bundle.hpp"


/**
 *	This class processes a request to retrieve the DBID of an entity from the
 * 	database.
 */
class LookUpDBIDHandler : public GetEntityHandler
{
public:
	LookUpDBIDHandler( const Mercury::Address& srcAddr,
			Mercury::ReplyID replyID ) :
		replyBundle_(),
		srcAddr_( srcAddr )
	{
		replyBundle_.startReply( replyID );
	}

	virtual ~LookUpDBIDHandler() {}

	void lookUpDBID( EntityTypeID typeID, const std::string & name );

	// IDatabase::IGetEntityHandler/GetEntityHandler overrides
	virtual void onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation );

private:
	Mercury::Bundle		replyBundle_;
	Mercury::Address	srcAddr_;
};

#endif // LOOK_UP_DBID_HANDLER_HPP
