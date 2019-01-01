/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "database.hpp"
#include "entity_auto_loader.hpp"
#include "get_entity_handler.hpp"

DECLARE_DEBUG_COMPONENT( 0 )


namespace // anonymous
{


// -----------------------------------------------------------------------------
// Section: AutoLoadingEntityHandler
// -----------------------------------------------------------------------------
/**
 *	This class handles auto loading an entity from the database.
 */
class AutoLoadingEntityHandler :
							public Mercury::ShutdownSafeReplyMessageHandler,
							public GetEntityHandler,
							public IDatabase::IPutEntityHandler
{
public:
	AutoLoadingEntityHandler( EntityTypeID entityTypeID, DatabaseID dbID,
		EntityAutoLoader& mgr );
	virtual ~AutoLoadingEntityHandler()
	{
		mgr_.onAutoLoadEntityComplete( isOK_ );
	}

	void autoLoad();

	virtual void handleMessage( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * );
	virtual void handleException( const Mercury::NubException & ne, void * );

	// IDatabase::IGetEntityHandler/GetEntityHandler overrides
	virtual void onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation );

	// IDatabase::IPutEntityHandler overrides
	virtual void onPutEntityComplete( bool isOK, DatabaseID dbID );

private:
	enum State
	{
		StateInit,
		StateWaitingForSetBaseToLoggingOn,
		StateWaitingForCreateBase,
		StateWaitingForSetBaseToFinal
	};

	State				state_;
	EntityDBKey			ekey_;
	Mercury::Bundle		createBaseBundle_;
	EntityAutoLoader &	mgr_;
	bool				isOK_;
};


/**
 *	Constructor.
 */
AutoLoadingEntityHandler::AutoLoadingEntityHandler( EntityTypeID typeID,
		DatabaseID dbID, EntityAutoLoader & mgr ) :
	state_(StateInit),
	ekey_( typeID, dbID ),
	createBaseBundle_(),
	mgr_( mgr ),
	isOK_( true )
{}


/**
 *	Start auto-loading the entity.
 */
void AutoLoadingEntityHandler::autoLoad()
{
	// Start create new base message even though we're not sure entity exists.
	// This is to take advantage of getEntity() streaming properties into the
	// bundle directly.
	Database::prepareCreateEntityBundle( ekey_.typeID, ekey_.dbID,
		Mercury::Address( 0, 0 ), this, createBaseBundle_ );

	// Get entity data into bundle
	Database::instance().getEntity( ekey_, &createBaseBundle_,
			false, NULL, *this );
	// When getEntity() completes onGetEntityCompleted() is called.
}


/**
 *	Handles response from BaseAppMgr that base was created successfully.
 */
void AutoLoadingEntityHandler::handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, void * )
{
	Mercury::Address proxyAddr;
	data >> proxyAddr;
	EntityMailBoxRef baseRef;
	data >> baseRef;
	// Still may contain a sessionKey if it is a proxy and contains
	// latestVersion and impendingVersion from the BaseAppMgr.
	data.finish();

	state_ = StateWaitingForSetBaseToFinal;

	Database::instance().setBaseEntityLocation( ekey_, baseRef, *this );
	// When completes, onPutEntityComplete() is called.
}


/**
 *	Handles response from BaseAppMgr that base creation has failed.
 */
void AutoLoadingEntityHandler::handleException(
	const Mercury::NubException & ne, void * )
{
	isOK_ = false;
	delete this;
}


void AutoLoadingEntityHandler::onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation )
{
	ekey_ = entityKey;

	if (isOK)
	{
		state_ = StateWaitingForSetBaseToLoggingOn;
		EntityMailBoxRef	baseRef;
		Database::setBaseRefToLoggingOn( baseRef, ekey_.typeID );

		Database::instance().setBaseEntityLocation( ekey_,
				baseRef, *this );
		// When completes, onPutEntityComplete() is called.
	}
	else
	{
		ERROR_MSG( "AutoLoadingEntityHandler::onGetEntityCompleted: "
				"Failed to load entity %"FMT_DBID" of type %d\n",
				entityKey.dbID, entityKey.typeID );
		isOK_ = false;
		delete this;
	}
}


void AutoLoadingEntityHandler::onPutEntityComplete( bool isOK, DatabaseID dbID )
{
	MF_ASSERT( isOK );

	if (state_ == StateWaitingForSetBaseToLoggingOn)
	{
		state_ = StateWaitingForCreateBase;
		Database::instance().baseAppMgr().channel().send( &createBaseBundle_ );
	}
	else
	{
		MF_ASSERT(state_ == StateWaitingForSetBaseToFinal);
		delete this;
	}
}

} // end namespace


// -----------------------------------------------------------------------------
// Section: EntityAutoLoader
// -----------------------------------------------------------------------------
/**
 *	Constructor.
 */
EntityAutoLoader::EntityAutoLoader() :
	numOutstanding_( 0 ),
	numSent_( 0 ),
	hasErrors_( false )
{}


/**
 *	Optimisation. Reserves the correct number of entities to be auto-loaded.
 */
void EntityAutoLoader::reserve( int numEntities )
{
	entities_.reserve( numEntities );
}


/**
 *	This method starts loading the entities into the system.
 */
void EntityAutoLoader::start()
{
	// TODO: Make this configurable.
	const int maxOutstanding = 5;

	while ((numOutstanding_ < maxOutstanding) && this->sendNext())
	{
		// Do nothing.
	}
}


/**
 *	This method is used instead of start() to indicate that there was an
 * 	error.
 */
void EntityAutoLoader::abort()
{
	entities_.clear();
	Database::instance().startServerError();
	delete this;
}


/**
 *	This method adds a database entry that will later be loaded.
 */
void EntityAutoLoader::addEntity( EntityTypeID entityTypeID, DatabaseID dbID )
{
	entities_.push_back( std::make_pair( entityTypeID, dbID ) );
}


/**
 *	This method loads the next pending entity.
 */
bool EntityAutoLoader::sendNext()
{
	bool done = this->allSent();
	if (!done)
	{
		AutoLoadingEntityHandler* pEntityAutoLoader =
			new AutoLoadingEntityHandler( entities_[numSent_].first,
				entities_[numSent_].second, *this );
		pEntityAutoLoader->autoLoad();

		++numSent_;

		// TRACE_MSG( "EntityAutoLoader::sendNext: numSent = %d\n", numSent_ );

		++numOutstanding_;
	}

	this->checkFinished();

	return !done;
}


/**
 *	AutoLoadingEntityHandler calls this method when the process of auto-loading
 *	the entity has completed - regardless of success or failure.
 */
void EntityAutoLoader::onAutoLoadEntityComplete( bool isOK )
{
	--numOutstanding_;
	if (isOK)
	{
		if (!hasErrors_)
		{
			this->sendNext();
		}
	}
	else
	{
		hasErrors_ = true;
		this->checkFinished();
	}
}


/**
 *	This method checks whether or not this object has finished its job. If it
 *	has, this object deletes itself.
 */
void EntityAutoLoader::checkFinished()
{
	if (numOutstanding_ == 0)
	{
		if (hasErrors_)
		{
			Database::instance().startServerError();
		}
		else if (this->allSent())
		{
			bool didAutoLoadEntitiesFromDB = (numSent_ > 0);
			Database::instance().startServerEnd( /*isRecover=*/ false,  
				didAutoLoadEntitiesFromDB );
		}
		delete this;
	}
}

// entity_auto_loader.cpp
