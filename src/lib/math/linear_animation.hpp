/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef LINEAR_ANIMATION_HPP
#define LINEAR_ANIMATION_HPP

#include <iostream>

#include <map>

template< class _Ty >
class LinearAnimation : public std::map< float, _Ty >
{
public:

	//maxTime is only used if loop = true
	LinearAnimation( bool loop = false, float maxTime = 0 )
	{
		loop_ = loop;
		maxTime_ = maxTime;
	}
	~LinearAnimation()
	{
	}

	_Ty animate( float time ) const
	{
		// Do we have any keyframes in our list?
		if( !size() )
		{
			return _Ty();
		}

		_Ty res;

		// Does our animation loop?
		if( loop_ )
		{
			// Is our time within the segment?
			if( time > maxTime_ || time < 0 )
			{
				time -= floorf( time / maxTime_ ) * maxTime_;
			}

			//get the two keyframes which are closest to the time value
			//we want to animate to.
			const_iterator iter = upper_bound( time );
			const_iterator iter2 = iter;

			if( iter == begin() )
				iter = end();

			iter--;

			if( iter2 == end() )
			{
				iter2 = begin();
			}

			
			float firstTime = iter->first;
			_Ty   firstValue = iter->second;

			float lastTime = iter2->first;
			_Ty	  lastValue = iter2->second;

			//if the first keyframe is at time, return it.
			if( firstTime == time )
				return firstValue;

			//adjust times for loop
			if( firstTime > lastTime )
				lastTime += maxTime_;

			if( firstTime > time )
				time += maxTime_;

			//do linear interpolation
			float fraction = ( time - firstTime ) / ( lastTime - firstTime );
			res = firstValue + ( ( lastValue - firstValue ) * fraction ) ;

		}
		else
		{
			//get the closest keyframe to the time
			const_iterator iter = this->upper_bound( time );

			//if the keyframe we got was past the last keyframe, return the last keyframe
			if( iter == this->end() )
			{
				iter --;
				return iter->second;
			}
			
			//if the keyframe returned is the first one return the first keyframe
			if( iter == this->begin() )
				return iter->second;

			//get the two closest keyframes
			const_iterator iter2 = iter--;

			float firstTime = iter->first;
			_Ty   firstValue = iter->second;

			float lastTime = iter2->first;
			_Ty	  lastValue = iter2->second;

			//if the first keyframe is at time, return it.
			if( time == firstTime )
				return firstValue;
			
			//do linear interpolation
			float fraction = ( time - firstTime ) / ( lastTime - firstTime );
			res = firstValue + ( ( lastValue - firstValue ) * fraction );
		}
		return res;
	}

	void addKey( float time, const _Ty &value )
	{
		//is our animation looping
		if( loop_ )
		{
			//if the keyframe is inside our time segment, add it
            if( time < maxTime_  && time >= 0 )
            {
                (*this)[time] = value;
            }
		}
		else
		{
			//add the key
			(*this)[time] = value;
		}
	}

	float getEndTime( void )
	{
		if (empty() ||
			almostZero( (--end())->first - begin()->first ))
		{
			return 0.f;
		}

		//is it not looping?		
		if( !loop_ )
		{
			//return the last time
			if( size() )
				return (--end())->first;
		}
		//return the max time
		return maxTime_;
	}

	float getBeginTime( void )
	{
		//is it not looping?		
		if( !loop_ )
		{
			//return the first time
			if( size() )
				return begin()->first;
		}

		return 0;
	}

	float getTotalTime( void )
	{
		if (empty() ||
			almostZero( (--end())->first - begin()->first ))
		{
			return 0.f;
		}

		//is it not looping?		
		if( !loop_ )
		{
			//return the difference between the first and last time
			if( size() )
				return (--end())->first - begin()->first;
		}
		//return the maximum time (total time for a looping animation
		return maxTime_;
	}

	void loop( bool state, float loopTime )
	{
		loop_ = state;
		maxTime_ = loopTime;
	}

	void reset( void )
	{
		//erase all elements in the map
        clear();
	}

private:

	bool loop_;
	float maxTime_;
	
	LinearAnimation(const LinearAnimation&);
	LinearAnimation& operator=(const LinearAnimation&);

	template<class _Ty> friend std::ostream& operator<<(std::ostream&, const LinearAnimation<_Ty>&);
};


#endif
/*linear_animation.hpp*/