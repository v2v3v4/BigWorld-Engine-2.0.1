/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACES_HPP
#define SPACES_HPP

#include "cstdmf/watcher.hpp"

#include "network/basictypes.hpp"

#include <map>

class BinaryOStream;
class Space;

class Spaces
{
public:
	~Spaces();

	Space * find( SpaceID id ) const;
	Space * create( SpaceID id );

	void prepareNewlyLoadedChunksForDelete();
	void tickChunks();
	void deleteOldSpaces();
	void writeRecoveryData( BinaryOStream & stream );

	size_t size() const		{ return container_.size(); }

	WatcherPtr pWatcher();

private:
	typedef std::map< SpaceID, Space * > Container;
	Container container_;
};
#endif // SPACES_HPP
