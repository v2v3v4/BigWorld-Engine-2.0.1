/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_HPP
#define CELL_HPP

class Cell;
class Entity;

#include "Python.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <deque>

#include "cellapp_interface.hpp"
#include "cell_info.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/memory_stream.hpp"

#include "math/math_extra.hpp"
#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include "server/common.hpp"

typedef SmartPointer<Entity> EntityPtr;

class CellAppChannel;
class Space;

typedef std::vector< EntityPtr >::size_type EntityRemovalHandle;
const EntityRemovalHandle NO_ENTITY_REMOVAL_HANDLE = EntityRemovalHandle( -1 );


class Chunk;

/**
 *	This class is used to represent a cell.
 */
class Cell
{
public:
	/**
	 *	This class is used to store the collection of real entities.
	 */
	class Entities
	{
	public:
		typedef std::vector< EntityPtr > Collection;
		typedef Collection::iterator iterator;
		typedef Collection::const_iterator const_iterator;
		typedef Collection::size_type size_type;
		typedef Collection::value_type value_type;
		typedef Collection::reference reference;

		iterator begin()				{ return collection_.begin(); }
		const_iterator begin() const	{ return collection_.begin(); }
		iterator end()					{ return collection_.end(); }
		const_iterator end() const		{ return collection_.end(); }

		bool empty() const				{ return collection_.empty(); }
		size_type size() const			{ return collection_.size(); }

		bool add( Entity * pEntity );
		bool remove( Entity * pEntity );

		EntityPtr front()				{ return collection_.front(); }
		EntityPtr front() const			{ return collection_.front(); }

	private:
		void swapWithBack( Entity * pEntity );


	private:
		Collection collection_;
	};

	// Constructor/Destructor
	Cell( Space & space, const CellInfo & cellInfo );

	~Cell();

	void shutDown();

	// Accessors and inline methods
	const CellInfo & cellInfo()	{ return *pCellInfo_; }

	// Entity Maintenance C++ methods
	void offloadEntity( Entity * pEntity, CellAppChannel * pChannel,
			bool shouldSendPhysicsCorrection = false );

	void addRealEntity( Entity * pEntity, bool shouldSendNow );

	void entityDestroyed( Entity * pEntity );

	EntityPtr createEntityInternal(	BinaryIStream & data, PyObject * pDict,
		bool isRestore = false,
		Mercury::ChannelVersion channelVersion = Mercury::SEQ_NULL,
		EntityPtr pNearbyEntity = NULL );

	void backup( int index, int period );

	bool checkOffloadsAndGhosts();
	void checkChunkLoading();

	void onSpaceGone();

	void debugDump();

	// Communication message handlers concerning entities
	void createEntity( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data,
		EntityPtr pNearbyEntity );

	void createEntity( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data )
	{
		this->createEntity( srcAddr, header, data, NULL );
	}

	// Instrumentation
	static WatcherPtr pWatcher();

	SpaceID spaceID() const;
	Space & space()						{ return space_; }

	const Space & space() const			{ return space_; }

	const BW::Rect & rect() const		{ return pCellInfo_->rect(); }

	int numRealEntities() const;

	void sendEntityPositions( Mercury::Bundle & bundle ) const;

	Entities & realEntities();

	// Load balancing
	bool shouldOffload() const;
	void shouldOffload( bool shouldOffload );
	void shouldOffload( BinaryIStream & data );

	void retireCell( BinaryIStream & data );
	void removeCell( BinaryIStream & data );

	void notifyOfCellRemoval( BinaryIStream & data );

	void ackCellRemoval( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	bool reuse();

	void handleCellAppDeath( const Mercury::Address & addr );

	void restoreEntity( const Mercury::Address& srcAddr,
		const Mercury::UnpackedMessageHeader& header,
		BinaryIStream & data );

	bool isRemoved() const	{ return isRemoved_; }

private:
	// General private data
	Entities realEntities_;

	bool shouldOffload_;

	mutable float lastERTFactor_;
	mutable uint64 lastERTCalcTime_;

	friend class CellViewerConnection;

	// Load balance related data
	float initialTimeOfDay_;
	float gameSecondsPerSecond_;
	bool isRetiring_;
	bool isRemoved_;

	Space & space_;

	int backupIndex_;
	ConstCellInfoPtr pCellInfo_;

	typedef std::multiset< Mercury::Address > RemovalAcks;
	RemovalAcks pendingAcks_;
	RemovalAcks receivedAcks_;
};


#ifdef CODE_INLINE
#include "cell.ipp"
#endif

#endif // CELL_HPP
