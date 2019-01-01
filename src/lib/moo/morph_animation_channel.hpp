/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MORPH_ANIMATION_CHANNEL_HPP
#define MORPH_ANIMATION_CHANNEL_HPP

#include "animation_channel.hpp"

namespace Moo
{

/**
 *	This class is an animation channel that controls a morph factor
 */
class MorphAnimationChannel : public AnimationChannel
{
public:
	MorphAnimationChannel();
	~MorphAnimationChannel();

	virtual	Matrix		result( float time ) const
		{ nodeless(time, 1.f); return Matrix::identity; }
	virtual void		result( float time, Matrix& out ) const
		{ nodeless(time, 1.f); out = Matrix::identity; }
	virtual void		result( float time, BlendTransform& out ) const
		{ nodeless(time, 1.f); Matrix m; this->result( time, m ); out.init( m ); }
	virtual void		nodeless( float time, float blendRatio ) const;

	virtual bool		load( BinaryFile & bf );
	virtual bool		save( BinaryFile & bf ) const;

	virtual void		preCombine ( const AnimationChannel & rOther );
	virtual void		postCombine( const AnimationChannel & rOther );
	virtual AnimationChannel * duplicate() const;

	virtual int	 type() const						{ return 2; }

private:
	std::vector< float > influences_;

	static AnimationChannel * New()
		{ return new MorphAnimationChannel(); }
	static TypeRegisterer s_rego_;
};

} // namespace Moo


#ifdef CODE_INLINE
#include "morph_animation_channel.ipp"
#endif

#endif // MORPH_ANIMATION_CHANNEL_HPP
