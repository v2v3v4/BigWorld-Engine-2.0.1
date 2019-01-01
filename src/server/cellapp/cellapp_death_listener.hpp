/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_APP_DEATH_LISTENER_HPP
#define CELL_APP_DEATH_LISTENER_HPP

#include "network/basictypes.hpp"
#include <list>

class CellAppDeathListener;

/**
 *  A global collection of CellAppDeathListeners.
 */
class CellAppDeathListeners
{
public:
	typedef std::list< CellAppDeathListener* > Listeners;
	typedef Listeners::iterator iterator;

	static CellAppDeathListeners & instance();

	void handleCellAppDeath( const Mercury::Address & addr );
	iterator addListener( CellAppDeathListener * pListener );
	void delListener( iterator & iter );

private:
	static CellAppDeathListeners * s_pInstance_;
	Listeners listeners_;
};


/**
 *  This interface should be derived from by anything that needs to know when a
 *  CellApp dies.  A good example is any object that holds onto a
 *  CellAppChannel.  If a CellApp dies, they will be automatically notified of
 *  the dead CellApp's address.
 *
 *  Generally the idea is that collections should implement this interface,
 *  e.g. ReplacedGhosts and EntityPopulation.
 */
class CellAppDeathListener
{
public:
	CellAppDeathListener();
	virtual ~CellAppDeathListener();

	/**
	 *  This method will be called on each listener when a CellApp dies.
	 */
	virtual void handleCellAppDeath( const Mercury::Address & addr ) = 0;

private:
	/// Iterator that tracks this object's position in the global collection.
	CellAppDeathListeners::iterator iter_;
};


#endif // CELL_APP_DEATH_LISTENER_HPP
