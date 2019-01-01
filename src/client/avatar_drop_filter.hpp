/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#ifndef AVATAR_DROP_FILTER_HPP
#define AVATAR_DROP_FILTER_HPP


#include "avatar_filter.hpp"
#include "entity.hpp"

class GroundNormalProvider;

/*~ class BigWorld.AvatarDropFilter
 *
 *	This class inherits from AvatarFilter.  It is nearly exactly the same
 *	as its parent, except that a findDropPoint is used to place each input 
 *	point on the ground.
 *
 *	An AvatarDropFilter is created using the BigWorld.AvatarDropFilter
 *	function.
 */
/**
 *	This is a specialised AvatarFilter that places its owner on the ground.
 *	It is intended for use with  navigating entities which have a tendency
 *	to hover above the ground as they follow the navigation mesh. \n\n
 *	
 *	@note	Currently the dropping is performed on input rather than output
 *			because it is less frequent and so cheaper. However this does
 *			mean that entities will clip through hills during periods of sparse
 *			input.
 */
class AvatarDropFilter : public AvatarFilter
{
	Py_Header( AvatarDropFilter, AvatarFilter )

public:
	AvatarDropFilter( PyTypePlus * pType = &s_type_ );
	AvatarDropFilter( const AvatarFilter & filter, PyTypePlus * pType = &s_type_ );
	~AvatarDropFilter();

	virtual void input(
						double time,
						SpaceID spaceID,
						EntityID vehicleID,
						const Position3D & pos,
						const Vector3 & posError,
						float * auxFiltered );

	void output( double time );

	static void drawDebugStuff();
	Entity * entity() { return entity_; }

	PY_FACTORY_DECLARE();

	PY_RW_ATTRIBUTE_DECLARE( alignToGround_, alignToGround )
	PY_RO_ATTRIBUTE_DECLARE( groundNormalProvider(), groundNormal );

	const Vector3 & groundNormal() const;
	PyObjectPtr groundNormalProvider();
	void onGroundNormalProviderDestroyed();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

private:
	bool alignToGround_;
	Vector3 groundNormal_;
	GroundNormalProvider * groundNormalProvider_;
};




#endif // AVATAR_DROP_FILTER_HPP

// avatar_drop_filter.hpp
