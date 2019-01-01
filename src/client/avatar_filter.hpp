/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#ifndef AVATAR_FILTER_HPP
#define AVATAR_FILTER_HPP


#include <queue>

#include "filter.hpp"

#include "cstdmf/smartpointer.hpp"


typedef SmartPointer< PyObject >	PyObjectPtr;


/*~ class BigWorld.AvatarFilter
 *
 *	This class inherits from Filter.  It implements a filter which tracks
 *	the last 8 entity updates from the server, and linearly interpolates
 *	between them.
 *
 *	Linear interpolation is done at ( time - latency ), where time is the
 *	current engine time, and latency is how far in the past the entity
 *	currently is.
 *
 *	Latency moves from its current value in seconds to the "ideal latency"
 *	which is the time between the two most recent updates if an update
 *	has just arrived, otherwise it is 2 * minLatency.  Lantency moves at
 *	velLatency seconds per second.
 *
 *	An AvatarFilter is created using the BigWorld.AvatarFilter function.
 *	The constructor takes in an optional AvatarFilter to copy for example:
 *	
 *	@{
 *	# create an avatar filter
 *	filter = BigWorld.AvatarFilter() 
 *
 *	# create an avatar filter from another avatar filter
 *	filter = BigWorld.AvatarFilter(oldFilter)
 *	@}
 */
/**
 *	This is the standard filter to avatar like entities. It provides smooth
 *	movement and support for correctly timed python callbacks.
 *	The AvatarFilter works by storing a history of inputs rather than just
 *	one and then offsetting time so that requested outputs fall inside the
 *	period represented in the history.
 */
class AvatarFilter : public Filter
{
	Py_Header( AvatarFilter, Filter )

public:
	static const uint NUM_STORED_INPUTS = 8;

	static const int YAW	= 0;
	static const int PITCH	= 1;
	static const int ROLL	= 2;

	AvatarFilter( PyTypePlus * pType = &s_type_ );
	AvatarFilter( const AvatarFilter & filter, PyTypePlus * pType = &s_type_ );
	~AvatarFilter();

	void reset( double time );

	virtual void input(	double time,
									SpaceID spaceID,
									EntityID vehicleID,
									const Position3D & pos,
									const Vector3 & posError,
									float * auxFiltered );

	void output( double time );

	bool getLastInput(	double & time,
						SpaceID & spaceID,
						EntityID & vehicleID,
						Position3D & pos,
						Vector3 & posError,
						float * auxFiltered );

	void callback(	int whence,
					SmartPointer<PyObject> fn,
					float extra = 0.f,
					bool passMissedBy = false );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE();

	PY_RW_ATTRIBUTE_DECLARE( latency_, latency );

	PY_RW_ATTRIBUTE_DECLARE( s_latencyVelocity_, velLatency );
	PY_RW_ATTRIBUTE_DECLARE( s_latencyMinimum_, minLatency );
	PY_RW_ATTRIBUTE_DECLARE( s_latencyFrames_, latencyFrames );
	PY_RW_ATTRIBUTE_DECLARE( s_latencyCurvePower_, latencyCurvePower );

	PY_AUTO_METHOD_DECLARE( RETVOID,
							callback,
							ARG( int,
							NZARG(	SmartPointer<PyObject>,
									OPTARG( float, 0.f,
									OPTARG( bool, false, END ) ) ) ) );
	PY_METHOD_DECLARE( py_debugMatrixes );

	/**
	 *	This is an internal structure used to encapsulate a single set of
	 *	received input values for use by the AvatarFilter. Currently the
	 *	avatar filter ignores 'roll' and that is continued here.
	 */
	struct StoredInput
	{
		double		time_;
		SpaceID		spaceID_;
		EntityID	vehicleID_;
		Position3D	position_;
		Vector3		positionError_;
		Vector3		direction_;
		bool		onGround_;
	};

protected:

	/**
	 *	This structure stores a location in time and space for the filter
	 *	output to move too or from. 
	 */
	struct Waypoint
	{
		double			time_;
		SpaceID			spaceID_;
		EntityID		vehicleID_;
		Position3D		position_;
		Vector3			direction_;

		StoredInput		storedInput_;

		void changeCoordinateSystem( SpaceID spaceID, EntityID vehicleID );
	};


	/**
	 *	This structure holds a python callback to be called when the filter's
	 *	time reaches that specified.
	 */
	struct Callback
	{
		Callback( double time, PyObjectPtr function, bool passMB );

		bool operator<( const Callback & b ) const;

		double			time_;
		PyObjectPtr		function_;
		bool			passMissedBy_;

		/**
		 *	This functor exists so that priority queue of Callback pointers can
		 *	be ordered by timestamp rather than pointer value.
		 *	Containers using this functor must never contain null pointers.
		 */
		struct CallbackPtrLessThanFunctor
		{
			bool operator()( const Callback * a, const Callback * b );
		};
	};


	/**
	 *	This typedef defines a STL priority queue of Callback pointers. The
	 *	queue is sorted by the time stamps of the callbacks referenced.
	 *
	 *	@see Callback::CallbackPtrLessThanFunctor
	 */
	typedef std::priority_queue< Callback *,
					std::vector< Callback * >,
					Callback::CallbackPtrLessThanFunctor > CallbackQueue;


protected:

	void resetStoredInputs(	double time,
							SpaceID spaceID,
							EntityID vehicleID,
							const Position3D & position,
							const Vector3 & positionError,
							float * auxFiltered );


	void extract(	double time,
					SpaceID & outputSpaceID,
					EntityID & outputVehicleID,
					Position3D & outputPosition,
					Vector3 & outputVelocity,
					Vector3 & outputDirection );

	void chooseNextWaypoint( double time );

public:
	StoredInput & getStoredInput( uint index );
protected:
	const StoredInput & getStoredInput( uint index ) const;


	// Doxygen comments for all members can be found in the .cpp
	StoredInput		storedInputs_[NUM_STORED_INPUTS];
	uint			currentInputIndex_;

	Waypoint		nextWaypoint_;
	Waypoint		previousWaypoint_;

	float			latency_;
	float			idealLatency_;
	double			timeOfLastOutput_;
	bool			gotNewInput_;
	bool			reset_;

	CallbackQueue	callbacks_;

public:
	static float s_latencyVelocity_;
	static float s_latencyMinimum_;
	static float s_latencyFrames_;
	static float s_latencyCurvePower_;
};


#endif // AVATAR_FILTER_HPP


// avatar_filter.hpp
