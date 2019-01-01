/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEAD_CELL_APPS_HPP
#define DEAD_CELL_APPS_HPP

#include "cstdmf/shared_ptr.hpp"

#include <list>
#include <vector>

class Bases;
class BinaryIStream;
class DeadCellApp;
class DyingCellApp;

namespace Mercury
{
class Address;
class NetworkInterface;
}


/**
 *	This class handles identifying CellApps that have recently died.
 */
class DeadCellApps
{
public:
	bool isRecentlyDead( const Mercury::Address & addr ) const;
	void addApp( const Mercury::Address & addr, BinaryIStream & data );

	void tick( const Bases & bases, Mercury::NetworkInterface & intInterface );

private:
	void removeOldApps();

	typedef std::vector< shared_ptr< DeadCellApp > > Container;
	Container apps_;

	typedef std::list< shared_ptr< DyingCellApp > > DyingCellApps;
	DyingCellApps dyingApps_;
};

#endif // DEAD_CELL_APPS_HPP
