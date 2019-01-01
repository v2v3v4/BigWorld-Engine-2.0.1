/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DISCRETE_ANIMATION_CHANNEL_HPP
#define DISCRETE_ANIMATION_CHANNEL_HPP

#include <iostream>
#include <map>
#include "animation_channel.hpp"

namespace Moo
{

/**
 *	A discretely timed animation channel (no interpolation of the transform 
 *	between keys).
 */
class DiscreteAnimationChannel : public AnimationChannel
{
public:
	// Note: These matrices are not aligned, so do no calculations on them
	typedef std::map< float, D3DXMATRIX > DiscreteKeyframes;


	DiscreteAnimationChannel();
	~DiscreteAnimationChannel();

	virtual Matrix	result( float time ) const;
	virtual void result( float time, Matrix& out ) const;

	void	addKey( float time, const Matrix& key );

	void	finalise();

	virtual void		preCombine ( const AnimationChannel & rOther );
	virtual void		postCombine( const AnimationChannel & rOther );

	virtual bool		load( BinaryFile & bf );
	virtual bool		save( BinaryFile & bf ) const;

	virtual int			type() const { return 0; }

	virtual AnimationChannel * duplicate() const
		{ return new DiscreteAnimationChannel( *this ); }

private:	
	DiscreteKeyframes		keyframes_;

	DiscreteAnimationChannel& operator=(const DiscreteAnimationChannel&);

	friend std::ostream& operator<<(std::ostream&, const DiscreteAnimationChannel&);

	static AnimationChannel * New()
		{ return new DiscreteAnimationChannel(); }
	static TypeRegisterer s_rego_;
};

}

#ifdef CODE_INLINE
#include "discrete_animation_channel.ipp"
#endif




#endif
/*discrete_animation_channel.hpp*/
