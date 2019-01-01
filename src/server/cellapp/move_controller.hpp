/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOVE_CONTROLLER_HPP
#define MOVE_CONTROLLER_HPP

#include "controller.hpp"
#include "network/basictypes.hpp"
#include "updatable.hpp"

typedef SmartPointer< Entity > EntityPtr;

/**
 *	Abstract base class from which other controllers are derived.
 *	It just knows how to move towards a given destination by a given
 *	velocity in metres per second.
 */
class MoveController : public Controller, public Updatable
{
public:
	MoveController( float velocity, bool faceMovement = true,
		bool moveVertically = false );

	virtual void		startReal( bool isInitialStart );
	virtual void		stopReal( bool isFinalStop );

	bool				move( const Position3D & destination );

	void				writeRealToStream( BinaryOStream & stream );
	bool 				readRealFromStream( BinaryIStream & stream );

protected:
	bool faceMovement() const				{ return faceMovement_; }
	bool moveVertically() const				{ return moveVertically_; }

private:
	float				metresPerTick_;
	bool faceMovement_;
	bool moveVertically_;
};



/**
 *	This controller moves an entity towards a given point. It registers itself
 *	to be called every game tick. The velocity is measured in metres per second.
 */
class MoveToPointController : public MoveController
{
	DECLARE_CONTROLLER_TYPE( MoveToPointController )

public:
	MoveToPointController(const Position3D & destination = Position3D(0, 0, 0),
		const std::string & destinationChunk = "",
		int32 destinationWaypoint = 0,
		float velocity = 0.0f,
		bool faceMovement = true,
		bool moveVertically = false );

	void				writeRealToStream( BinaryOStream & stream );
	bool 				readRealFromStream( BinaryIStream & stream );
	void				update();

private:
	Position3D			destination_;
	std::string			destinationChunk_;
	int32				destinationWaypoint_;
};


/**
 * 	This controller moves an entity towards a given entity. It will stop
 * 	when it gets within a certain range of the entity.
 */
class MoveToEntityController : public MoveController
{
	DECLARE_CONTROLLER_TYPE( MoveToEntityController )

public:
	MoveToEntityController( EntityID destEntityID = 0,
			float velocity = 0.0f,
			float range = 0.0f,
			bool faceMovement = true,
			bool moveVertically = false );

	virtual void		stopReal( bool isFinalStop );
	void				writeRealToStream( BinaryOStream & stream );
	bool 				readRealFromStream( BinaryIStream & stream );
	void				update();

private:

	void				recalcOffset();

	EntityID			destEntityID_;
	EntityPtr			pDestEntity_;
	Position3D			offset_;
	float				range_;
};

#endif // MOVE_CONTROLLER_HPP
