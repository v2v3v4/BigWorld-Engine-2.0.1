/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "vision_controller.hpp"

#include "cellapp.hpp"
#include "entity.hpp"
#include "entity_population.hpp"
#include "entity_range_list_node.hpp"
#include "entity_vision.hpp"
#include "range_list_node.hpp"

DECLARE_DEBUG_COMPONENT(0)

/**
 * This class is used for vision range triggers
 */
class VisionRangeTrigger : public RangeTrigger
{
public:
	VisionRangeTrigger( Entity & around, float range ) :
		RangeTrigger( around.pRangeListNode(), range ),
		disabled_( false )
	{
		this->insert();
		Entity * pEntity = ((EntityRangeListNode*)pSubject_)->getEntity();
		pEntity->addTrigger( this );
	}

	~VisionRangeTrigger()
	{
		Entity * pEntity = ((EntityRangeListNode*)pSubject_)->getEntity();
		pEntity->delTrigger( this );
		if (disabled_)
		{
			this->removeWithoutContracting();
		}
		else
		{
			this->remove();
		}
	}

	void setRangeAndMod( float r )
	{
		this->setRange( r );
		Entity * pEntity = ((EntityRangeListNode*)pSubject_)->getEntity();
		pEntity->modTrigger( this );
	}

	virtual std::string debugString() const;

	virtual void triggerEnter( RangeListNode * who );
	virtual void triggerLeave( RangeListNode * who );

	void disable()	{ disabled_ = true; }

private:
	bool	disabled_;
};


IMPLEMENT_CONTROLLER_TYPE( VisionController, DOMAIN_REAL )

/**
 * 	VisionController constructor
 *
 *	@param visionAngle	This is angle (in radians) of the vision cone.
 * 						The field of view is double this value.
 *	@param visionRange	The range of the vision.
 *	@param seeingHeight	The height above the entity position to see from.
 *	@param updatePeriod This is how much in between checking vision (in ticks).
 */
VisionController::VisionController( float visionAngle, float visionRange,
		float seeingHeight, int updatePeriod ):
	visionAngle_( visionAngle ),
	visionRange_( visionRange ),
	seeingHeight_( seeingHeight ),
	updatePeriod_( updatePeriod ),
	tickSinceLast_( 0 ),
	pVisionTrigger_( NULL ),
	pOnloadedVisible_( NULL )
{
}


/**
 *	This method overrides the Controller method.
 */
void VisionController::startReal( bool isInitialStart )
{
	// We only have pOnloadedVisible_ if this is not the initial start.
	MF_ASSERT( !isInitialStart || (pOnloadedVisible_ == NULL) );

	EntityVision & ev = EntityVision::instance( this->entity() );

	if (pOnloadedVisible_)
	{
		ev.setVisibleEntities( *pOnloadedVisible_ );
	}

	VisionController* oldVC = ev.getVision();
	if (oldVC != NULL)
	{
		ERROR_MSG( "VisionController::startReal: "
			"Entity %u already has vision controller. old=%d, new=%d\n",
			this->entity().id(), oldVC->id(), this->id() );
	}
	ev.setVision( this );

	CellApp::instance().registerForUpdate( this );

	// add our vision trigger if we have an appropriate range
	this->setVisionRange( visionAngle_, visionRange_ );
	// we might now be cancelled or deleted
}


/**
 *	This method overrides the Controller method.
 */
void VisionController::stopReal( bool isFinalStop )
{
	MF_VERIFY( CellApp::instance().deregisterForUpdate( this ) );

	EntityPtr pEntity = &this->entity();

	if (!isFinalStop && pVisionTrigger_ != NULL)
	{
		pVisionTrigger_->disable();
	}

	this->setVisionRange( visionAngle_, 0.f );
	// we might now be cancelled or deleted

	// Note: the line below might want to move above the script callbacks...
	EntityVision::instance( *pEntity ).setVision( NULL );
}


/**
 *	Yaw offset for vision
 *
 */
float VisionController::getYawOffset()
{
	return 0.f;
}

/**
 *	This method is called every tick.
 */
void VisionController::update()
{
	AUTO_SCOPED_PROFILE( "visionUpdate" );

	if (tickSinceLast_ >= updatePeriod_)
	{
		tickSinceLast_ = 0;

		// Keep ourselves alive until we have finished cleaning up,
		// with an extra reference count from a smart pointer.
		ControllerPtr pController = this;

		EntityVision::instance( this->entity() ).updateVisibleEntities(
			seeingHeight_, this->getYawOffset() );
	}
	else
	{
		tickSinceLast_++;
	}
}


/**
 *	This method writes our state to a stream.
 *
 *	@param stream		Stream to which we should write
 */
void VisionController::writeRealToStream( BinaryOStream & stream )
{
	this->Controller::writeRealToStream( stream );
	stream << visionAngle_ << visionRange_ << tickSinceLast_ << updatePeriod_;
	const EntitySet & visibleEntities =
		EntityVision::instance( this->entity() ).visibleEntities();

	stream << (int32)visibleEntities.size();
	EntitySet::const_iterator iter = visibleEntities.begin();
	EntitySet::const_iterator endIter = visibleEntities.end();

	while (iter != endIter)
	{
		stream << (*iter)->id();
		++iter;
	}
}


/**
 *	This method reads our state from a stream.
 *
 *	@param stream		Stream from which to read
 *	@return				true if successful, false otherwise
 */
bool VisionController::readRealFromStream( BinaryIStream & stream )
{
	this->Controller::readRealFromStream( stream );
	stream >> visionAngle_ >> visionRange_ >> tickSinceLast_ >> updatePeriod_;

	int visibleSize;
	stream >> visibleSize;

	if (visibleSize > 0)
	{
		pOnloadedVisible_ = new std::vector< EntityID >( visibleSize );
		for (int i = 0; i < visibleSize; ++i)
		{
			stream >> (*pOnloadedVisible_)[i];
		}
	}

	return true;
}


/**
 *	This method sets the height that you look from.
 */
void VisionController::seeingHeight( float h )
{
	MF_ASSERT( entity().isReal() );

	seeingHeight_ = h;
}


/**
 *	This method sets the range of your vision.
 */
void VisionController::setVisionRange( float visionAngle, float range )
{
	visionAngle_ = visionAngle;
	visionRange_ = range;

	// all three of the cases below can call script
	Entity::callbacksPermitted( false );

	START_PROFILE( SHUFFLE_TRIGGERS_PROFILE );
	if (range <= 0.f)
	{
		delete pVisionTrigger_;	// could be NULL
		pVisionTrigger_ = NULL;
		visionRange_ = 0.f;
	}
	else if (pVisionTrigger_ == NULL)
	{
		pVisionTrigger_ = new VisionRangeTrigger( this->entity(), range );
	}
	else
	{
		pVisionTrigger_->setRangeAndMod( range );
	}
	STOP_PROFILE( SHUFFLE_TRIGGERS_PROFILE );

	Entity::callbacksPermitted( true );
	// we might now be cancelled or deleted
}


// -----------------------------------------------------------------------------
// Section: VisionRangeTrigger
// -----------------------------------------------------------------------------

/**
 *	This method is called when an entity triggers the node. It forwards the call
 *	to the entity.
 *
 *	@param who who triggered this trigger
 */
void VisionRangeTrigger::triggerEnter( RangeListNode * who )
{
	MF_ASSERT( !disabled_ );

	Entity * pEntity = ((EntityRangeListNode*)pSubject_)->getEntity();

	if (pEntity->isDestroyed())
	{
		// ERROR_MSG( "VisionRangeTrigger::triggerEnter: "
		//			"Called on destroyed entity %d\n", pEntity->id() );
		return;
	}

	EntityVision::instance( *pEntity ).
		triggerVisionEnter( ((EntityRangeListNode*)who)->getEntity() );
}


/**
 *	This method is called when an entity untriggers the node. It forwards the
 *	call to the entity.
 *
 *	@param who who untriggered this trigger
 */
void VisionRangeTrigger::triggerLeave( RangeListNode * who )
{
	MF_ASSERT( !disabled_ );

	Entity * pEntity = ((EntityRangeListNode*)pSubject_)->getEntity();

	if (pEntity->isDestroyed())
	{
		// ERROR_MSG( "VisionRangeTrigger::triggerLeave: "
		// 			"Called on destroyed entity %d\n", pEntity->id() );
		return;
	}

	EntityVision::instance( *pEntity ).
		triggerVisionLeave( ((EntityRangeListNode*)who)->getEntity() );
}


/**
 *	This method returns the identifier for the VisionRangeTriggerNode.
 *
 *	@return the string identifier of the node
 */
std::string VisionRangeTrigger::debugString() const
{
	char buf[80];
	bw_snprintf( buf, sizeof(buf), "%d for vision",
		(int)((EntityRangeListNode*)pSubject_)->getEntity()->id() );

	return buf;
}

// vision_controller.cpp
