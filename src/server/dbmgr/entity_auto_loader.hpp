/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_AUTO_LOADER_HPP
#define ENTITY_AUTO_LOADER_HPP

#include "dbmgr_lib/entity_auto_loader_interface.hpp"

#include <vector>
#include <utility>

/**
 *	This class is used for loading entities from the database over a period of
 *	time.
 */
class EntityAutoLoader : public IEntityAutoLoader
{
public:
	EntityAutoLoader();

	virtual void reserve( int numEntities );

	virtual void start();

	virtual void abort();

	virtual void addEntity( EntityTypeID entityTypeID, DatabaseID dbID );

	virtual void onAutoLoadEntityComplete( bool isOK );

private:
	void checkFinished();
	bool sendNext();

	bool allSent() const	{ return numSent_ >= int(entities_.size()); }

	typedef std::vector< std::pair< EntityTypeID, DatabaseID > > Entities;
	Entities 	entities_;
	int 		numOutstanding_;
	int 		numSent_;
	bool 		hasErrors_;
};

#endif // ENTITY_AUTO_LOADER_HPP
