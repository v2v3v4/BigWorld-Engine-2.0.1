/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEED_EXTRA_HPP
#define SPEED_EXTRA_HPP

#include "cellapp/controller.hpp"
#include "cellapp/entity_extra.hpp"

#include "speed_controller.hpp"

/**
 * Simple example entity extra... can print a message to the screen.
 */

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_ENTITY_EXTRA_WITH_DOC


/**
 *	This is an example EntityExtra that exposes the entity's speed via the
 *	Python API.
 */
class SpeedExtra : public EntityExtra
{
	Py_EntityExtraHeader( SpeedExtra );

public:
	SpeedExtra( Entity & e );
	~SpeedExtra();

	void onMovement( const Vector3 & oldPosition );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	void writeToStream( BinaryOStream & stream );
	void readFromStream( BinaryIStream & stream );

	PY_AUTO_METHOD_DECLARE( RETOK, trackSpeed, ARG( bool, END ) );
	bool trackSpeed( bool enable );

	PY_RO_ATTRIBUTE_DECLARE( speed_, mySpeed );
	PY_RO_ATTRIBUTE_DECLARE( velocity_, myVelocity );

	void setController( SpeedController * pController );

	float speed() const						{	return speed_; }
	const Vector3 & velocity() const		{	return velocity_; }

	static const Instance<SpeedExtra> instance;

private:
	bool isEnabled() const	{ return pController_ != NULL; }

	float speed_;
	Vector3 velocity_;
	uint64 lastTime_;

	SpeedController * pController_;
};

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC

#endif // SPEED_EXTRA_HPP
