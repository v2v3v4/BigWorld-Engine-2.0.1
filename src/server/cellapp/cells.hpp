/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELLS_HPP
#define CELLS_HPP

#include "cstdmf/watcher.hpp"

#include <vector>

class Cell;
class BinaryOStream;

namespace Mercury
{
class Address;
}


/**
 *	This class handles a collection of Cell instances.
 */
class Cells
{
public:
	bool empty() const 				{ return container_.empty(); }
	size_t size() const 			{ return container_.size(); }

	int numRealEntities() const;

	void handleCellAppDeath( const Mercury::Address & addr );
	void checkOffloads();
	void backup( int index, int period );

	void add( Cell * pCell );
	void destroy( Cell * pCell );

	void addBoundsToStream( BinaryOStream & stream, int maxEntityOffload ) const;
	void destroyAll();

	void debugDump();

	static WatcherPtr pWatcher();

private:
	typedef std::vector< Cell * > Container;
	Container container_;
};

#endif // CELLS_HPP
