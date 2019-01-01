/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ADAPTIVE_LOD_CONTROLLER_HPP
#define ADAPTIVE_LOD_CONTROLLER_HPP

#pragma warning ( disable : 4786 )

#include <iostream>
#include <vector>
#include "cstdmf/debug.hpp"


/**
 *	This class performs adaptive lodding based on the frame rate.
 *	The frame rate passed in is smoothed over n frames, to remove
 *	outliers, and to aid in smoothly adapting the level of detail.
 *
 *	( Assuming the level of detail of an application can be specified
 *	by changing a bunch of floats... )
 */
class AdaptiveLODController
{
public:
	AdaptiveLODController( int smoothingPeriod = 7 );
	~AdaptiveLODController();

	/**
	 * TODO: to be documented.
	 */
	struct LODController
	{
		LODController()
			:name_( "unknown" ),
			 variable_( NULL ),
			 default_( 1.f ),
			 worst_( 0.1f ),
			 current_( 1.f ),
			 speed_( 50.f ),
			 relativeImportance_( 1.f )
		{
			calculateDValue();
		}

		void speed( float s )
		{
			speed_ = s;
			calculateDValue();

		};

		float speed( void ) const	{ return speed_; }

		void worst( float w )
		{
			worst_ = w;
			calculateDValue();
		}

		float worst( void ) const	{ return worst_; }

		void defaultValue( float d )
		{
			default_ = d;
			calculateDValue();
		}

		float defaultValue( void ) const	{ return default_; }

		std::string name_;
		float * variable_;
		float	relativeImportance_;
		float	current_;
		float	dValue_;
	private:
		void calculateDValue()
		{
			dValue_ = ( ( default_ - worst_ ) / speed_ );
		}
		float	default_;
		float	worst_;
		float	speed_;
	};

	///adds controllers to the system
	int		addController( const std::string& name, float * variable, 
				float _default, float worst, float speed = 50.f,
				float relativeImportance_ = 1.f );
	int		addController( const LODController & controller );
	AdaptiveLODController::LODController & controller( int idx );
	int		numControllers( void ) const;

	///minimumFPS specifies the point where the adaptive degradation kicks in
	void	minimumFPS( float fps );
	float	minimumFPS( void ) const;

	///the smoothed fps
	float	effectiveFPS( void ) const;

	///Call fpsTick to update the level of detail of the application.
	///use the instantaneous fps, because AdaptiveLODController will
	///smooth the values
	void	fpsTick( float instantaneousFPS );

private:
	///performs sma on the instantaneous fps values
	float	filter( float instantaneousFPS );

	///our control variables
	typedef std::vector<LODController>	LodControllerVector;
	LodControllerVector	lodControllers_;
	float	minimumFPS_;

	///These members are involved in performing a simple moving average
	///of the frame rate ticks
	int		filterIdx_;
	int		maxFilterIdx_;
	float	filterSum_;
	int		numSamples_;
	float *	framesPerSecond_;
	float	effectiveFPS_;

	AdaptiveLODController(const AdaptiveLODController&);
	AdaptiveLODController& operator=(const AdaptiveLODController&);
	friend std::ostream& operator<<(std::ostream&, const AdaptiveLODController&);
};

#ifdef CODE_INLINE
#include "adaptive_lod_controller.ipp"
#endif




#endif
/*adaptive_lod_controller.hpp*/
