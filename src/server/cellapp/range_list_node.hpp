/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RANGE_LIST_NODE_HPP
#define RANGE_LIST_NODE_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/vectornodest.hpp"

#include <float.h>
#include <string>
#include <vector>

class Entity;
class RangeTrigger;
class RangeListNode;


/**
 * This class is the base of all range nodes. Range Nodes are used to keep track of
 * the order of entities and triggers relative to the x axis or the z axis.
 * This is used for AoI calculations and range queries.
 */
class RangeListNode
{
public:
	enum Flags
	{
		FLAG_MAKES_CROSSINGS = 1,
		FLAG_WANTS_CROSSINGS = 2,
		FLAG_LAST_BASE = 128
	};

	RangeListNode( uint16 flags, uint16 order ) :
		pPrevX_( NULL ),
		pNextX_( NULL ),
		pPrevZ_( NULL ),
		pNextZ_( NULL ),
		flags_( flags ),
		order_( order )
	{ }
	virtual ~RangeListNode()	{}

	//oldZ is used to make the shuffle think entity is moving in orthogonal directions only
	void shuffleXThenZ( float oldX, float oldZ );

	void shuffleX( float oldX, float oldZ );
	void shuffleZ( float oldX, float oldZ );

	RangeListNode* prevX() const	{ return pPrevX_; }
	RangeListNode* prevZ() const	{ return pPrevZ_; }
	RangeListNode* nextX() const	{ return pNextX_; }
	RangeListNode* nextZ() const	{ return pNextZ_; }

	RangeListNode * getNeighbour( bool getNext, bool getZ ) const
	{
		return getNext ?
			(getZ ? pNextZ_ : pNextX_) :
			(getZ ? pPrevZ_ : pPrevX_);
	}

	void prevX( RangeListNode* rln )	{ pPrevX_ = rln; }
	void prevZ( RangeListNode* rln )	{ pPrevZ_ = rln; }
	void nextX( RangeListNode* rln )	{ pNextX_ = rln; }
	void nextZ( RangeListNode* rln )	{ pNextZ_ = rln; }

	uint16 flags() const 				{ return flags_; }
	uint16 order() const 				{ return order_; }

	bool isEntity() const	{ return flags_ & FLAG_MAKES_CROSSINGS; }

	void removeFromRangeList();
	void insertBeforeX( RangeListNode* entry );
	void insertBeforeZ( RangeListNode* entry );

	virtual void  debugRangeX() const;
	virtual void  debugRangeZ() const;
	virtual std::string debugString() const	{ return std::string( "Base" ); };

	float getCoord( bool getZ ) const { return getZ ? this->z() : this->x(); }

	virtual float x() const = 0;
	virtual float z() const = 0;

	virtual void crossedX( RangeListNode * /*node*/, bool /*positiveCrossing*/,
		float /*oldOthX*/, float /*oldOthZ*/ ) {}
	virtual void crossedZ( RangeListNode * /*node*/, bool /*positiveCrossing*/,
		float /*oldOthX*/, float /*oldOthZ*/ ) {}

protected:
	//pointers to the prev and next entities in the X and Z direction
	RangeListNode *pPrevX_;
	RangeListNode *pNextX_;
	RangeListNode *pPrevZ_;
	RangeListNode *pNextZ_;
	uint16			flags_;
	uint16			order_;
};


/**
 *	This class is used for the terminators of the range list. They are either
 *	the head or tail of the list. They always have a position of +/- FLT_MAX.
 */
class RangeListTerminator: public RangeListNode
{
public:
	RangeListTerminator( bool isHead ) :
		RangeListNode( 0, isHead ? 0 : uint16(-1)) {}
	float x() const { return (order_ ? FLT_MAX : -FLT_MAX); }
	float z() const { return (order_ ? FLT_MAX : -FLT_MAX); }
	std::string debugString() const { return order_ ? "Tail" : "Head"; }
};


/**
 *	This class is used for range triggers (traps). Its position is the same as
 *	the entity's position plus a range. Once another entity crosses the node, it
 *	will either trigger or untrigger it and it will notify its owner entity.
 */
class RangeTriggerNode: public RangeListNode
{
public:
	enum Flags
	{
		FLAG_LOWER_BOUND = FLAG_LAST_BASE * 2
	};

	RangeTriggerNode( RangeTrigger * pTriggerBase,
		float range, bool positive );

	float x() const;
	float z() const;
	virtual std::string debugString() const;

	float range() const			{ return range_; }
	void range( float r )		{ range_ = r; }		// don't call this
	void setRange( float range );					// call this instead

	float oldRange() const		{ return oldRange_; }

	virtual void crossedX( RangeListNode * node, bool positiveCrossing,
		float oldOthX, float oldOthZ );
	virtual void crossedZ( RangeListNode * node, bool positiveCrossing,
		float oldOthX, float oldOthZ );

	void initialShuffle( float oldX, float oldZ );

protected:
	RangeTrigger* pTriggerBase_;
	float range_;
	float oldRange_;
};


/**
 * This class encapsulates a full range trigger. It contains a upper and lower
 * bound trigger node.
 */
class RangeTrigger
{
public:
	RangeTrigger( RangeListNode * pSubject, float range );
	virtual ~RangeTrigger();

	void insert();
	void remove();
	void removeWithoutContracting();

	void shuffleXThenZ( float oldX, float oldZ );
	void shuffleXThenZExpand( bool xInc, bool zInc, float oldX, float oldZ );
	void shuffleXThenZContract( bool xInc, bool zInc, float oldX, float oldZ );

	void setRange( float range );

	virtual std::string debugString() const;

	virtual void triggerEnter( RangeListNode * who ) = 0;
	virtual void triggerLeave( RangeListNode * who ) = 0;

	bool contains( RangeListNode * pQuery ) const;
	bool containsInZ( RangeListNode * pQuery ) const;

	RangeListNode * pSubject() const					{ return pSubject_; }
	const RangeListNode * pUpperTrigger() const		{ return &upperBound_; }
	const RangeListNode * pLowerTrigger() const		{ return &lowerBound_; }

	bool wasInXRange( float x, float range ) const
	{
		float subX = oldX_;
		// These volatile values are important. If this is not done, the
		// calculation may be done with greater precision than what was used in
		// calculating the nodes position in the range list. This may cause
		// some triggers to be missed or to occur when they should not have.
		volatile float lowerBound = subX - range;
		volatile float upperBound = subX + range;
		return (lowerBound < x) && (x <= upperBound);
	}
	bool isInXRange( float x, float range ) const
	{
		float subX = pSubject_->x();
		// These volatile values are important. If this is not done, the
		// calculation may be done with greater precision than what was used in
		// calculating the nodes position in the range list. This may cause
		// some triggers to be missed or to occur when they should not have.
		volatile float lowerBound = subX - range;
		volatile float upperBound = subX + range;
		return (lowerBound < x) && (x <= upperBound);
	}
	bool wasInZRange( float z, float range ) const
	{
		float subZ = oldZ_;
		// These volatile values are important. If this is not done, the
		// calculation may be done with greater precision than what was used in
		// calculating the nodes position in the range list. This may cause
		// some triggers to be missed or to occur when they should not have.
		volatile float lowerBound = subZ - range;
		volatile float upperBound = subZ + range;
		return (lowerBound < z) && (z <= upperBound);
	}
	bool isInZRange( float z, float range ) const
	{
		float subZ = pSubject_->z();
		// These volatile values are important. If this is not done, the
		// calculation may be done with greater precision than what was used in
		// calculating the nodes position in the range list. This may cause
		// some triggers to be missed or to occur when they should not have.
		volatile float lowerBound = subZ - range;
		volatile float upperBound = subZ + range;
		return (lowerBound < z) && (z <= upperBound);
	}

	float range() const	{ return upperBound_.range(); }
protected:
	RangeListNode *		pSubject_;
	RangeTriggerNode	upperBound_;
	RangeTriggerNode	lowerBound_;
public:
	float				oldX_;
	float				oldZ_;
};

#ifdef _WIN32
#pragma warning (disable:4355)
#endif

#endif // RANGE_LIST_NODE_HPP
