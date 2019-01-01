/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PASSENGER_EXTRA_HPP
#define PASSENGER_EXTRA_HPP

#include "entity_extra.hpp"

class PassengerController;


/// This macro defines the attribute function for a method
#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_ENTITY_EXTRA_WITH_DOC

/**
 *	This class manages the functionality related to an entity being a passenger.
 */
class PassengerExtra : public EntityExtra
{
	Py_EntityExtraHeader( PassengerExtra )

public:
	PassengerExtra( Entity & e );
	~PassengerExtra();

	void setController( PassengerController * pController );

	void onVehicleGone();

	static const Instance<PassengerExtra> instance;

	bool boardVehicle( EntityID vehicleID, bool shouldUpdateClient = true );
	bool alightVehicle( bool shouldUpdateClient = true );

	static bool isInLimbo( Entity & e );

	virtual void relocated();

protected:
	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_METHOD_DECLARE( RETOK, boardVehicle, ARG( EntityID, END ) );
	PY_AUTO_METHOD_DECLARE( RETOK, alightVehicle, END );

private:
	PassengerExtra( const PassengerExtra& );
	PassengerExtra& operator=( const PassengerExtra& );

	PassengerController * pController_;
};

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC


#endif // PASSENGER_EXTRA_HPP
