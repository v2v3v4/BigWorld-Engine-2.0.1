/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLIENT_CAMERA_HPP
#define CLIENT_CAMERA_HPP

#include "input/input.hpp"

#include "cstdmf/smartpointer.hpp"
#include "camera/direction_cursor.hpp"
#include "pyscript/stl_to_py.hpp"
#include "math/sma.hpp"
#include "entity_picker.hpp"


typedef SmartPointer< class DataSection > DataSectionPtr;

class BaseCamera;
class ProjectionAccess;
class DirectionCursor;
class Entity;

typedef std::vector< Vector2 > V2Vector;
typedef SmartPointer< BaseCamera > BaseCameraPtr;

/**
 *  AutoAim is the python interface to the targeting values.
 *  It is used exclusively by ClientSpeedProvider.
 */
class AutoAim : public PyObjectPlus
{
	Py_Header( AutoAim, PyObjectPlus )

	AutoAim( PyTypePlus * pType = &s_type_ );
public:
	~AutoAim() {}

	static AutoAim& instance();
	static void initInstance();
	static void fini();

	virtual void readInitialValues( DataSectionPtr rootSection );

	PY_RW_ATTRIBUTE_DECLARE( friction_, friction )
	PY_RW_ATTRIBUTE_DECLARE( forwardAdhesion_, forwardAdhesion )
	PY_RW_ATTRIBUTE_DECLARE( strafeAdhesion_, strafeAdhesion )
	PY_RW_ATTRIBUTE_DECLARE( turnAdhesion_, turnAdhesion )
	PY_RW_ATTRIBUTE_DECLARE( adhesionPitchToYawRatio_, adhesionPitchToYawRatio )
	PY_RW_ATTRIBUTE_DECLARE( reverseAdhesionStyle_, reverseAdhesionStyle )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionHorizontalAngle, frictionHorizontalAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionVerticalAngle, frictionVerticalAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionDistance, frictionDistance )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionHorizontalFalloffAngle, frictionHorizontalFalloffAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionVerticalFalloffAngle, frictionVerticalFalloffAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionMinimumDistance, frictionMinimumDistance )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, frictionFalloffDistance, frictionFalloffDistance )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionHorizontalAngle, adhesionHorizontalAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionVerticalAngle, adhesionVerticalAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionDistance, adhesionDistance )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionHorizontalFalloffAngle, adhesionHorizontalFalloffAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionVerticalFalloffAngle, adhesionVerticalFalloffAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, adhesionFalloffDistance, adhesionFalloffDistance )

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	float frictionHorizontalAngle() const;
	void frictionHorizontalAngle( float angle );
	float frictionVerticalAngle() const;
	void frictionVerticalAngle( float angle );
	float frictionDistance() const;
	void frictionDistance( float distance );
	float frictionHorizontalFalloffAngle() const;
	void frictionHorizontalFalloffAngle( float angle );
	float frictionVerticalFalloffAngle() const;
	void frictionVerticalFalloffAngle( float angle );
	float frictionMinimumDistance() const;
	void frictionMinimumDistance( float distance );
	float frictionFalloffDistance() const;
	void frictionFalloffDistance( float distance );

	float adhesionHorizontalAngle() const;
	void adhesionHorizontalAngle( float angle );
	float adhesionVerticalAngle() const;
	void adhesionVerticalAngle( float angle );
	float adhesionDistance() const;
	void adhesionDistance( float distance );
	float adhesionHorizontalFalloffAngle() const;
	void adhesionHorizontalFalloffAngle( float angle );
	float adhesionVerticalFalloffAngle() const;
	void adhesionVerticalFalloffAngle( float angle );
	float adhesionFalloffDistance() const;
	void adhesionFalloffDistance( float distance );

	float friction_;
	float forwardAdhesion_;
	float strafeAdhesion_;
	float turnAdhesion_;
	float adhesionPitchToYawRatio_;
	int reverseAdhesionStyle_;

protected:
	static AutoAim* pInstance_;
	
	AutoAim( const AutoAim& );
	AutoAim& operator=( const AutoAim& );
};

PY_SCRIPT_CONVERTERS_DECLARE( AutoAim )


/**
 *  Targeting is the python interface to the targeting values.
 *  It is used exclusively by ClientSpeedProvider.
 */
class Targeting : public PyObjectPlus
{
	Py_Header( Targeting, PyObjectPlus )

	Targeting( PyTypePlus * pType = &s_type_ );
public:
	~Targeting() {}

	static Targeting& instance();
	static void initInstance();

	virtual void readInitialValues( DataSectionPtr rootSection );

	Entity*	hasAnAutoAimTarget( float& autoAimTargetDistance, float& autoAimTargetAngle,
								bool useFriction, bool wantHorizontalAngle );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, selectionAngle, selectionAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, deselectionAngle, deselectionAngle )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, selectionDistance, selectionDistance )
	PY_RO_ATTRIBUTE_DECLARE( this->defaultSelectionAngle(), defaultSelectionAngle )
	PY_RO_ATTRIBUTE_DECLARE( this->defaultDeselectionAngle(), defaultDeselectionAngle )
	PY_RO_ATTRIBUTE_DECLARE( this->defaultSelectionDistance(), defaultSelectionDistance )

	PY_RW_ATTRIBUTE_DECLARE( autoAimOn_, autoAimOn )
	PY_RO_ATTRIBUTE_DECLARE( &autoAim_, autoAim )

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	float selectionAngle() const;
	void selectionAngle( float angle );
	float deselectionAngle() const;
	void deselectionAngle( float angle );
	float selectionDistance() const;
	void selectionDistance( float distance );
	float defaultSelectionAngle() const;
	float defaultDeselectionAngle() const;
	float defaultSelectionDistance() const;

	bool autoAimOn_;

	AutoAim& autoAim_;

protected:
	static Targeting* pInstance_;
	
	Targeting( const Targeting& );
	Targeting& operator=( const Targeting& );
};


/**
 *  CameraSpeed is the python interface to the camera movement values.
 *  It is used exclusively by ClientSpeedProvider.
 */
class CameraSpeed : public PyObjectPlus
{
	Py_Header( CameraSpeed, PyObjectPlus )

	CameraSpeed( PyTypePlus * pType = &s_type_ );
public:
	~CameraSpeed() {}

	static CameraSpeed& instance();
	static void initInstance();

	virtual void readInitialValues( DataSectionPtr rootSection );

	PY_RW_ATTRIBUTE_DECLARE( lookAxisMin_, lookAxisMin )
	PY_RW_ATTRIBUTE_DECLARE( lookAxisMax_, lookAxisMax )
	PY_RW_ATTRIBUTE_DECLARE( accelerateStart_, accelerateStart )
	PY_RW_ATTRIBUTE_DECLARE( accelerateRate_, accelerateRate )
	PY_RW_ATTRIBUTE_DECLARE( accelerateEnd_, accelerateEnd )
	PY_RW_ATTRIBUTE_DECLARE( accelerateWhileTargetting_, accelerateWhileTargetting )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, cameraYawSpeed, cameraYawSpeed )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, cameraPitchSpeed, cameraPitchSpeed )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, turningHalfLife, turningHalfLife )

	PY_RW_ATTRIBUTE_DECLARE( lookTableHolder_, lookTable )

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	float cameraYawSpeed() const;
	void cameraYawSpeed( float speed );
	float cameraPitchSpeed() const;
	void cameraPitchSpeed( float speed );
	float turningHalfLife() const;
	void turningHalfLife( float halfLife );

	V2Vector lookTable_;
	float lookAxisMin_;
	float lookAxisMax_;
	float cameraYawSpeed_;
	float cameraPitchSpeed_;
	float accelerateStart_ ;
	float accelerateRate_ ;
	float accelerateEnd_;
	bool accelerateWhileTargetting_;

	PySTLSequenceHolder<V2Vector>	lookTableHolder_;

protected:
	static CameraSpeed* pInstance_;
	
	CameraSpeed( const CameraSpeed& );
	CameraSpeed& operator=( const CameraSpeed& );
};


/**
 *	The ClientSpeedProvider object provides control of the
 *	DirectionCursor that is non-linear, accelerates and
 *	has friction when near targets.
 */
class ClientSpeedProvider : public SpeedProvider
{
	enum { kNumSamples = 50 };

	ClientSpeedProvider();
public:
	~ClientSpeedProvider() {}

	static ClientSpeedProvider& instance();
	static void initInstance();
	static void fini();

	virtual void readInitialValues( DataSectionPtr rootSection );

	virtual float value( const AxisEvent &event );
	virtual void adjustDirection( float dTime, Angle& pitch, Angle& yaw, Angle& roll );

	void drawDebugStuff();

protected:
	Targeting& targeting_;
	CameraSpeed& cameraSpeed_;

	Entity* target_;
	float previousPitch_;
	float previousYaw_;
	bool previousHadATarget_;
	float yawAccelerationTime_;
	SMA<float> yawStrafeSMA_;
	SMA<float> yawTurningSMA_;

	// Debug stuff
	bool showTotalYaw_;
	bool showForwardYaw_;
	bool showStrafeYaw_;
	bool showTurnYaw_;
	bool showTotalPitch_;
	bool showFriction_;
	int yawPitchScale_;
	unsigned int numSamples_;
	typedef std::vector<float> GraphInfo;
	float lastFriction_;
	GraphInfo friction_;
	GraphInfo yawForwardChange_;
	GraphInfo yawStrafeChange_;
	GraphInfo yawTurnChange_;
	GraphInfo yawChange_;
	GraphInfo pitchChange_;

	static ClientSpeedProvider* pInstance_;
	
	ClientSpeedProvider( const ClientSpeedProvider& );
	ClientSpeedProvider& operator=( const ClientSpeedProvider& );
};


/**
 *	This class maintains the current camera-related objects
 */
class ClientCamera
{
public:
	~ClientCamera();

	static ClientCamera& instance();
	static void initInstance( DataSectionPtr configSection );
	static void fini();

	BaseCameraPtr	camera() const { return pCamera_; };
	void			camera( BaseCameraPtr pCamera ) { pCamera_ = pCamera; };

	ProjectionAccess*	projAccess() const { return pProjAccess_; };
	void			projAccess( ProjectionAccess* pProjAccess ) { pProjAccess_ = pProjAccess; };

	DirectionCursor* directionCursor() const { return pDirectionCursor_; };

	void			update( float dTime );

	void			drawDebugStuff();

private:
	ClientCamera( DataSectionPtr configSection );

	BaseCameraPtr pCamera_;
	ProjectionAccess* pProjAccess_;
	ClientSpeedProvider& speedProvider_;
	DirectionCursor* pDirectionCursor_;

	static ClientCamera* pInstance_;
	
	ClientCamera( const ClientCamera& );
	ClientCamera& operator=( const ClientCamera& );
};


#ifdef CODE_INLINE
#include "client_camera.ipp"
#endif

#endif // CLIENT_CAMERA_HPP
