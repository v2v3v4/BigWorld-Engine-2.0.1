/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <iostream>

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stringmap.hpp"
#include "animation_channel.hpp"

namespace Moo
{

typedef std::vector<float> NodeAlphas;

typedef std::vector< struct BlendedAnimation> BlendedAnimations;

class Animation;
class StreamedAnimation;

typedef SmartPointer< Animation >			AnimationPtr;
typedef SmartPointer< StreamedAnimation >	StreamedAnimationPtr;


/**
 *	The animation class is a collection of channelbinders, these channelbinders
 *	bind together a node and an animation channel so that we can animate a node
 *	hierarchy.
 */
class Animation : public ReferenceCount
{
public:

	Animation( Animation * anim, NodePtr root );
	Animation( Animation * anim );
	Animation();

	void					animate( float time ) const;

	static void				animate( const BlendedAnimations & list );
	static void				animate( BlendedAnimations::const_iterator first,
								BlendedAnimations::const_iterator final );

//	void					blend( AnimationPtr anim, float time1, float time2, float t ) const;

	void					animate(
								int blendCookie, float frame, float blendRatio,
								const NodeAlphas * pAlphas = NULL );

	void					tick( float dtime, float oframe, float nframe,
								float bframe, float eframe );

	uint32					nChannelBinders( ) const;
	const ChannelBinder&	channelBinder( uint32 i ) const;
	/*const*/ ChannelBinder&	channelBinder( uint32 i ) /*const*/;
	void					addChannelBinder( const ChannelBinder& binder );

	AnimationChannelPtr		findChannel( NodePtr node ) const;
	AnimationChannelPtr		findChannel( const std::string& identifier );


	ChannelBinder *			itinerantRoot( ) const;

	float					totalTime( ) const;
	void					totalTime( float time );

	const std::string		identifier( ) const;
	void					identifier( const std::string& identifier );

	const std::string		internalIdentifier( ) const;
	void					internalIdentifier( const std::string& ii );

	bool					load( const std::string & resourceID );
	bool					save( const std::string & resourceID,
								uint64 useModifiedTime = 0 );

	void					translationOverrideAnim( AnimationPtr pBase, AnimationPtr pTranslationReference,
								const std::vector< std::string >& noOverrideChannels );

private:
	~Animation();

	float								totalTime_;
	ChannelBinderVector					channelBinders_;
	std::string							identifier_;
	std::string							internalIdentifier_;

	mutable ChannelBinder				* pItinerantRoot_;

	StreamedAnimationPtr				pStreamer_;

	AnimationPtr						pMother_;

	AnimationChannelVector				tickers_;

	Animation(const Animation&);
	Animation& operator=(const Animation&);

	friend std::ostream& operator<<(std::ostream&, const Animation&);
	friend class AnimationManager;
};

/**
 *	Animation blending information. Holds the details required for combining
 *	multiple animations.
 */
struct BlendedAnimation
{
	Animation						*animation_;
	const NodeAlphas				*alphas_;
	float							frame_;
	float							blendRatio_;
};

};

#ifdef CODE_INLINE
#include "animation.ipp"
#endif




#endif // ANIMATION_HPP
