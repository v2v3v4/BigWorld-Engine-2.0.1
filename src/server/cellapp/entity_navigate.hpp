/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_NAVIGATE_HPP
#define ENTITY_NAVIGATE_HPP

#include "entity.hpp"
#include "entity_extra.hpp"


/// This macro defines the attribute function for a method
#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_ENTITY_EXTRA_WITH_DOC

/**
 *	This class is an entity extra that supports navigation with a navmesh.
 */
class EntityNavigate : public EntityExtra
{
	Py_EntityExtraHeader( EntityNavigate )

public:
	EntityNavigate( Entity & e );
	~EntityNavigate();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_METHOD_DECLARE( RETOWN, moveToPoint,
		ARG( Vector3, ARG( float, OPTARG( int, 0,
		OPTARG( bool, true, OPTARG( bool, false, END ) ) ) ) ) )
	PyObject * moveToPoint( Vector3 destination,
							float velocity,
							int userArg = 0,
							bool faceMovement = true,
							bool moveVertically = false );

	PY_AUTO_METHOD_DECLARE( RETOWN, moveToEntity,
		ARG( int, ARG( float, ARG( float, OPTARG( int, 0,
		OPTARG( bool, true, OPTARG( bool, false, END ) ) ) ) ) ) )
	PyObject * moveToEntity( int destEntityID,
								float velocity,
								float range,
								int userArg = 0,
								bool faceMovement = true,
								bool moveVertically = false );

	PY_AUTO_METHOD_DECLARE( RETOWN, accelerateToPoint,
		ARG( Position3D,
		ARG( float,
		ARG( float,
		OPTARG( int, 1,
		OPTARG( bool, false,
		OPTARG( int, 0, END ) ) ) ) ) ) );
	PyObject * accelerateToPoint(	Position3D destination,
									float acceleration,
									float maxSpeed,
									int facing = 1,
									bool stopAtDestination = true,
									int userArg = 0);

	PY_AUTO_METHOD_DECLARE( RETOWN, accelerateAlongPath,
		ARG( std::vector<Position3D>,
		ARG( float,
		ARG( float,
		OPTARG( int, 1,
		OPTARG( int, 0, END ) ) ) ) ) );
	PyObject * accelerateAlongPath(	std::vector<Position3D> waypoints,
									float acceleration,
									float maxSpeed,
									int facing = 1,
									int userArg = 0);

	PY_AUTO_METHOD_DECLARE( RETOWN, accelerateToEntity,
		ARG( EntityID,
		ARG( float,
		ARG( float,
		ARG( float,
		OPTARG( int, 1,
		OPTARG( int, 0, END ) ) ) ) ) ) );
	PyObject * accelerateToEntity(	EntityID destinationEntity,
									float acceleration,
									float maxSpeed,
									float range,
									int facing = 1,
									int userArg = 0);


	PY_AUTO_METHOD_DECLARE( RETOWN, navigate,
		ARG( Vector3, ARG( float,
		OPTARG( bool, true, OPTARG( float, 500.f, OPTARG( float, 0.5f, 
		OPTARG( float, 0.01f, OPTARG(int, 0, END ) ) ) ) ) ) ) )
	PyObject * navigate( const Vector3 & dstPosition, float velocity,
		bool faceMovement = true, float maxDistance = 500.f, float girth = 0.5f, float closeEnough = 0.01f, int userArg = 0 );

	PY_AUTO_METHOD_DECLARE( RETOWN, navigateStep,
		ARG( Vector3, ARG( float, ARG( float, OPTARG( float, 500.f,
		OPTARG( bool, true, OPTARG( float, 0.5f, OPTARG( int, 0, END ) ) ) ) ) ) ) )
	PyObject * navigateStep( const Vector3 & dstPosition, float velocity,
		float maxMoveDistance, float maxSearchDistance = 500.f, bool faceMovement = true,
		float girth = 0.5f, int userArg = 0 );


	PY_AUTO_METHOD_DECLARE( RETOWN, navigateFollow,
			ARG( PyObjectPtr, ARG( float, ARG( float,
			ARG( float, ARG( float, OPTARG( float, 500.f, OPTARG( bool, true,
			OPTARG ( float, 0.5f, OPTARG( int, 0, END ) ) ) ) ) ) ) ) ) )

	PyObject * navigateFollow( PyObjectPtr pEntity, float angle, float offset,
		float velocity, float maxMoveDistance, float maxSearchDistance = 500.f, 
		bool faceMovement = true, float girth = 0.5f, int userArg = 0 );


	PY_AUTO_METHOD_DECLARE( RETOWN, canNavigateTo,
		ARG( Vector3, OPTARG( float, 500.f, OPTARG( float, 0.5f, END ) ) ) )
	PyObject * canNavigateTo( const Vector3 & dstPosition,
		float maxDistance = 500.f, float girth = 0.5f );

	PY_AUTO_METHOD_DECLARE( RETDATA, waySetPathLength, END )
	int waySetPathLength();

	PY_AUTO_METHOD_DECLARE( RETOWN, getStopPoint,
		ARG( Vector3, ARG( bool, OPTARG( float, 500.f, OPTARG( float, 0.5f,
		OPTARG( float, 1.5f, OPTARG( float, 1.8f, END ) ) ) ) ) ) )
	PyObject * getStopPoint( const Vector3 & dstPosition,
			bool ignoreFirstStopPoint, float maxDistance = 500.f,
			float girth = 0.5f, float stopDistance = 1.5f, 
			float nearPortalDistance = 1.8f );

	PY_AUTO_METHOD_DECLARE( RETOWN, navigatePathPoints, 
		ARG( Vector3, 
		OPTARG( float, 500.f, OPTARG( float, 0.5f, END ) ) ) )
	PyObject * navigatePathPoints( const Vector3 & dstPosition, 
		float maxSearchDistance = 500.f, float girth = 0.5 );

	bool getNavigatePosition( class NavLoc srcLoc, class NavLoc dstLoc,
		float maxDistance, Vector3& nextPosition, bool & passedActivatedPortal,
		float girth = 0.5f );

	static const Instance<EntityNavigate> instance;
	void validateNavLoc( const Vector3 & position,
			float girth, class NavLoc & out );
};

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC

#endif // ENTITY_NAVIGATE_HPP
