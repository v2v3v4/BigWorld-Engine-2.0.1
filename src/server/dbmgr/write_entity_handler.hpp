/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WRITE_ENTITY_HANDLER_HPP
#define WRITE_ENTITY_HANDLER_HPP

#include "dbmgr_lib/idatabase.hpp"

#include "network/misc.hpp"


/**
 *	This class is used by Database::writeEntity() to write entities into
 *	the database and wait for the result.
 */
class WriteEntityHandler : public IDatabase::IPutEntityHandler,
                           public IDatabase::IDelEntityHandler
{

public:
	WriteEntityHandler( const EntityDBKey ekey, EntityID entityID,
			int8 flags, bool shouldReply,
			Mercury::ReplyID replyID, const Mercury::Address & srcAddr );

	void writeEntity( BinaryIStream & data, EntityID entityID );

	void deleteEntity();

	// IDatabase::IPutEntityHandler override
	virtual void onPutEntityComplete( bool isOK, DatabaseID );

	// IDatabase::IDelEntityHandler override
	virtual void onDelEntityComplete( bool isOK );

private:
	void putEntity( BinaryIStream * pStream,
			UpdateAutoLoad updateAutoLoad,
			EntityMailBoxRef * pBaseMailbox = NULL,
			bool removeBaseMailbox = false );

	void finalise( bool isOK );

private:
	EntityDBKey				ekey_;
	EntityID				entityID_;
	int8					flags_;
	bool					shouldReply_;
	Mercury::ReplyID		replyID_;
	const Mercury::Address	srcAddr_;
};

#endif // WRITE_ENTITY_HANDLER_HPP
