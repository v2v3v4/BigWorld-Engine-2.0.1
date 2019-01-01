/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REAL_ENTITY_HPP
#define REAL_ENTITY_HPP

#include "cellapp_interface.hpp"
#include "cell_app_channel.hpp"
#include "controller.hpp"
#include "entity_cache.hpp"
#include "entity_type.hpp"
#include "history_event.hpp"
#include "mailbox.hpp"
#include "updatable.hpp"
#include "entity.hpp"

#include "network/channel.hpp"


#include "waypoint/navigator.hpp"
#include "waypoint/navloc.hpp"

#include "pyscript/pyobject_plus.hpp"

#include "server/auto_backup_and_archive.hpp"
#include "server/util.hpp"

#include <math.h>
#include <set>

typedef SmartPointer<Entity> EntityPtr;
class MemoryOStream;
class Space;
class Witness;

// From cell.hpp
typedef std::vector< EntityPtr >::size_type EntityRemovalHandle;

#define PY_METHOD_ATTRIBUTE_REAL_ENTITY_WITH_DOC( METHOD_NAME, DOC_STRING )	\
	PyObject * get_##METHOD_NAME()											\
	{																		\
		return new RealEntityMethod( this, &_##METHOD_NAME );				\
	}																		\


/**
 *	This class is the Python object for methods in a Real Entity
 */
class RealEntityMethod : public PyObjectPlus
{
	Py_Header( RealEntityMethod, PyObjectPlus )

public:
	typedef PyObject * (*StaticGlue)(
		PyObject * pRealEntity, PyObject * args, PyObject * kwargs );

	RealEntityMethod( RealEntity * re, StaticGlue glueFn,
			PyTypePlus * pType = &s_type_ );

	PY_KEYWORD_METHOD_DECLARE( pyCall )

private:
	EntityPtr	pEntity_;
	StaticGlue	glueFn_;
};


#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_REAL_ENTITY_WITH_DOC

typedef SmartPointer<BaseEntityMailBox> BaseEntityMailBoxPtr;

/**
 *	An object of this type is used by Entity to store additional data when the
 *	entity is "real" (as opposed to ghosted).
 */
class RealEntity
{
	PY_FAKE_PYOBJECTPLUS_BASE_DECLARE()
	Py_FakeHeader( RealEntity, PyObjectPlus )

public:
	/**
	 *	This class is used to represent the location of a ghost.
	 */
	class Haunt
	{
	public:
		Haunt( CellAppChannel * pChannel, GameTime creationTime ) :
			pChannel_( pChannel ),
			creationTime_( creationTime )
		{}

		// A note about these accessors.  We don't need to guard their callers
		// with ChannelSenders because having haunts guarantees that the
		// underlying channel is regularly sent.  If haunts are destroyed and
		// the channel becomes irregular, unsent data is sent immediately.
		CellAppChannel & channel() { return *pChannel_; }
		Mercury::Bundle & bundle() { return pChannel_->bundle(); }
		const Mercury::Address & addr() const { return pChannel_->addr(); }

		void creationTime( GameTime time )	{ creationTime_ = time; }
		GameTime creationTime() const		{ return creationTime_; }

	private:
		CellAppChannel * pChannel_;
		GameTime creationTime_;
	};

	typedef std::vector< Haunt > Haunts;

	static void addWatchers();

	RealEntity( Entity & owner );

	bool init( BinaryIStream & data, CreateRealInfo createRealInfo,
			Mercury::ChannelVersion channelVersion =
				Mercury::SEQ_NULL,
			const Mercury::Address * pBadHauntAddr = NULL );

	void destroy( const Mercury::Address * pNextRealAddr = NULL );

	void writeOffloadData( BinaryOStream & data,
			const Mercury::Address & dstAddr,
			bool shouldSendPhysicsCorrection );

	void enableWitness( BinaryIStream & data, Mercury::ReplyID replyID );
	void disableWitness( bool isRestore = false );

	Entity & entity()								{ return entity_; }
	const Entity & entity() const					{ return entity_; }

	Witness * pWitness()							{ return pWitness_; }
	const Witness * pWitness() const				{ return pWitness_; }

	Haunts::iterator hauntsBegin() { return haunts_.begin(); }
	Haunts::iterator hauntsEnd() { return haunts_.end(); }
	int numHaunts() const { return haunts_.size(); }

	void addHaunt( CellAppChannel & channel );
	Haunts::iterator delHaunt( Haunts::iterator iter );

    HistoryEvent * addHistoryEvent( uint8 type,
		MemoryOStream & stream,
		bool sendToGhosts,
		HistoryEvent::Level level,
		EntityMemberStats * pChangedDescription,
		const std::string * pName = NULL );

	void backup();
	void autoBackup();

	friend PyObject * calcCellBackupData( PyObjectPtr pEnt );
	void writeBackupProperties( BinaryOStream & data ) const;

	void debugDump();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PyObject * pyAdditionalMembers( PyObject * pSeq ) { return pSeq; }
	PyObject * pyAdditionalMethods( PyObject * pSeq ) { return pSeq; }

	void sendPhysicsCorrection();
	void newPosition( const Vector3 & position );

	void addDelGhostMessage( Mercury::Bundle & bundle );
	void deleteGhosts();

	const NavLoc & navLoc() const	{ return navLoc_; }
	void navLoc( const NavLoc & n )		{ navLoc_ = n; }

	Navigator & navigator()			{ return navigator_; }

	const Vector3 & velocity() const			{ return velocity_; }

	EntityRemovalHandle removalHandle() const		{ return removalHandle_; }
	void removalHandle( EntityRemovalHandle h )		{ removalHandle_ = h; }

	const EntityMailBoxRef & controlledByRef() const{ return controlledBy_; }

	GameTime creationTime() const	{ return creationTime_; }

	void delControlledBy( EntityID deadID );

	Mercury::Channel & channel()	{ return *pChannel_; }

	bool controlledBySelf() const	{ return entity_.id() == controlledBy_.id; }
	bool controlledByOther() const
			{ return !this->controlledBySelf() && (controlledBy_.id != 0); }

	void teleport( const EntityMailBoxRef & dstMailBoxRef );

	// ---- Python methods ----
	bool teleport( const EntityMailBoxRef & nearbyMBRef,
		const Vector3 & position, const Vector3 & direction );
	PY_AUTO_METHOD_DECLARE( RETOK, teleport, ARG( EntityMailBoxRef,
		ARG( Vector3, ARG( Vector3, END ) ) ) )

	BaseEntityMailBoxPtr controlledBy();
	void controlledBy( BaseEntityMailBoxPtr pNewMaster );
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE(
		BaseEntityMailBoxPtr, controlledBy, controlledBy )

	bool isWitnessed() const;
	PY_RO_ATTRIBUTE_DECLARE( isWitnessed(), isWitnessed )

	PY_RO_ATTRIBUTE_DECLARE( !!pWitness_, hasWitness )

	PY_RW_ATTRIBUTE_DECLARE( shouldAutoBackup_, shouldAutoBackup )

private:
	// ---- Private methods ----
	~RealEntity();

	bool readOffloadData( BinaryIStream & data,
		const Mercury::Address * pBadHauntAddr = NULL,
		bool * pHasChangedSpace = NULL );
	void readBackupData( BinaryIStream & data );
	void readBackupDataInternal( BinaryIStream & data );

	void writeBackupData( BinaryOStream & data ) const;
	void writeBackupDataInternal( BinaryOStream & data ) const;

	void setWitness( Witness * pWitness );

	void notifyWardOfControlChange( bool hasControl );

	// ---- Private data ----
	Entity & entity_;

	Witness * pWitness_;

	Haunts haunts_;

	NavLoc			navLoc_;
	Navigator		navigator_;

	// Used by cell to quick remove the entity from the real entities.
	EntityRemovalHandle	removalHandle_;

	EntityMailBoxRef	controlledBy_;

	Vector3			velocity_;
	Vector3			positionSample_;
	GameTime		positionSampleTime_;

	GameTime		creationTime_;

	AutoBackupAndArchive::Policy shouldAutoBackup_;

	Mercury::Channel *			pChannel_;
};

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC


#ifdef CODE_INLINE
#include "real_entity.ipp"
#endif

#endif // REAL_ENTITY_HPP

// real_entity.hpp
