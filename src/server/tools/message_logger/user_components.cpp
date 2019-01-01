/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_components.hpp"

#include "logging_component.hpp"
#include "user_log.hpp"

#include "cstdmf/debug.hpp"


UserComponents::UserComponents() :
	idTicker_( 0 )
{ }


UserComponents::~UserComponents()
{
	IDMap::const_iterator iter = idMap_.begin();

	while (iter != idMap_.end())
	{
		delete iter->second;
		++iter;
	}
}


bool UserComponents::init( const char *root, const char *mode )
{
	// We store the filename in this object because we might need it later
	// when we're trying to insert app IDs into the file retrospectively.
	filename_ = this->join( root, "components" );

	return this->BinaryFileHandler::init( filename_.c_str(), mode );
}


void UserComponents::flush()
{
	IDMap::iterator it = idMap_.begin();

	while (it != idMap_.end())
	{
		delete it->second;
		++it;
	}

	idMap_.clear();
	addrMap_.clear();
}


/*
 * Override from FileHandler
 */
bool UserComponents::read()
{
	if (pFile_->error())
	{
		ERROR_MSG( "UserComponents::read: '%s'\n", pFile_->strerror() );
		return false;
	}

	long len = pFile_->length();
	pFile_->seek( 0 );

	while (pFile_->tell() < len)
	{
		LoggingComponent *pComponent = new LoggingComponent( this );
		pComponent->read( *pFile_ );

		if (pFile_->error())
		{
			ERROR_MSG( "UserComponents::read: "
				"Error encountered while reading '%s': %s\n",
				filename_.c_str(), pFile_->strerror() );
			return false;
		}

		addrMap_[ pComponent->getAddress() ] = pComponent;
		idMap_[ pComponent->getAppTypeID() ] = pComponent;

		// Keep the ticker ahead of any components we read from disk so that we
		// don't re-use existing id's when new components register.
		if (pComponent->getAppTypeID() >= idTicker_)
		{
			idTicker_ = pComponent->getAppTypeID() + 1;
		}
	}

	return true;
}


/*
 * Override from FileHandler
 */
bool UserComponents::write( LoggingComponent *logComponent )
{
	MF_ASSERT( logComponent != NULL );

	logComponent->write( *pFile_ );
	return pFile_->good();
}


/**
 * Returns the Component object for a particular LCM and address, and adds the
 * component to the mapping if it doesn't already exist.
 */
LoggingComponent * UserComponents::getComponentFromMessage(
	const LoggerComponentMessage &msg, const Mercury::Address &addr,
	LogComponentNames &logComponentNames )
{
	LoggingComponent *pComponent = NULL;

	AddrMap::iterator it = addrMap_.find( addr );
	if (it != addrMap_.end())
	{
		// Remove existing entries for that address if it's a different process
		LoggingComponent &existing = *it->second;

		if ((existing.msg_.version_ != msg.version_) ||
			(existing.msg_.uid_ != msg.uid_) ||
			(existing.msg_.pid_ != msg.pid_) ||
			(existing.msg_.componentName_ != msg.componentName_))
		{
			addrMap_.erase( it );
			idMap_.erase( idMap_.find( existing.getAppTypeID() ) );
			delete &existing;
		}
		else
		{
			pComponent = &existing;
		}
	}

	// If the component doesn't exist, create it and make entries for it in the
	// runtime mappings, but don't dump it to disk yet ... this is done in
	// UserLog::addEntry once we know the offset of the first log entry.
	if (pComponent == NULL)
	{
		int componentID = logComponentNames.getIDFromName( msg.componentName_ );
		pComponent = new LoggingComponent( this, addr, msg, componentID );

		MF_ASSERT( pComponent != NULL );

		addrMap_[ addr ] = pComponent;
		idMap_[ pComponent->getAppTypeID() ] = pComponent;
	}

	return pComponent;
}


/**
 * Returns the component for a particular address, but obviously can't make the
 * entry if it doesn't already exist.
 */
LoggingComponent * UserComponents::getComponentByAddr(
	const Mercury::Address &addr )
{
	AddrMap::const_iterator it = addrMap_.find( addr );

	if (it == addrMap_.end())
	{
		return NULL;
	}

	return it->second;
}


const LoggingComponent* UserComponents::getComponentByID( int id )
{
	IDMap::const_iterator it = idMap_.find( id );

	if (it == idMap_.end())
	{
		return NULL;
	}

	return it->second;
}


bool UserComponents::erase( const Mercury::Address &addr )
{
	AddrMap::iterator adit = addrMap_.find( addr );
	if (adit == addrMap_.end())
	{
		return false;
	}

	LoggingComponent *pComponent = adit->second;
	addrMap_.erase( adit );

	IDMap::iterator idit = idMap_.find( pComponent->getAppTypeID() );
	if (idit != idMap_.end())
	{
		idMap_.erase( idit );
	}
	else
	{
		ERROR_MSG( "UserComponents::erase: %s wasn't in the ID map!\n",
			pComponent->getString().c_str() );
	}

	delete pComponent;

	return true;
}


const char * UserComponents::getFilename() const
{
	return filename_.c_str();
}


int UserComponents::getID()
{
	return idTicker_++;
}


bool UserComponents::visitAllWith( UserComponentVisitor &visitor ) const
{
	IDMap::const_iterator iter = idMap_.begin();
	bool status = true;

	while ((iter != idMap_.end()) && (status == true))
	{
		status = visitor.onComponent(
					const_cast< LoggingComponent & >( *(iter->second) ));
		++iter;
	}

	return status;
}

// user_components.cpp
