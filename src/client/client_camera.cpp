/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "client_camera.hpp"

#include "resmgr/datasection.hpp"
#include "camera/base_camera.hpp"
#include "camera/projection_access.hpp"
#include "camera/free_camera.hpp"
#include "camera/cursor_camera.hpp"
#include "moo/render_context.hpp"
#include "romp/geometrics.hpp"
#include "client/app.hpp"
#include "entity_picker.hpp"
#include "player.hpp"

#ifndef CODE_INLINE
#include "client_camera.ipp"
#endif

inline float roundf( float value, int decimals )
{
	float multiplier = powf( 10.0f, float( decimals ) );
	return floorf( value * multiplier + 0.5f ) / multiplier;
}

DECLARE_DEBUG_COMPONENT2( "Targeting", 0 );

// -----------------------------------------------------------------------------
// Section: AutoAim
// -----------------------------------------------------------------------------

AutoAim* AutoAim::pInstance_ = NULL;

AutoAim& AutoAim::instance()
{
	if (pInstance_ == NULL)
		pInstance_ = new AutoAim();
	return *pInstance_;
}

PY_SCRIPT_CONVERTERS( AutoAim )

void AutoAim::initInstance()
{
	AutoAim::instance();
}

/*~	class BigWorld.AutoAim
 *
 *	AutoAim is made up of essentially two components.  The first is friction and defines to
 *	what degree a Camera will 'stick' to its target and the second is adhesion, which describes
 *	to what degree the target reticle (eg, crosshairs, target box, etc) will follow the target.
 *	Each has a field of view where the friction and adhesion will be the maximum specified and
 *	a falloff field of view, where the values linearly blend to 0.
 *
 *	The object is contained within the BigWorld.Targeting object. Defaults for any number of
 *	values can be provided in engine_config.xml under &lt;targeting&gt;/&lt;autoAim&gt;, eg;
 *
 *	@{
 *	&lt;root&gt;
 *		...
 *		&lt;targeting&gt;
 *			...
 *			&lt;autoAim&gt;
 *				...
 *				&lt;friction&gt; .5 &lt;/friction&gt;
 *				...
 *			&lt;/autoAim&gt;
 *			...
 *		&lt;/targeting&gt;
 *		...
 *	&lt;/root&gt;
 *	@{
 *
 *	AutoAim Object can be accessed through BigWorld.autoAim function.
 *	Note that this is only valid for hand-held controllers...
 */
PY_TYPEOBJECT( AutoAim )

PY_BEGIN_METHODS( AutoAim )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( AutoAim )

	/*~ attribute AutoAim.friction
	 *
	 *	This describes to what degree the Camera will 'stick' to a target.  It is on a scale from 0.0 to 1.0.
	 *	A value of 0.0 would mean there was no friction at all, so the camera would move freely when a target
	 *	is present and a value of 1.0 would mean that the camera would lock onto the target until it reaches the
	 *	friction falloff angle/distance.  Defaults to 0.5.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( friction )

	/*~ attribute AutoAim.forwardAdhesion
	 *
	 *	This describes to what degree the target reticle will 'stick' to a target as the targeter moves forward.
	 *	The value is between 0.0 and 1.0. 0 would mean the target reticle is not affected, whereas 1 would cause
	 *	the target reticle to follow the target until it slides out of view.  Defaults to 0.05.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( forwardAdhesion )

	/*~ attribute AutoAim.strafeAdhesion
	 *
	 *	This describes to what degree the target reticle will 'stick' to a target as the targeter moves sideways.
	 *	The value is between 0.0 and 1.0. 0 would mean the target reticle is not affected, whereas 1 would cause
	 *	the target reticle to follow the target until it slides out of view.  Defaults to 0.2.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( strafeAdhesion )

	/*~ attribute AutoAim.turnAdhesion
	 *
	 *	This describes to what degree the target reticle will 'stick' to a target as the targeter turns.
	 *	The value is between 0.0 and 1.0. 0 would mean the target reticle is not affected, whereas 1 would cause
	 *	the target reticle to follow the target until it slides out of view.  Defaults to 0.1.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( turnAdhesion )

	/*~ attribute AutoAim.adhesionPitchToYawRatio
	 *
	 *	This value is used in calculations to handle pitch and yaw.  Defaults to 0.5.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionPitchToYawRatio )

	/*~ attribute AutoAim.reverseAdhesionStyle
	 *
	 *	Adjusts the forwardAdhesion for backwards movement.  The default, -1 will perform the usually desired
	 *	effect, 0 will deactivate it.  Other values will adjust it proportionally.
	 *
	 *	@type int
	 */
	PY_ATTRIBUTE( reverseAdhesionStyle )

	/*~ attribute AutoAim.frictionHorizontalAngle
	 *
	 *	This attribute specifies the left and right field of view angles, in radians, within which the friction
	 *	will be the maximum specified by friction.
	 *
	 *	@type int
	 */
	PY_ATTRIBUTE( frictionHorizontalAngle )

	/*~ attribute AutoAim.frictionVerticalAngle
	 *
	 *	This attribute specifies the upper and lower field of view angles, in radians, within which the friction
	 *	will be maximum specified by friction.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionVerticalAngle )

	/*~ attribute AutoAim.frictionDistance
	 *
	 *	This attribute specifies the distance within which the friction will be maximum specified by friction.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionDistance )

	/*~ attribute AutoAim.frictionHorizontalFalloffAngle
	 *
	 *	This attribute specifies the left and right field of view angles, in radians, outside which the friction
	 *	will be 0.  The friction will linearly deplete from frictionHorizontalAngle to frictionHorizontalFalloffAngle.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionHorizontalFalloffAngle )

	/*~ attribute AutoAim.frictionVerticalFalloffAngle
	 *
	 *	This attribute specifies the upper and lower field of view angles, in radians, outside which the friction
	 *	will be 0.  The friction will linearly deplete from frictionVerticalAngle to frictionVerticalFalloffAngle.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionVerticalFalloffAngle )

	/*~ attribute AutoAim.frictionMinimumDistance
	 *
	 *	This attribute specifies the distance before friction will come into effect,
	 *	ie, if the target is too close, 0 friction will be used...
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionMinimumDistance )

	/*~ attribute AutoAim.frictionFalloffDistance
	 *
	 *	This attribute specifies the distance outside which the friction will be 0.
	 *	The friction will linearly deplete from frictionDistance to frictionFalloffDistance.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( frictionFalloffDistance )

	/*~ attribute AutoAim.adhesionHorizontalAngle
	 *
	 *	This attribute specifies the left and right field of view angles, in radians, within which the adhesion
	 *	will be the maximum specified for the current movement.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionHorizontalAngle )

	/*~ attribute AutoAim.adhesionVerticalAngle
	 *
	 *	This attribute specifies the upper and lower field of view angles, in radians, within which the adhesion
	 *	will be the maximum specified for the current movement.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionVerticalAngle )

	/*~ attribute AutoAim.adhesionDistance
	 *
	 *	This attribute specifies the distance within which the adhesion
	 *	will be the maximum specified for the current movement.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionDistance )

	/*~ attribute AutoAim.adhesionHorizontalFalloffAngle
	 *
	 *	This attribute specifies the left and right field of view angles, in radians, outside which the adhesion
	 *	will be 0.  The adhesion will linearly deplete from adhesionHorizontalAngle to adhesionHorizontalFalloffAngle.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionHorizontalFalloffAngle )

	/*~ attribute AutoAim.adhesionVerticalFalloffAngle
	 *
	 *	This attribute specifies the upper and lower field of view angles, in radians, outside which the adhesion
	 *	will be 0.  The adhesion will linearly deplete from adhesionVerticalAngle to adhesionVerticalFalloffAngle.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionVerticalFalloffAngle )

	/*~ attribute AutoAim.adhesionFalloffDistance
	 *
	 *	This attribute specifies the distance outside which the adhesion will be 0.
	 *	The adhesion will linearly deplete from adhesionDistance to adhesionFalloffDistance.
	 *
	 *	@type float
	 */
	PY_ATTRIBUTE( adhesionFalloffDistance )

PY_END_ATTRIBUTES()



/**
 *	Constructor.
 */
AutoAim::AutoAim( PyTypePlus * pType ) :
	PyObjectPlus( pType )
{
	BW_GUARD;
	friction_ = 0.5f;
	forwardAdhesion_ = 0.05f;
	strafeAdhesion_ = 0.2f;
	turnAdhesion_ = 0.1f;
	adhesionPitchToYawRatio_ = 0.5f;
	reverseAdhesionStyle_ = -1;

	MF_WATCH( "Targeting/Friction", friction_ );
	MF_WATCH( "Targeting/Forward Adhesion", forwardAdhesion_ );
	MF_WATCH( "Targeting/Strafe Adhesion", strafeAdhesion_ );
	MF_WATCH( "Targeting/Turn Adhesion", turnAdhesion_ );
	MF_WATCH( "Targeting/Adhesion Pitch::Yaw Ratio", adhesionPitchToYawRatio_ );
	MF_WATCH( "Targeting/Reverse Adhesion Style", reverseAdhesionStyle_ );
}

/**
 *	readInitialValues: gets the starting setup from the XML file.
 */
void AutoAim::readInitialValues( DataSectionPtr rootSection )
{
	BW_GUARD;
	DataSectionPtr section = rootSection->openSection( "autoAim" );

	if (section) {
		friction_ = section->readFloat( "friction", 0.5f );
		forwardAdhesion_ = section->readFloat( "forwardAdhesion", forwardAdhesion_ );
		strafeAdhesion_ = section->readFloat( "strafeAdhesion", strafeAdhesion_ );
		turnAdhesion_ = section->readFloat( "turnAdhesion", turnAdhesion_ );
		adhesionPitchToYawRatio_ = section->readFloat( "adhesionPitchToYawRatio", adhesionPitchToYawRatio_ );
		reverseAdhesionStyle_ = section->readInt( "reverseAdhesionStyle", reverseAdhesionStyle_ );

		EntityPicker& picker = EntityPicker::instance();
		picker.autoAimFrictionHorizontalAngle( DEG_TO_RAD( section->readFloat( "frictionHorizontalAngle",
															RAD_TO_DEG( picker.autoAimFrictionHorizontalAngle() ) ) ) );
		picker.autoAimFrictionVerticalAngle( DEG_TO_RAD( section->readFloat( "frictionVerticalAngle",
															RAD_TO_DEG( picker.autoAimFrictionVerticalAngle() ) ) ) );
		picker.autoAimFrictionDistance( section->readFloat( "frictionDistance",
															picker.autoAimFrictionDistance() ) );
		picker.autoAimFrictionHorizontalFalloffAngle( DEG_TO_RAD( section->readFloat( "frictionHorizontalFalloffAngle",
															RAD_TO_DEG( 2.0f * picker.autoAimFrictionHorizontalAngle() ) ) ) );
		picker.autoAimFrictionVerticalFalloffAngle( DEG_TO_RAD( section->readFloat( "frictionVerticalFalloffAngle",
															RAD_TO_DEG( 2.0f * picker.autoAimFrictionVerticalAngle() ) ) ) );
		picker.autoAimFrictionMinimumDistance( section->readFloat( "frictionMinimumDistance",
															0.0f ) );
		picker.autoAimFrictionFalloffDistance( section->readFloat( "frictionFalloffDistance",
															2.0f * picker.autoAimFrictionDistance() ) );

		picker.autoAimAdhesionHorizontalAngle( DEG_TO_RAD( section->readFloat( "adhesionHorizontalAngle",
															RAD_TO_DEG( picker.autoAimAdhesionHorizontalAngle() ) ) ) );
		picker.autoAimAdhesionVerticalAngle( DEG_TO_RAD( section->readFloat( "adhesionVerticalAngle",
															RAD_TO_DEG( picker.autoAimAdhesionVerticalAngle() ) ) ) );
		picker.autoAimAdhesionDistance( section->readFloat( "adhesionDistance",
															picker.autoAimAdhesionDistance() ) );
		picker.autoAimAdhesionHorizontalFalloffAngle( DEG_TO_RAD( section->readFloat( "adhesionHorizontalFalloffAngle",
															RAD_TO_DEG( 2.0f * picker.autoAimAdhesionHorizontalAngle() ) ) ) );
		picker.autoAimAdhesionVerticalFalloffAngle( DEG_TO_RAD( section->readFloat( "adhesionVerticalAngle",
															RAD_TO_DEG( 2.0f * picker.autoAimAdhesionVerticalAngle() ) ) ) );
		picker.autoAimAdhesionFalloffDistance( section->readFloat( "adhesionDistance",
															2.0f * picker.autoAimAdhesionDistance() ) );
	}
}


//////////
// Python attribute methods
//////////
PyObject * AutoAim::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

int AutoAim::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

float AutoAim::frictionHorizontalAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimFrictionHorizontalAngle() ), 4 );
}

void AutoAim::frictionHorizontalAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionHorizontalAngle(DEG_TO_RAD( angle ));
}

float AutoAim::frictionVerticalAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimFrictionVerticalAngle() ), 4 );
}

void AutoAim::frictionVerticalAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionVerticalAngle(DEG_TO_RAD( angle ));
}

float AutoAim::frictionDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().autoAimFrictionDistance();
}

void AutoAim::frictionDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionDistance(distance);
}

float AutoAim::frictionHorizontalFalloffAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimFrictionHorizontalFalloffAngle() ), 4 );
}

void AutoAim::frictionHorizontalFalloffAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionHorizontalFalloffAngle(DEG_TO_RAD( angle ));
}

float AutoAim::frictionVerticalFalloffAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimFrictionVerticalFalloffAngle() ), 4 );
}

void AutoAim::frictionVerticalFalloffAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionVerticalFalloffAngle(DEG_TO_RAD( angle ));
}

float AutoAim::frictionMinimumDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().autoAimFrictionMinimumDistance();
}

void AutoAim::frictionMinimumDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionMinimumDistance(distance);
}

float AutoAim::frictionFalloffDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().autoAimFrictionFalloffDistance();
}

void AutoAim::frictionFalloffDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().autoAimFrictionFalloffDistance(distance);
}

float AutoAim::adhesionHorizontalAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimAdhesionHorizontalAngle() ), 4 );
}

void AutoAim::adhesionHorizontalAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionHorizontalAngle(DEG_TO_RAD( angle ));
}

float AutoAim::adhesionVerticalAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimAdhesionVerticalAngle() ), 4 );
}

void AutoAim::adhesionVerticalAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionVerticalAngle(DEG_TO_RAD( angle ));
}

float AutoAim::adhesionDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().autoAimAdhesionDistance();
}

void AutoAim::adhesionDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionDistance(distance);
}

float AutoAim::adhesionHorizontalFalloffAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimAdhesionHorizontalFalloffAngle() ), 4 );
}

void AutoAim::adhesionHorizontalFalloffAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionHorizontalFalloffAngle(DEG_TO_RAD( angle ));
}

float AutoAim::adhesionVerticalFalloffAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().autoAimAdhesionVerticalFalloffAngle() ), 4 );
}

void AutoAim::adhesionVerticalFalloffAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionVerticalFalloffAngle(DEG_TO_RAD( angle ));
}

float AutoAim::adhesionFalloffDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().autoAimAdhesionFalloffDistance();
}

void AutoAim::adhesionFalloffDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().autoAimAdhesionFalloffDistance(distance);
}

// -----------------------------------------------------------------------------
// Section: Targeting
// -----------------------------------------------------------------------------

Targeting* Targeting::pInstance_ = NULL;

Targeting& Targeting::instance()
{
	if (pInstance_ == NULL)
		pInstance_ = new Targeting();
	return *pInstance_;
}

/*~ attribute BigWorld.targeting
 *	Stores a Targeting object, which is used to configure the BigWorld Targeting System.
 */
PY_MODULE_ATTRIBUTE( BigWorld, targeting, &Targeting::instance() )

void Targeting::initInstance()
{
	Targeting::instance();
}

/*~	class BigWorld.Targeting
 *	This class manages the field of view in which an Entity can become a potential target for any given client.
 *	Default values can be specified in engine_config.xml in the &lt;targeting&gt; data section.
 *	See BigWorld.PyTarget to see how potential targets are used to select the final target.
 */
PY_TYPEOBJECT( Targeting )

PY_BEGIN_METHODS( Targeting )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Targeting )

	/*~	attribute Targeting.selectionAngle
	 *	This is the field of view angle, in degrees, at which an Entity will become a potential target.
	 *	Lower values will provide a tighter fov, whilst higher values will capture more targets.
	 *	@type	float
	 */
	PY_ATTRIBUTE( selectionAngle )

	/*~	attribute Targeting.deselectionAngle
	 *	This is the angle, in degrees, in the field of view at which a potential target will no longer be
	 *	considered viable.  Lower values will provide a tighter fov, whilst higher values will keep more targets.
	 *	@type	float
	 */
	PY_ATTRIBUTE( deselectionAngle )

	/*~	attribute Targeting.selectionDistance
	 *	The distance at which an Entity can form a potential target.
	 *	@type	float
	 */
	PY_ATTRIBUTE( selectionDistance )

	/*~	attribute Targeting.defaultSelectionAngle
	 *	This is the default field of view angle, in degrees, at which an Entity will become a potential target.
	 *	@type	float
	 */
	PY_ATTRIBUTE( defaultSelectionAngle )

	/*~	attribute Targeting.defaultDeselectionAngle
	 *	This is the default angle, in degrees, in the field of view at which a potential target will
	 *	no longer be considered viable.
	 *	@type	float
	 */
	PY_ATTRIBUTE( defaultDeselectionAngle )

	/*~	attribute Targeting.defaultSelectionDistance
	 *	The default distance at which an Entity can form a potential target.
	 *	@type	float
	 */
	PY_ATTRIBUTE( defaultSelectionDistance )

	/*~	attribute Targeting.autoAimOn
	 *	If true, AutoAim settings will be used when a target is in view, otherwise,
	 *	camera behaviour will be as normal, regardless of current target status.
	 *	@type	boolean (0 or 1)
	 */
	PY_ATTRIBUTE( autoAimOn )

	/*~	attribute Targeting.autoAim
	 *	This attribute provides access to the BigWorld AutoAim object, which
	 *	is used to define how a camera will behave when a target is in view.
	 *	@type	AutoAim
	 */
	PY_ATTRIBUTE( autoAim )

PY_END_ATTRIBUTES()



/**
 *	Constructor.
 */
Targeting::Targeting( PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	autoAimOn_(false),
	autoAim_(AutoAim::instance())
{
}

/**
 *	readInitialValues: gets the starting setup from the XML file.
 */
void Targeting::readInitialValues( DataSectionPtr rootSection )
{
	BW_GUARD;
	DataSectionPtr section = rootSection->openSection( "targeting" );

	if (section) {
		selectionAngle( section->readFloat( "selectionAngle", selectionAngle() ) );
		deselectionAngle( section->readFloat( "deselectionAngle", deselectionAngle() ) );
		selectionDistance( section->readFloat( "selectionDistance", selectionDistance() ) );

		EntityPicker& picker = EntityPicker::instance();
		picker.defaultSelectionFoV( picker.selectionFoV() );
		picker.defaultDeselectionFoV( picker.deselectionFoV() );
		picker.defaultSelectionDistance( picker.selectionDistance() );

		autoAim_.readInitialValues( section );
	}
}


//////////
// Python attribute methods
//////////
PyObject * Targeting::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

int Targeting::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

float Targeting::selectionAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().selectionFoV() / 2.0f ), 4 );
}

void Targeting::selectionAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().selectionFoV(DEG_TO_RAD( angle * 2.0f ));
}

float Targeting::deselectionAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().deselectionFoV() / 2.0f ), 4 );
}

void Targeting::deselectionAngle( float angle )
{
	BW_GUARD;
	EntityPicker::instance().deselectionFoV(DEG_TO_RAD( angle * 2.0f ));
}

float Targeting::selectionDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().selectionDistance();
}

void Targeting::selectionDistance( float distance )
{
	BW_GUARD;
	EntityPicker::instance().selectionDistance( distance );
}

float Targeting::defaultSelectionAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().defaultSelectionFoV() / 2.0f ), 4 );
}

float Targeting::defaultDeselectionAngle() const
{
	BW_GUARD;
	return roundf( RAD_TO_DEG( EntityPicker::instance().defaultDeselectionFoV() / 2.0f ), 4 );
}

float Targeting::defaultSelectionDistance() const
{
	BW_GUARD;
	return EntityPicker::instance().defaultSelectionDistance();
}

// -----------------------------------------------------------------------------
// Section: CameraSpeed
// -----------------------------------------------------------------------------

CameraSpeed* CameraSpeed::pInstance_ = NULL;

CameraSpeed& CameraSpeed::instance()
{
	if (pInstance_ == NULL)
		pInstance_ = new CameraSpeed();
	return *pInstance_;
}

/*~	attribute BigWorld.cameraSpeed
 *	Stores the CameraSpeed object which is used to describe how a camera can move.
 *  @type CameraSpeed.
 */
PY_MODULE_ATTRIBUTE( BigWorld, cameraSpeed, &CameraSpeed::instance() )

void CameraSpeed::initInstance()
{
	CameraSpeed::instance();
}

/*~	class BigWorld.CameraSpeed
 *	The CameraSpeed object stores current camera movement settings.  Defaults
 *	can be specified in engine_config.xml through the &lt;camera&gt; data section.
 *
 *	CameraSpeed object can be accessed through BigWorld.cameraSpeed attribute.
 */
PY_TYPEOBJECT( CameraSpeed )

PY_BEGIN_METHODS( CameraSpeed )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( CameraSpeed )

	/*~	attribute CameraSpeed.lookTable
	 *	This is a table that provides values for basic, non-linear camera movement.
	 *	Rather than just have the camera speed be a multiple of the axis control,
	 *	it is also multiplied by a number from this table.  The table is made up of
	 *	Vector2s, with the first item in the vector being an index, and the second
	 *	being the value.  The item in the table with the highest index less than the
	 *	current value of the axis control is used.  The value associated with that
	 *	item is used to mulitply the camera speed.
	 *
	 *	The table defaults to:
	 *	@{
	 *		( 0.00, 0.00 )
	 *		( 0.10, 0.05 )
	 *		( 0.30, 0.10 )
	 *		( 0.50, 0.25 )
	 *		( 0.70, 0.58 )
	 *		( 0.84, 1.00 )
	 *	@}
	 *	For example, An axis value of 0.55 would pick the 4th item, giving a value of 0.25.
	 *	The camera speed would then be:
	 *	@{
	 *		base camera speed * 0.55 * 0.25
	 *	@}
	 *	@type	list of tuple2
	 */
	PY_ATTRIBUTE( lookTable )

	/*~	attribute CameraSpeed.lookAxisMin
	 *	The minimum value used for camera control with axis oriented devices.  In addition,
	 *	if the position is less than the minimum, it is ignored (the deadzone).
	 *	If the position is more than the maximum, it is considered "pinned", i.e. at the extreme.
	 *	@type	float
	 */
	PY_ATTRIBUTE( lookAxisMin )

	/*~	attribute CameraSpeed.lookAxisMax
	 *	The maximum value used for camera control with axis oriented devices.  In addition,
	 *	if the position is less than the minimum, it is ignored (the deadzone).
	 *	If the position is more than the maximum, it is considered "pinned", i.e. at the extreme.
	 *	@type	float
	 */
	PY_ATTRIBUTE( lookAxisMax )

	/*~	attribute CameraSpeed.cameraYawSpeed
	 *	This is the basic turning speed (in degrees per second) for horizonal turning.
	 *	Horizontal turning can be accelerated.
	 *	@type	float
	 */
	PY_ATTRIBUTE( cameraYawSpeed )

	/*~	attribute CameraSpeed.cameraPitchSpeed
	 *	This is the basic turning speed (in degrees per second) for vertical turning.
	 *	@type	float
	 */
	PY_ATTRIBUTE( cameraPitchSpeed )

	/*~	attribute CameraSpeed.accelerateStart
	 *	The delay, in seconds, before acceleration can start.
	 *	@type	float
	 */
	PY_ATTRIBUTE( accelerateStart )

	/*~	attribute CameraSpeed.accelerateRate
	 *	Acceleration is linear, and the maximum speed multiplier is given by this value.
	 *	The speed increases linearly between the time acceleration starts and ends.
	 *	@type	float
	 */
	PY_ATTRIBUTE( accelerateRate )

	/*~	attribute CameraSpeed.accelerateEnd
	 *	This is the time in seconds at which acceleration stops, and the movement has reached it maximum speed.
	 *	@type	float
	 */
	PY_ATTRIBUTE( accelerateEnd )

	/*~	attribute CameraSpeed.accelerateWhileTargetting
	 *	If true, the camera speed is permitted to accelerate if targeting, otherwise it won't.  Defaults to false (0).
	 *	@type	boolean (0 or 1)
	 */
	PY_ATTRIBUTE( accelerateWhileTargetting )

	/*~	attribute CameraSpeed.turningHalfLife
	 *	The time the camera should take to move half the distance required to meet its objective.
	 *	If this is zero, there is no momentum, and the camera always point exactly where it's told.
	 *	A value of 1.0 would mean than each second the camera would get half-way to the desired direction.
	 *	A value of 0.5 would mean that each second the camera would get 70% of the wat to the desired direction.
	 *	Other values would be:
	 *	@{
	 *		0.25  84%
	 *		0.10  93%
	 *		0.08  95%
	 *		0.05  97%
	 *		0.03  98%
	 *		0.01  99%
	 *	@}
	 *	@type	float
	 */
	PY_ATTRIBUTE( turningHalfLife )

PY_END_ATTRIBUTES()


#pragma warning (disable:4355)

/**
 *	Constructor.
 */
CameraSpeed::CameraSpeed( PyTypePlus * pType ) :
	PyObjectPlus( pType ),
	lookTableHolder_( lookTable_, this, true ),
	accelerateWhileTargetting_(false)
{
	BW_GUARD;
	lookTable_.push_back( Vector2( 0.0f, 0.00f ) );
	lookTable_.push_back( Vector2( 0.1f, 0.05f ) );
	lookTable_.push_back( Vector2( 0.3f, 0.10f ) );
	lookTable_.push_back( Vector2( 0.5f, 0.25f ) );
	lookTable_.push_back( Vector2( 0.7f, 0.58f ) );
	lookTable_.push_back( Vector2( 0.84f, 1.00f ) );
	lookAxisMin_ = 0.1f;
	lookAxisMax_ = 0.85f;
	cameraYawSpeed_ = DEG_TO_RAD( 120 );
	cameraPitchSpeed_ = DEG_TO_RAD( 60 );
	accelerateStart_ = 1.0f;
	accelerateRate_ = 3.0f;
	accelerateEnd_ = 3.0f;
}

/**
 *	readInitialValues: gets the starting setup from the XML file.
 */
void CameraSpeed::readInitialValues( DataSectionPtr rootSection )
{
	BW_GUARD;
	DataSectionPtr section = rootSection->openSection( "camera" );

	if (section) {
		// Read in camera behaviour defaults.
		lookTable_.clear();
		section->readVector2s( "lookTable", lookTable_ );

		lookAxisMin_ = section->readFloat( "lookAxisMin", 0.1f );
		lookAxisMax_ = section->readFloat( "lookAxisMax", 0.85f );
		cameraYawSpeed_ = DEG_TO_RAD( float( section->readInt( "cameraYawSpeed", 120 ) ) );
		cameraPitchSpeed_ = DEG_TO_RAD( float( section->readInt( "cameraPitchSpeed", 60 ) ) );
		accelerateStart_ = section->readFloat( "accelerateStart", 1.0f );
		accelerateRate_ = section->readFloat( "accelerateRate", 3.0f );
		accelerateEnd_ = section->readFloat( "accelerateEnd", 3.0f );
	}
}


//////////
// Python attribute methods
//////////
PyObject * CameraSpeed::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

int CameraSpeed::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

float CameraSpeed::cameraYawSpeed() const
{
	return roundf( RAD_TO_DEG( cameraYawSpeed_ ), 4 );
}

void CameraSpeed::cameraYawSpeed( float speed )
{
	cameraYawSpeed_ = DEG_TO_RAD( speed );
}

float CameraSpeed::cameraPitchSpeed() const
{
	return roundf( RAD_TO_DEG( cameraPitchSpeed_ ), 4 );
}

void CameraSpeed::cameraPitchSpeed( float speed )
{
	cameraPitchSpeed_ = DEG_TO_RAD( speed );
}

float CameraSpeed::turningHalfLife() const
{
	BW_GUARD;
	BaseCameraPtr camera = ClientCamera::instance().camera();
	CursorCamera* pCamera = static_cast<CursorCamera*>(camera.getObject());
	return pCamera->turningHalfLife();
}

void CameraSpeed::turningHalfLife( float halfLife )
{
	BW_GUARD;
	BaseCameraPtr camera = ClientCamera::instance().camera();
	CursorCamera* pCamera = static_cast<CursorCamera*>(camera.getObject());
	pCamera->turningHalfLife( halfLife );
}


// -----------------------------------------------------------------------------
// Section: ClientSpeedProvider
// -----------------------------------------------------------------------------

ClientSpeedProvider* ClientSpeedProvider::pInstance_ = NULL;

ClientSpeedProvider& ClientSpeedProvider::instance()
{
	if (pInstance_ == NULL)
		pInstance_ = new ClientSpeedProvider();
	return *pInstance_;
}

void ClientSpeedProvider::initInstance()
{
	ClientSpeedProvider::instance();
}

/*static*/ void ClientSpeedProvider::fini()
{
	delete pInstance_;
	pInstance_ = NULL;
}

/**
 *	Constructor.
 */
ClientSpeedProvider::ClientSpeedProvider() :
	targeting_(Targeting::instance()),
	cameraSpeed_(CameraSpeed::instance()),
	target_(NULL),
	yawAccelerationTime_(0.0f),
	previousPitch_(0.0f),
	previousYaw_(0.0f),
	previousHadATarget_(false),
	yawStrafeSMA_( 4 ),
	yawTurningSMA_( 4 ),
	showTotalYaw_(false),
	showForwardYaw_(false),
	showStrafeYaw_(false),
	showTurnYaw_(false),
	showTotalPitch_(false),
	showFriction_(false),
	yawPitchScale_(5),
	numSamples_( kNumSamples ),
	lastFriction_(0.0f)
{
	BW_GUARD;
	friction_.resize( kNumSamples );
	yawChange_.resize( kNumSamples );
	yawForwardChange_.resize( kNumSamples );
	yawStrafeChange_.resize( kNumSamples );
	yawTurnChange_.resize( kNumSamples );
	pitchChange_.resize( kNumSamples );

	MF_WATCH( "Targeting/Show Total Yaw", showTotalYaw_ );
	MF_WATCH( "Targeting/Show Forward Yaw", showForwardYaw_ );
	MF_WATCH( "Targeting/Show Strafe Yaw", showStrafeYaw_ );
	MF_WATCH( "Targeting/Show Turn Yaw", showTurnYaw_ );
	MF_WATCH( "Targeting/Show Total Pitch", showTotalPitch_ );
	MF_WATCH( "Targeting/Yaw&Pitch Scale", yawPitchScale_ );
	MF_WATCH( "Targeting/Show Friction", showFriction_ );
	MF_WATCH( "Targeting/Number of Samples", numSamples_ );
}

/**
 *	readInitialValues: gets the starting setup from the XML file.
 */
void ClientSpeedProvider::readInitialValues( DataSectionPtr rootSection )
{
	BW_GUARD;
	targeting_.readInitialValues( rootSection );
	cameraSpeed_.readInitialValues( rootSection );
}

/**
 *	value: works out the speed based on the axis input.
 */
float ClientSpeedProvider::value( const AxisEvent &event )
{
	BW_GUARD;
	bool doingHorizontal = event.axis() == AxisEvent::AXIS_RX;

	// Work out an adjusted value, compensating somewhat for the direction
	Joystick& joystick = InputDevices::instance().joystick();
	AxisEvent::Axis otherAxisID = doingHorizontal ? AxisEvent::AXIS_RY : AxisEvent::AXIS_RX;
	const Joystick::Axis& otherAxis = joystick.getAxis( otherAxisID );

	float axisValue = fabsf( event.value() );
	float otherAxisValue = fabsf( otherAxis.value() );
	float adjustment = ( 1.0f  - axisValue ) * otherAxisValue * 1.25f;
	if (axisValue < 0.5f && axisValue < otherAxisValue)
		adjustment *= axisValue * 2.0f;

//if (otherAxisID == AxisEvent::AXIS_RY && axisValue != adjustment + adjustment) {
//	char message[256];
//	sprintf(message, "SimpleSpeedProvider::value() Adjusting from %g to %g (other axis = %g)\n",
//										axisValue, axisValue + adjustment, otherAxisValue);
//	OutputDebugString(message);
//}

	// Work out some basics
	axisValue = Math::clamp( cameraSpeed_.lookAxisMin_, axisValue + adjustment, cameraSpeed_.lookAxisMax_ );
	float lookValue = 1.0f;
	if (axisValue > cameraSpeed_.lookAxisMin_)
	{
		V2Vector::iterator last = cameraSpeed_.lookTable_.begin();
		for (V2Vector::iterator iter = cameraSpeed_.lookTable_.begin(); iter != cameraSpeed_.lookTable_.end(); ++iter)
		{
			if (iter->x >= axisValue)
				break;
			last = iter;
		}
		if (last != cameraSpeed_.lookTable_.end())
			lookValue = last->y;
	}
	else
		lookValue = 0.0f;

	if (event.value() < 0)
		axisValue = -axisValue;

	// Sticky target friction
	float friction = 0.0f;
//	EntityPicker& picker = EntityPicker::instance();
	float autoAimDistance = 0.0f;
	float autoAimAngle = 0.0f;
	if (targeting_.autoAimOn_)
		target_ = targeting_.hasAnAutoAimTarget( autoAimDistance, autoAimAngle, true, doingHorizontal );
	else
		target_ = NULL;
	if (target_)
	{
		float level = 1.0f;

		// The friction is full between 0 and fullDistance. After that it decreases to 0 at falloffDistance
		float fullDistance = targeting_.autoAim_.frictionDistance();
		float minimumDistance = targeting_.autoAim_.frictionMinimumDistance();
		float falloffDistance = targeting_.autoAim_.frictionFalloffDistance();
		if (autoAimDistance > fullDistance)
			level *= 1.0f - (autoAimDistance - fullDistance) / (falloffDistance - fullDistance);
		else if (autoAimDistance < minimumDistance && minimumDistance > 0.f)
			level *= autoAimDistance / minimumDistance;

		// The friction is full within the autoAimAngle. After that it decreases to 0 at falloffAngle
		float fullAngle = doingHorizontal ? targeting_.autoAim_.frictionHorizontalAngle()
											: targeting_.autoAim_.frictionVerticalAngle();
		float falloffAngle = doingHorizontal ? targeting_.autoAim_.frictionHorizontalFalloffAngle()
											: targeting_.autoAim_.frictionVerticalFalloffAngle();
		if (autoAimAngle > fullAngle)
			level *= 1.0f - (autoAimAngle - fullAngle) / (falloffAngle - fullAngle);

		friction = level * targeting_.autoAim_.friction_;
	}
	lastFriction_ = friction;

	float result = 0.0f;
	switch (event.axis())
	{
		case AxisEvent::AXIS_RX:
			if (lookValue == 0.0f) {
				yawAccelerationTime_ = 0.0f;
			}
			else
			{
				double timeNow = App::instance().getTime();
				float accelTime = yawAccelerationTime_ == 0.0 || lookValue < 1.0f ? 0.0f : yawAccelerationTime_;
				if (lookValue == 1.0f && (cameraSpeed_.accelerateWhileTargetting_ || target_ == NULL))
					yawAccelerationTime_ += event.dTime();

				float accel = accelTime < cameraSpeed_.accelerateStart_ ? 1.0f
								: 1.0f + (cameraSpeed_.accelerateRate_ - 1.0f ) *
										min( accelTime, cameraSpeed_.accelerateEnd_ ) /
											cameraSpeed_.accelerateEnd_;

				result = axisValue * lookValue * (1.0f - friction) * cameraSpeed_.cameraYawSpeed_ * accel;
			}
			break;

		case AxisEvent::AXIS_RY:
			result = axisValue * lookValue * (1.0f - friction) * cameraSpeed_.cameraPitchSpeed_;
			break;
	}

	return result;
}

void ClientSpeedProvider::adjustDirection( float dTime, Angle& pitch, Angle& yaw, Angle& roll )
{
	BW_GUARD;
	float autoAimDistance = 0.0f;
	float autoAimAngle = 0.0f;
//	EntityPicker& picker = EntityPicker::instance();
	if (targeting_.autoAimOn_)
		target_ = targeting_.hasAnAutoAimTarget( autoAimDistance, autoAimAngle, false, true );
	else
		target_ = NULL;

	float yawChange = 0.0f;
	float pitchChange = 0.0f;
	Angle forwardYawLevel = 0.0f;
	Angle strafingYawLevel = 0.0f;
	Angle turningYawLevel = 0.0f;

	if (previousHadATarget_ && target_ != NULL)
	{
		Entity* pPlayer = Player::instance().entity();
		Vector3 directionMoved = pPlayer->pPhysics()->movedDirection();
		float distanceMoved = pPlayer->pPhysics()->movedDistance();

		const Matrix& cameraToWorld = Moo::rc().invView();
		Vector3 cameraPosition = cameraToWorld.applyToOrigin();
		cameraPosition.y = pPlayer->position().y;
		Vector3 targetDelta = target_->position() - cameraPosition;
		float targetDistance = targetDelta.length();

		// Get the movement of the player, divided into forward and sideways movement
		Vector3 forward = cameraToWorld.applyToUnitAxisVector( Z_AXIS );

		Vector3 forwardMovement = directionMoved.projectOnto( forward );
		Vector3 strafeMovement = directionMoved - forwardMovement;
		float absForwardDistance = forwardMovement.length();
		float absStrafeDistance = strafeMovement.length();
		// Did we walk forwards or backwards?
		bool backwards = directionMoved.dotProduct( forward ) < 0.0f;

		// Base level, based on distance and angle
		float fullDistance = targeting_.autoAim_.adhesionDistance();
		float falloffDistance = targeting_.autoAim_.adhesionFalloffDistance();
		Angle baseLevel = 1.0f;
		if (targetDistance > falloffDistance)
			baseLevel = 0.0f;
		else if (targetDistance > fullDistance)
			baseLevel = baseLevel * (1.0f - (targetDistance - fullDistance) / (falloffDistance - fullDistance));

		float fullHorizAngle = targeting_.autoAim_.adhesionHorizontalAngle();
		float falloffHorizAngle = targeting_.autoAim_.adhesionHorizontalFalloffAngle();
		Angle yawDeltaToTarget = targetDelta.yaw() - yaw;
		Angle absYawDeltaToTarget = fabsf( yawDeltaToTarget );
		Angle baseYawLevel = baseLevel;
		if (absYawDeltaToTarget > falloffHorizAngle)
			baseYawLevel = 0.0f;
		else if (absYawDeltaToTarget > fullHorizAngle)
			baseYawLevel = baseYawLevel * (1.0f - (absYawDeltaToTarget - fullHorizAngle) /
														(falloffHorizAngle - fullHorizAngle));

		Angle pitchDeltaToTarget = -targetDelta.pitch() - pitch;
		Angle absPitchDeltaToTarget = fabsf( pitchDeltaToTarget );
		Angle basePitchLevel = baseLevel;
		if (absPitchDeltaToTarget > falloffHorizAngle)
			basePitchLevel = 0.0f;
		else if (absPitchDeltaToTarget > fullHorizAngle)
			basePitchLevel = basePitchLevel * (1.0f - (absPitchDeltaToTarget - fullHorizAngle) /
														(falloffHorizAngle - fullHorizAngle));

		// Change in yaw from forward movement
		forwardYawLevel = baseYawLevel * targeting_.autoAim_.forwardAdhesion_ * absForwardDistance;
		float deltaYaw = forwardYawLevel * absYawDeltaToTarget;
		if (yawDeltaToTarget < 0.0f)
			deltaYaw *= -1.0f;
		if (backwards)
			deltaYaw *= float(targeting_.autoAim_.reverseAdhesionStyle_);
		yaw += deltaYaw;

		// Change in yaw from strafing
		strafingYawLevel = baseYawLevel * targeting_.autoAim_.strafeAdhesion_ * absStrafeDistance;
		yawStrafeSMA_.append( strafingYawLevel );
		strafingYawLevel = yawStrafeSMA_.average();
		deltaYaw = strafingYawLevel * absYawDeltaToTarget;
		if (yawDeltaToTarget < 0.0f)
			deltaYaw *= -1.0f;
		yaw += deltaYaw;

		// Change in yaw from turning
		Angle angle = fabsf( yaw - previousYaw_ );
		while (angle >= MATH_PI)
			angle = fabsf( angle - 2 * MATH_PI );
		turningYawLevel = baseYawLevel * targeting_.autoAim_.turnAdhesion_ * fabsf( angle );
		yawTurningSMA_.append( turningYawLevel );
		turningYawLevel = yawTurningSMA_.average();
		deltaYaw = turningYawLevel * absYawDeltaToTarget;
		if (yawDeltaToTarget < 0.0f)
			deltaYaw *= -1.0f;
		yaw += deltaYaw;

		// Change in pitch from forward movement
		Angle forwardPitchLevel = basePitchLevel * targeting_.autoAim_.forwardAdhesion_ * absForwardDistance;
		float deltaPitch = forwardPitchLevel * targeting_.autoAim_.adhesionPitchToYawRatio_ * absPitchDeltaToTarget;
		if (pitchDeltaToTarget < 0.0f)
			deltaPitch *= -1.0f;
		if (backwards)
			deltaPitch *= float(targeting_.autoAim_.reverseAdhesionStyle_);
		pitch += deltaPitch;

		// Change in pitch from strafing
		Angle strafingPitchLevel = basePitchLevel * targeting_.autoAim_.strafeAdhesion_ * absStrafeDistance;
		deltaPitch = strafingPitchLevel * targeting_.autoAim_.adhesionPitchToYawRatio_ * absPitchDeltaToTarget;
		if (pitchDeltaToTarget < 0.0f)
			deltaPitch *= -1.0f;
		pitch += deltaPitch;

		// Change in pitch from turning
		angle = fabsf( pitch - previousPitch_ );
		while (angle >= MATH_PI)
			angle = fabsf( angle - 2 * MATH_PI );
		Angle turningPitchLevel = basePitchLevel * targeting_.autoAim_.turnAdhesion_ * fabsf( angle );
		deltaPitch = turningPitchLevel * absPitchDeltaToTarget;
		if (pitchDeltaToTarget < 0.0f)
			deltaPitch *= -1.0f;
		pitch += deltaPitch;

		yawChange = forwardYawLevel + strafingYawLevel + turningYawLevel;
		pitchChange = (forwardPitchLevel + strafingPitchLevel + turningPitchLevel) *
													targeting_.autoAim_.adhesionPitchToYawRatio_;
	}
	else
	{
		yawStrafeSMA_.append( 0.0f );
		yawTurningSMA_.append( 0.0f );
	}

	if (showTotalYaw_ || showForwardYaw_ || showStrafeYaw_ || showTurnYaw_ || showTotalPitch_ || showFriction_)
	{
		friction_.push_back( lastFriction_ );
		lastFriction_ = 0.0f;
		yawChange_.push_back( yawChange / dTime );
		yawForwardChange_.push_back( forwardYawLevel / dTime );
		yawStrafeChange_.push_back( strafingYawLevel / dTime );
		yawTurnChange_.push_back( turningYawLevel / dTime );
		pitchChange_.push_back( pitchChange / dTime );
	}

	previousPitch_ = pitch;
	previousYaw_ = yaw;
	previousHadATarget_ = target_ != NULL;
}

void ClientSpeedProvider::drawDebugStuff()
{
	BW_GUARD;
	if (!(showTotalYaw_ || showForwardYaw_ || showStrafeYaw_ || showTurnYaw_ || showTotalPitch_ || showFriction_)
			|| yawChange_.size() == 0)
		return;

	if (yawChange_.size() > numSamples_)
		yawChange_.erase(yawChange_.begin(), yawChange_.begin() + yawChange_.size() - numSamples_);
	if (yawForwardChange_.size() > numSamples_)
		yawForwardChange_.erase(yawForwardChange_.begin(), yawForwardChange_.begin() + yawForwardChange_.size() - numSamples_);
	if (yawStrafeChange_.size() > numSamples_)
		yawStrafeChange_.erase(yawStrafeChange_.begin(), yawStrafeChange_.begin() + yawStrafeChange_.size() - numSamples_);
	if (yawTurnChange_.size() > numSamples_)
		yawTurnChange_.erase(yawTurnChange_.begin(), yawTurnChange_.begin() + yawTurnChange_.size() - numSamples_);
	if (pitchChange_.size() > numSamples_)
		pitchChange_.erase(pitchChange_.begin(), pitchChange_.begin() + pitchChange_.size() - numSamples_);
	if (friction_.size() > numSamples_)
		friction_.erase(friction_.begin(), friction_.begin() + friction_.size() - numSamples_);

	Vector2	lineseg[3];

	lineseg[0].x = -0.8f;
	lineseg[0].y = 0.0f;
	lineseg[1].x = 0.8f;
	lineseg[1].y = 0.0f;
	lineseg[2] = lineseg[1];
	Geometrics::drawLinesInClip( lineseg, 3, Moo::Colour( 1.f, 1.f, 1.f, 1.f ) );

	float x = -0.8f;
	Moo::Colour colour( 1.f, 0.f, 0.f, 1.f );
	GraphInfo::iterator iter = yawChange_.begin();
	GraphInfo::iterator iter2 = iter + 1;
	if (showTotalYaw_)
		for (; iter2 != yawChange_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = *iter / yawPitchScale_;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = *iter2 / yawPitchScale_;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
		}

	x = -0.8f;
	colour = Moo::Colour( 1.f, 1.f, 0.f, 1.f );
	iter = yawForwardChange_.begin();
	iter2 = iter + 1;
	if (showForwardYaw_)
		for (; iter2 != yawForwardChange_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = *iter / yawPitchScale_;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = *iter2 / yawPitchScale_;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
		}

	x = -0.8f;
	colour = Moo::Colour( 1.f, 0.f, 1.f, 1.f );
	iter = yawStrafeChange_.begin();
	iter2 = iter + 1;
	if (showStrafeYaw_)
		for (; iter2 != yawStrafeChange_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = *iter / yawPitchScale_;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = *iter2 / yawPitchScale_;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
		}

	x = -0.8f;
	colour = Moo::Colour( 0.f, 1.f, 1.f, 1.f );
	iter = yawTurnChange_.begin();
	iter2 = iter + 1;
	if (showTurnYaw_)
		for (; iter2 != yawTurnChange_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = *iter / yawPitchScale_;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = *iter2 / yawPitchScale_;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
		}

	x = -0.8f;
	colour = Moo::Colour( 0.f, 1.f, 0.f, 1.f );
	iter = pitchChange_.begin();
	iter2 = iter + 1;
	if (showTotalPitch_)
		for (; iter2 != pitchChange_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = -*iter / yawPitchScale_;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = -*iter2 / yawPitchScale_;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
	}

	x = -0.8f;
	colour = Moo::Colour( 0.f, 0.f, 1.f, 1.f );
	iter = friction_.begin();
	iter2 = iter + 1;
	if (showFriction_)
		for (; iter2 != friction_.end(); ++iter, ++iter2) {
			lineseg[0].x = x;
			lineseg[0].y = -0.8f + *iter;

			x += 1.6f / numSamples_;

			lineseg[1].x = x;
			lineseg[1].y = -0.8f + *iter2;
			lineseg[2] = lineseg[1];

			Geometrics::drawLinesInClip( lineseg, 3, colour );
	}
}


// -----------------------------------------------------------------------------
// Section: ClientCamera
// -----------------------------------------------------------------------------

ClientCamera* ClientCamera::pInstance_ = NULL;

ClientCamera& ClientCamera::instance()
{
	MF_ASSERT_DEV( pInstance_ );
	return *pInstance_;
}
void ClientCamera::initInstance( DataSectionPtr configSection )
{
	BW_GUARD;
	pInstance_ = new ClientCamera( configSection );
}

/*static*/ void ClientCamera::fini()
{
	BW_GUARD;
	ClientSpeedProvider::fini();

	delete pInstance_;
	pInstance_ = NULL;
}


/**
 *	Constructor.
 */
ClientCamera::ClientCamera( DataSectionPtr configSection )
  : speedProvider_( ClientSpeedProvider::instance() )
{
	BW_GUARD;
	// We set up a cursor camera that is controlled by the dcursor and follows
	// the player.  It seems the most useful default, esp for new customers.
	CursorCamera *pCC = new CursorCamera();
	pCamera_ = pCC;
	Py_DecRef(pCC);

	pProjAccess_ = new ProjectionAccess;

	pDirectionCursor_ = new DirectionCursor( configSection, &speedProvider_ );
	pCC->source( pDirectionCursor_->provider() );
	pCC->target( Player::instance().camTarget() );

	speedProvider_.readInitialValues( configSection );
}


/**
 *	Destructor.
 */
ClientCamera::~ClientCamera()
{
	BW_GUARD;

	Py_DecRef(pDirectionCursor_);
	pDirectionCursor_ = NULL;
	Py_DecRef(pProjAccess_);
	pProjAccess_ = NULL;
}


void ClientCamera::update( float dTime )
{
	BW_GUARD;
	pCamera_->update( dTime );
	pProjAccess_->update( dTime );
}

/**
 * Helper class used to capture current state of camera
 * used for debug drawing
 */
class CameraState
{
public:
	// near plane axes
	Vector3 uaxis;
	Vector3 vaxis;
	Vector3 waxis;
	// entities position
	Vector3 entityPosition;
	// camera position
	Vector3 cameraPosition;
};

// static camera state
static CameraState * s_cameraState;

/**
 * This function draws a plane
 */
void drawPlane(const Vector3 & u, const Vector3 & v, const Vector3 & p, const Moo::Colour & colour)
{
	const Vector3 v0 = -u - v + p;
	const Vector3 v1 = -u + v + p;
	const Vector3 v2 = u + v + p;
	const Vector3 v3 = u - v + p;

	Geometrics::drawLine(v0, v1, colour, false);
	Geometrics::drawLine(v1, v2, colour, false);
	Geometrics::drawLine(v2, v3, colour, false);
	Geometrics::drawLine(v3, v0, colour, false);
}

/**
 * This function draws the debug stuff for the client camera
 */
void ClientCamera::drawDebugStuff()
{
	BW_GUARD;
	if (s_cameraState)
	{
		// draw near plane at entity position
		drawPlane(s_cameraState->uaxis, s_cameraState->vaxis, s_cameraState->entityPosition + s_cameraState->waxis, Moo::Colour(0, 0, 1, 0));

		// draw near plane at camera position
		drawPlane(s_cameraState->uaxis, s_cameraState->vaxis, s_cameraState->cameraPosition + s_cameraState->waxis, Moo::Colour(0, 1, 0, 0));

		// draw camera position
		Geometrics::instance().drawSphere(s_cameraState->cameraPosition, 0.02f, Moo::Colour(1, 0, 0, 0));
	}
}

/**
 * This function captures the current camera state to visualise
 */
static void debugNearPlane(PyObjectPtr camera)
{
	BW_GUARD;
	// clear last camera state
	if (s_cameraState)
	{
		delete s_cameraState;
		s_cameraState = NULL;
	}

	if (camera)
	{
		if ( !CursorCamera::Check( camera.get() ) )
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.debugNearPlane: "
				"debug drawing only works for cursor camera\n" );
			return ;
		}

		// check if camera is a cursor camera
		CursorCamera * cursorCamera = static_cast<CursorCamera *>( camera.get() );

		// get camera and target position
		Vector3 camPos = cursorCamera->posInWS();
		Vector3 camDir = cursorCamera->dirInWS();

		Matrix matrix;
		// cursor camera always has a target
		cursorCamera->pTarget()->matrix( matrix );
		Vector3 entityPos = matrix.applyPoint( Vector3( 0.f, cursorCamera->pivotPosition().y, 0.f ) );

		// create near plane at target entities position
		Vector3 up(0, 1, 0);
		Vector3 zAxis = camDir;
		Vector3 xAxis = up.crossProduct( zAxis );
		xAxis.normalise();
		Vector3 yAxis = zAxis.crossProduct( xAxis );

		const Moo::Camera * pCamera = &Moo::rc().camera();
		if(pCamera == NULL)
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.debugNearPlane: "
				"cannot find device camera\n" );
			return ;
		}

		const float fov = pCamera->fov();
		const float nearPlane = pCamera->nearPlane();
		const float aspectRatio = pCamera->aspectRatio();

		const float yLength = nearPlane * tanf( fov / 2.0f );
		const float xLength = yLength * aspectRatio;

		xAxis *= xLength;
		yAxis *= yLength;
		zAxis *= nearPlane;

		// set camera state
		s_cameraState = new CameraState();
		s_cameraState->uaxis = xAxis;
		s_cameraState->vaxis = yAxis;
		s_cameraState->waxis = zAxis;
		s_cameraState->entityPosition = entityPos;
		s_cameraState->cameraPosition = camPos;
	}
}

PY_AUTO_MODULE_FUNCTION(
	RETVOID, debugNearPlane, OPTARG(PyObjectPtr, NULL, END), BigWorld )


// -----------------------------------------------------------------------------
// Section: Python stuff
// -----------------------------------------------------------------------------


/*~ function BigWorld.camera
 *
 *	This function is used to get and set the camera that is used by BigWorld
 *	to render the world.  It can be called with either none or one arguments.
 *	If it has no arguments, then it returns the camera currently in use.
 *	If it is called with one argument, then that argument should be a camera
 *	and it is set to be the active camera.  In this case the function returns
 *	None.
 *
 *	@param	cam		a Camera object, which is set to be the current camera.
 *					If this is not specified, the current camera will be returned.
 *
 *	@return			A Camera object or None
 */
/**
 *	Get or set the app's camera
 */
static PyObject * camera( BaseCameraPtr pCam )
{
	BW_GUARD;
	if (!pCam) return ClientCamera::instance().camera().getObject();
	ClientCamera::instance().camera( pCam );
	return Py_None;
}
PY_AUTO_MODULE_FUNCTION(	// using retdata so ret value is incref'd
	RETDATA, camera, OPTARG( BaseCameraPtr, NULL, END ), BigWorld )


/*~ function BigWorld.projection
 *
 *	This function retrieves the ProjectionAccess object for the current camera,
 *  allowing access to such things as the fov and the clip planes.
 *
 *	@return		ProjectionAccess
 */
/**
 *	Get the app's projection access object
 */
static PyObject * projection()
{
	BW_GUARD;
	return ClientCamera::instance().projAccess();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, projection, END, BigWorld )

// client_camera.cpp
