/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "baseapp.hpp"
#include "baseappmgr.hpp"
#include "baseappmgr_config.hpp"

#include "baseapp/baseapp_int_interface.hpp"

// -----------------------------------------------------------------------------
// Section: BaseApp
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
BaseApp::BaseApp( BaseAppMgr & baseAppMgr, 
			const Mercury::Address & intAddr,
			const Mercury::Address & extAddr,
			int id ) :
	ChannelOwner( baseAppMgr.interface(), intAddr ),
	baseAppMgr_( baseAppMgr ),
	externalAddr_( extAddr ),
	id_( id ),
	load_( 0.f ),
	numBases_( 0 ),
	numProxies_( 0 ),
	backupHash_(),
	newBackupHash_(),
	isRetiring_( false ),
	isOffloading_( false )
{
	this->channel().isLocalRegular( false );
	this->channel().isRemoteRegular( true );
}


/**
 *	This method estimates the cost of adding an entity to the BaseApp.
 */
void BaseApp::addEntity()
{
	// TODO: Make this configurable and consider having different costs for
	// different entity types.
	load_ =+ 0.01f;
}


/**
 *	This static method makes a watcher associated with this object type.
 */
WatcherPtr BaseApp::pWatcher()
{
	// generic watcher of a BaseApp structure
	WatcherPtr pWatcher = new DirectoryWatcher();
	BaseApp * pNullBaseApp = NULL;

	pWatcher->addChild( "id", makeWatcher( &BaseApp::id_ ) );
	pWatcher->addChild( "internalChannel",
			Mercury::ChannelOwner::pWatcher(),
			(ChannelOwner *)pNullBaseApp );
	pWatcher->addChild( "externalAddr", &Mercury::Address::watcher(),
		&pNullBaseApp->externalAddr_ );
	pWatcher->addChild( "load", makeWatcher( &BaseApp::load_ ) );
	pWatcher->addChild( "numBases", makeWatcher( &BaseApp::numBases_ ) );
	pWatcher->addChild( "numProxies", makeWatcher( &BaseApp::numProxies_ ) );

	return pWatcher;
}


/**
 *	This method returns whether or not the Base App Manager has heard
 *	from this Base App in the timeout period.
 */
bool BaseApp::hasTimedOut( uint64 currTime, uint64 timeoutPeriod,
	   uint64 timeSinceAnyHeard ) const
{
	bool result = false;

	uint64 diff = currTime - this->channel().lastReceivedTime();
	result = (diff > timeoutPeriod);

	if (result)
	{
		INFO_MSG( "BaseApp::hasTimedOut: Timed out - %.2f (> %.2f) %s\n",
				double( (int64)diff )/stampsPerSecondD(),
				double( (int64)timeoutPeriod )/stampsPerSecondD(),
				this->addr().c_str() );

		// The logic behind the following block of code is that if we
		// haven't heard from any baseapp in a long time, the baseappmgr is
		// probably the misbehaving app and we shouldn't start forgetting
		// about baseapps.  If we want to shutdown our server on bad state,
		// we want to be able to return true when our last baseapp dies, so
		// relax the following check.
		if (!BaseAppMgrConfig::shutDownServerOnBadState())
		{
			if (timeSinceAnyHeard > timeoutPeriod/2)
			{
				INFO_MSG( "BaseApp::hasTimedOut: "
					"Last inform time not recent enough %f\n",
					double((int64)timeSinceAnyHeard)/stampsPerSecondD() );
				result = false;
			}
		}
	}

	return result;
}


void BaseApp::retireApp( const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	int8 dummy;
	data >> dummy;

	isRetiring_ = true;

	baseAppMgr_.onBaseAppRetire( *this );
	this->checkToStartOffloading();
}


void BaseApp::startBackup( const Mercury::Address & addr )
{
	backingUp_.insert( addr );
}


void BaseApp::stopBackup( const Mercury::Address & addr )
{
	backingUp_.erase( addr );
	this->checkToStartOffloading();
}


/**
 *  A retiring app must retain the same backup hash throughout the offload 
 *  process or the entities' positions may not match where they are expected to
 *  be. Thus there is an isOffloading_ flag which marks a baseapp as having an
 *  immutable backup hash. Before that is set, we must check if the backup hash
 *  is suitable for the entire offloading process.
 */
void BaseApp::checkToStartOffloading()
{
	if (!this->isRetiring() || this->isOffloading())
	{
		return;
	}

	// Most importantly, the backup hash must be useful before we commit to it.
	if (backupHash_.empty())
	{
		return;
	}

	// If an app dies during retirement, its entities should be restored
	// where they would have been sent, to prevent two instances from emerging.
	if (!newBackupHash_.empty())
	{
		return;
	}

	// The logic behind this is that we need to check to see if there is any
	// chance that a baseapp crashing during the offload sequence will need to
	// be restored to the offloading app and thereafter be offloaded to the
	// crashed app ad infinitum. This case prevents the system from finding 
	// anywhere logical to put it and should be avoided.
	BackingUp::iterator iter;
	for (iter = backingUp_.begin(); iter != backingUp_.end(); iter++)
	{
		BaseApp * pBaseApp = BaseAppMgr::instance().findBaseApp( *iter );
		// An offloading app has nothing but offloading apps backing up to it.
		// since they are added sequentially there is no chance for a cycle.
		if (!pBaseApp || !pBaseApp->isOffloading())
		{
			return;
		}
	}

	// From here on in, we are committed to our current backup hash from the
	// first entity transfer to the final redirection of base mailboxes.
	isOffloading_ = true;

	this->bundle().startMessage( BaseAppIntInterface::startOffloading );
	this->send();
}

// baseapp.cpp
