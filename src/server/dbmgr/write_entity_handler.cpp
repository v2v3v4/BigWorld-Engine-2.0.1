/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "write_entity_handler.hpp"

#include "database.hpp"

#include "network/channel_sender.hpp"
#include "server/writedb.hpp"


/**
 *	Constructor
 */
WriteEntityHandler::WriteEntityHandler( const EntityDBKey ekey,
			EntityID entityID,
			int8 flags, bool shouldReply,
			Mercury::ReplyID replyID, const Mercury::Address & srcAddr ) :
	ekey_( ekey ),
	entityID_( entityID ),
	flags_( flags ),
	shouldReply_( shouldReply ),
	replyID_( replyID ),
	srcAddr_( srcAddr )
{
}


/**
 *	This method writes the entity data into the database.
 *
 *	@param	data	Stream should be currently at the start of the entity's
 *	data.
 *	@param	entityID	The entity's base mailbox object ID.
 */
void WriteEntityHandler::writeEntity( BinaryIStream & data, EntityID entityID )
{
	BinaryIStream * pStream = NULL;

	UpdateAutoLoad updateAutoLoad = 
		(flags_ & WRITE_AUTO_LOAD_YES) 	? 	UPDATE_AUTO_LOAD_TRUE :
		(flags_ & WRITE_AUTO_LOAD_NO) 	? 	UPDATE_AUTO_LOAD_FALSE:
											UPDATE_AUTO_LOAD_RETAIN;

	if (flags_ & WRITE_BASE_CELL_DATA)
	{
		pStream = &data;
	}

	if (flags_ & WRITE_LOG_OFF)
	{
		this->putEntity( pStream, updateAutoLoad,
			/* pBaseMailbox: */ NULL,
			/* removeBaseMailbox: */ true );
	}
	else if (ekey_.dbID == 0)
	{
		// New entity is checked out straight away
		EntityMailBoxRef	baseRef;
		baseRef.init( entityID, srcAddr_, EntityMailBoxRef::BASE,
			ekey_.typeID );
		this->putEntity( pStream, updateAutoLoad, &baseRef );
	}
	else
	{
		this->putEntity( pStream, updateAutoLoad );
	}
	// When putEntity() completes onPutEntityComplete() is called.
}


/**
 *	IDatabase::IPutEntityHandler override
 */
void WriteEntityHandler::onPutEntityComplete( bool isOK, DatabaseID dbID )
{
	ekey_.dbID = dbID;

	if (!isOK)
	{
		ERROR_MSG( "WriteEntityHandler::writeEntity: "
						"Failed to update entity %"FMT_DBID" of type %d.\n",
					dbID, ekey_.typeID );
	}

	this->finalise(isOK);
}


/**
 *	Deletes the entity from the database.
 */
void WriteEntityHandler::deleteEntity()
{
	MF_ASSERT( flags_ & WRITE_DELETE_FROM_DB );
	Database::instance().delEntity( ekey_, entityID_, *this );
	// When delEntity() completes, onDelEntityComplete() is called.
}


/**
 *	IDatabase::IDelEntityHandler override
 */
void WriteEntityHandler::onDelEntityComplete( bool isOK )
{
	if (!isOK)
	{
		ERROR_MSG( "WriteEntityHandler::writeEntity: "
						"Failed to delete entity %"FMT_DBID" of type %d.\n",
					ekey_.dbID, ekey_.typeID );
	}

	this->finalise(isOK);
}


/**
 *	This method is invoked by WriteEntityHandler::writeEntity to pass through
 *	a putEntity request to the database implementation.
 */
void WriteEntityHandler::putEntity( BinaryIStream * pStream,
			UpdateAutoLoad updateAutoLoad,
			EntityMailBoxRef * pBaseMailbox,
			bool removeBaseMailbox )
{
	Database::instance().putEntity( ekey_, entityID_,
			pStream, pBaseMailbox, removeBaseMailbox, updateAutoLoad, *this );
}


/**
 *	This function does some common stuff at the end of the operation.
 */
void WriteEntityHandler::finalise( bool isOK )
{
	if (shouldReply_)
	{
		Mercury::ChannelSender sender( Database::getChannel( srcAddr_ ) );
		sender.bundle().startReply( replyID_ );
		sender.bundle() << isOK << ekey_.dbID;
	}

	if (isOK && (flags_ & WRITE_LOG_OFF))
	{
		Database::instance().onEntityLogOff( ekey_.typeID, ekey_.dbID );
	}

	delete this;
}

// write_entity_handler.cpp
