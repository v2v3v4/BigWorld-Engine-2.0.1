/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PASSENGERS_HPP
#define PASSENGERS_HPP

#include "entity_extra.hpp"

#include "cstdmf/aligned.hpp"


/**
 *	This class manages the passengers that are riding on this entity. It extends
 *	entities that are vehicles, moving platforms, etc.
 */
class Passengers : public EntityExtra, public Aligned
{
public:
	Passengers( Entity & e );
	~Passengers();

	bool add( Entity & entity );
	bool remove( Entity & entity );

	void updateInternalsForNewPosition( const Vector3 & oldPosition );

	const Matrix & transform() const	{ return transform_; }

	static const Instance<Passengers> instance;

private:
	Passengers( const Passengers& );
	Passengers& operator=( const Passengers& );

	void adjustTransform();

	typedef std::vector< Entity * > Container;

	bool inDestructor_;

	Container passengers_;
	Matrix transform_;
};



#endif // PASSENGERS_HPP
