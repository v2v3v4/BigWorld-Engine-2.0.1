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

#include "animation.hpp"

#ifndef CODE_INLINE
#include "animation.ipp"
#endif

#include "cstdmf/binaryfile.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/bwresource.hpp"
#include "animation_manager.hpp"
#include "streamed_animation_channel.hpp"
#include "interpolated_animation_channel.hpp"
#include "translation_override_channel.hpp"
#include "node_catalogue.hpp"

#ifdef _DEBUG
	#include "cstdmf/profile.hpp"
	#include <strstream>
#endif


DECLARE_DEBUG_COMPONENT2( "Moo", 0 )
PROFILER_DECLARE( Animation_translationOverrideAnim, "Animation translationOverrideAnim" );

namespace Moo {

/**
 * Constructor for animation
 * @param anim pointer to the animation to copy
 * @param root the rootnode of the hierarchy the animation applies to
 */
Animation::Animation( Animation * anim, NodePtr root ) :
	totalTime_( anim->totalTime() ),
	identifier_( anim->identifier() ),
	internalIdentifier_( anim->internalIdentifier() ),
	pItinerantRoot_( NULL ),
	pStreamer_( anim->pStreamer_ ),
	pMother_( anim ),
	tickers_( anim->tickers_ )
{
	BW_GUARD;
	for( uint32 i = 0; i < anim->nChannelBinders(); i++ )
	{
		AnimationChannelPtr pAC = anim->channelBinder( i ).channel();
		channelBinders_.push_back( ChannelBinder( pAC, root->find( pAC->identifier() ) ) );
	}
}

/**
 * Contructor for animation
 * @param anim the animation to apply 
 */
Animation::Animation( Animation * anim ) :
	totalTime_( anim->totalTime() ),
	identifier_( anim->identifier() ),
	internalIdentifier_( anim->internalIdentifier() ),
	pItinerantRoot_( NULL ),
	pStreamer_( anim->pStreamer_ ),
	pMother_( anim ),
	tickers_( anim->tickers_ )
{
    BW_GUARD;
	NodePtr nullMooNodePointer = NULL;

	for( uint32 i = 0; i < anim->nChannelBinders(); i++ )
	{
		AnimationChannelPtr pAC = anim->channelBinder( i ).channel();

		//2 is Morph channels.  They don't bind to nodes.
		//TODO : change this type check and instead make a 
		//new virtual interface AnimationChannel::wantsNode,
		//that way we don't check for magic numbers.
		if (pAC->type() != 2)
		{
			NodePtr pNode = NodeCatalogue::find( pAC->identifier().c_str() );
			if (!pNode)
			{
				pNode = new Moo::Node;
				pNode->identifier( pAC->identifier() );
				pNode = NodeCatalogue::findOrAdd( pNode );
			}

			channelBinders_.push_back(ChannelBinder( pAC,pNode ));
		}
		else
		{
			//Morph animation channels should never have a Node
			channelBinders_.push_back( ChannelBinder( pAC, NULL ) );
		}
	}
}

/**
 * Contructor for animation
 */
Animation::Animation() :
	totalTime_( 0 ),
	pItinerantRoot_( NULL )
{
}

/**
 * Destructor for animation
 */
Animation::~Animation()
{
	BW_GUARD;
	// remove ourselves from the animation manager's global map.
	// we will only be in it if we are an orphan ... if we have
	// a mother then she will be the one in the map.
	if (!pMother_) AnimationManager::instance().del( this );
}

uint32 Animation::nChannelBinders( ) const
{
	return channelBinders_.size();
}

const ChannelBinder& Animation::channelBinder( uint32 i ) const
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( channelBinders_.size() > i )
	{
		MF_EXIT( "index out of range" );
	}

	return channelBinders_[ i ];
}

ChannelBinder& Animation::channelBinder( uint32 i )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( channelBinders_.size() > i )
	{
		MF_EXIT( "index out of range" );
	}

	return channelBinders_[ i ];
}

void Animation::addChannelBinder( const ChannelBinder& binder )
{
	BW_GUARD;
	channelBinders_.push_back( binder );
	pItinerantRoot_ = NULL;
	if (binder.channel() && binder.channel()->wantTick())
		tickers_.push_back( binder.channel() );
}

/**
 * This method plays a frame of an animation.
 * @param time the frame to play back
 */
void Animation::animate( float time ) const
{
	BW_GUARD;
	ChannelBinderVector::const_iterator it = channelBinders_.begin();
	ChannelBinderVector::const_iterator end = channelBinders_.end();

	while (it != end)
	{
		it->animate( time );
		++it;
	}
}

#if 0
/**
 * This method blends two animations together
 * @param anim the animation to blend with.
 * @param time1 the frame to play back on this animation
 * @param time2 the frame to play back on the passed in animation
 * @param t the blend ratio between the two animations 0 = this animation 1 = passed in animation
 */
void Animation::blend( AnimationPtr anim, float time1, float time2, float t  ) const
{
	BW_GUARD;
	if( t <= 0 )
	{
		animate( time1 );
		return;
	}

	anim->animate( time2 );

	if( t >= 1 )
		return;


	ChannelBinderVector::const_iterator it = channelBinders_.begin();
	ChannelBinderVector::const_iterator end = channelBinders_.end();

	while (it != end)
	{
		NodePtr n = it->node();
		if( n )
		{
			BlendTransform b2( n->transform() );
			BlendTransform b1( it->channel()->result( time1 ) );
			b1.blend( t, b2 );
			b1.output( n->transform() );
		}

		++it;
	}
}
#endif

/**
 * This method blends a number of animations together
 * @param first the iterator of the first blended animation to play back.
 * @param final the iterator of the last + 1 blended animation to play back.
 */
void Animation::animate( BlendedAnimations::const_iterator first, BlendedAnimations::const_iterator final )
{
	BW_GUARD;
	if ( first+1 == final )
	{
		first->animation_->animate( first->frame_ );
	}
	else if (first != final)
	{
		BlendTransform channelTransform;

		ChannelBinderVector::const_iterator chan;
		ChannelBinderVector::const_iterator chanEnd;
		int chanIndex;

		for (BlendedAnimations::const_iterator it = first;
			it != final;
			++it)
		{
			if( it->blendRatio_ != 0 )
			{
				chan = it->animation_->channelBinders_.begin();
				chanEnd = it->animation_->channelBinders_.end();
				chanIndex = 0;
				while( chan != chanEnd )
				{
					float newBr = it->blendRatio_;
					if( it->alphas_ )
					{
						newBr *= (*it->alphas_)[ chanIndex ];
					}

					NodePtr n = chan->node();
					if( n )
					{
						if( newBr > 0 )
						{
							float oldBr = n->blend( Node::s_blendCookie_ );
							float sumBr = oldBr + newBr;
							chan->channel()->result( it->frame_, channelTransform );

							if (oldBr == 0.f)
							{
								n->blendTransform() = channelTransform;
							}
							else
							{
								n->blendTransform().blend( newBr / sumBr, channelTransform );
							}
							n->blend( Node::s_blendCookie_, sumBr );
						}
					}
					else
					{
						chan->channel()->nodeless( it->frame_, newBr );
					}

					++chan;
					++chanIndex;
				}
			}
		}
	}

	Node::s_blendCookie_ = (Node::s_blendCookie_+1) & 0x0fffffff;
}


/**
 * This method blends this animation into the node hierarchy
 * @param blendCookie the cookie of the current blend
 * @param frame the frame to play back
 * @param blendRatio the ratio of blend for this animation
 * @param pAlphas blendfactors for individual nodes
 */
void Animation::animate( int blendCookie, float frame, float blendRatio,
	const NodeAlphas * pAlphas )
{
	BW_GUARD;
	if (pStreamer_)
	{
		pStreamer_->touch( frame );
		pStreamer_->touch( min( totalTime_, frame + 8.f ) );
		pStreamer_->touch( max( 0.f, frame - 8.f ) );
	}

	if (blendRatio <= 0.f) return;

	BlendTransform channelTransform;

	ChannelBinderVector::const_iterator chan;
	int chanIndex;

	for (chan = channelBinders_.begin(), chanIndex = 0;
		chan != channelBinders_.end();
		++chan, ++chanIndex)
	{
		float newBr = (pAlphas == NULL) ?
			blendRatio : blendRatio * (*pAlphas)[ chanIndex ];

		NodePtr n = chan->node();
		if (!n)
		{
			chan->channel()->nodeless( frame, newBr );
			continue;
		}

		if( newBr > 0.f )
		{
			chan->channel()->result( frame, channelTransform );

			float oldBr = n->blend( blendCookie );
			float sumBr = oldBr + newBr;

			if (oldBr == 0.f)
			{
				n->blendTransform() = channelTransform;
			}
			else
			{
				n->blendTransform().blend( newBr / sumBr, channelTransform );
			}
			n->blend( blendCookie, sumBr );
		}
		else if (newBr < 0.f)
		{
			float oldBr = n->blend( blendCookie );
			if (oldBr > 0.f)
			{
				Matrix mtemp;
				chan->channel()->result( frame, channelTransform );
				channelTransform.blend( 1.f - newBr, BlendTransform() );
				channelTransform.output( mtemp );
				n->transform().preMultiply( mtemp );
			}
		}
	}
}


/**
 *	This method ticks this animation.
 *	Note that unlike with the animate methods, frame numbers here
 *	should not wrap around if the animation is being looped.
 */
void Animation::tick( float dtime, float oframe, float nframe,
	float bframe, float eframe )
{
	BW_GUARD;
	if (tickers_.empty()) return;

	for (AnimationChannelVector::const_iterator it = tickers_.begin();
		it != tickers_.end();
		it++)
	{
		(*it)->tick( dtime, oframe, nframe, bframe, eframe );
	}
}


/**
 *	This method finds the channel corresponding to the given node
 */
AnimationChannelPtr Animation::findChannel( NodePtr node ) const
{
	BW_GUARD;
	ChannelBinderVector::const_iterator it = channelBinders_.begin();
	ChannelBinderVector::const_iterator end = channelBinders_.end();

	while (it != end)
	{
		if( it->node() == node )
		{
			return it->channel();
		}
		++it;
	}
	return NULL;
}

AnimationChannelPtr Animation::findChannel( const std::string& identifier )
{
	BW_GUARD;
	ChannelBinderVector::const_iterator it = channelBinders_.begin();
	ChannelBinderVector::const_iterator end = channelBinders_.end();

	while (it != end)
	{
		if( it->channel()->identifier() == identifier )
		{
			return it->channel();
		}
		++it;
	}
	return NULL;
}

/**
 * This method returns the itinerant root of the animation. The channel closest
 * to the root that actually moves.
 * @return the channelbinder for the itinerant root.
 */
ChannelBinder * Animation::itinerantRoot( ) const
{
	BW_GUARD;
	if( pItinerantRoot_ )
		return pItinerantRoot_;

	// for now we assume that the itinerant root channel is always the
	//  second one in the animation; after the scene root, which stays still.
	if (channelBinders_.size() >= 2)
	{
		pItinerantRoot_ = const_cast<ChannelBinder*>( &channelBinders_[ 1 ] );
	}


#if 0
	// Try to find a channel that is the closest child of the root of all the other channels.
	bool foundRoot = false;

	// Step throught all the channels to find a root
	for( uint32 i = 0; i < channelBinders_.size() && foundRoot == false; i++ )
	{
		uint32 closestChild = 0xffffff;
		uint32 maxDescendants = 0;
		pItinerantRoot_ = NULL;
		const ChannelBinder& root = channelBinders_[ i ];
		if( root.node() )
		{
			pItinerantRoot_ = NULL;
			foundRoot = true;

			// Step through all the channels to find out if they are all children, and also
			// to find the closest child to the root.
			for( uint32 j = 0; j < channelBinders_.size() && foundRoot == true; j++ )
			{
				if( j != i )
				{
					NodePtr pChild = channelBinders_[ j ].node();
					if( pChild != NULL )
					{
						uint32 childDistance = 0;
						uint32 nDescendants = pChild->countDescendants();
						// Are we a child?
						while( pChild && !( pChild == root.node() ) )
						{
							childDistance++;
							pChild = pChild->parent();
						}
						foundRoot = pChild == root.node();

						// Are we the closest child, if we are a child at all?
						if ((foundRoot && childDistance < closestChild) ||
							(foundRoot && childDistance == closestChild && nDescendants > maxDescendants ) )
						{
							closestChild = childDistance;
							pItinerantRoot_ = const_cast<ChannelBinder*>( &channelBinders_[ j ] );
							maxDescendants = nDescendants;
						}
					}
				}
			}
		}
	}
#endif

	if( !pItinerantRoot_ )
	{
		WARNING_MSG( "Moo::Animation::itinerantRoot:"
				"Animation %s has no channel that matches the criteria for the itinerant root!\n"
				"\t(stuffed node hierarchy)\n",
			identifier_.c_str() );
		pItinerantRoot_ = const_cast<ChannelBinder*>( &channelBinders_[ 0 ] );
	}

	return pItinerantRoot_;
}

/**
 *	Load the given resource ID into this animation
 */
bool Animation::load( const std::string & resourceID )
{
	BW_GUARD;
	DataSectionPtr pAnimSect;
	BinaryPtr bptr;

	const StreamedDataCache::EntryInfo * ei = NULL;

	// find the modified time of the file if it's there
	uint64 modifiedTime = BWResource::modifiedTime( resourceID );

	// get our data either from a file or an anim cache
	BinaryFile * pbf = NULL;

	bool usingCache = StreamedAnimation::s_loadingAnimCache_ != NULL &&
		(ei = StreamedAnimation::s_loadingAnimCache_->
		findEntryData( resourceID, 0, modifiedTime )) != NULL;

	for(;;)
	{
		if( pbf )
		{
			delete pbf;
			pbf = NULL;
		}
		if( usingCache )
		{
			//DEBUG_MSG( "Loading animation from cache: %s\n", resourceID.c_str() );

			StreamedDataCache* pCache = StreamedAnimation::s_loadingAnimCache_;
			//uint32 preloadSize;
			//pCache->preload( dataOffset, sizeof(preloadSize), &preloadSize );
			//dataOffset += sizeof(preloadSize);

			pbf = new BinaryFile( NULL );
			void * preloadData = pbf->reserve( ei->preloadSize_ );
			pCache->preload( ei->fileOffset_, ei->preloadSize_, preloadData );

			pStreamer_ = new StreamedAnimation();
			StreamedAnimation::s_currentAnim_ = &*pStreamer_;
		}
		else
		{
			pAnimSect = BWResource::openSection( resourceID );
			if (!pAnimSect) return false;

			bptr = pAnimSect->asBinary();

			if (!bptr) return false;

			pbf = new BinaryFile( NULL );
			pbf->wrap( (void*)bptr->data(), bptr->len(), false );
		}
		BinaryFile & bf = *pbf;


	#ifdef _DEBUG
		uint64	loadTime = timestamp();
	#endif

		// read the anim header
		bf >> totalTime_ >> identifier_ >> internalIdentifier_;

		Animation* pOverride = NULL;

		// now read the channel data
		int	ncb;
		bf >> ncb;
		for (int i=0; i<ncb; i++)
		{
			int	type;
			bf >> type;
			AnimationChannelPtr	pAC = AnimationChannel::construct( type );
			if (pAC)
			{
				if (pAC->load( bf ))
				{
					this->addChannelBinder( ChannelBinder( pAC, NULL ) );		
				}
				else
				{
					ERROR_MSG(	"Error in resouce file: '%s' channel: '%s'\n",
								resourceID.c_str(),
								pAC->identifier().c_str());
					pbf->error(true);
					break;
				}
			}
			else
			{
				ERROR_MSG( "Unable to construct animation channel from file: '%s' channel binder: %d type: %d\n",
					resourceID.c_str(), i, type );
				pbf->error(true);
				break;
			}


			if (type == 7)
			{
				TranslationOverrideChannel* toc = reinterpret_cast<TranslationOverrideChannel*>(pAC.getObject());
				pOverride = toc->pBaseAnim().getObject();
			}
		}

		if( !bf )
		{
			if( usingCache )
			{
				usingCache = false;
				StreamedAnimation::s_loadingAnimCache_->deleteOnClose( true );
				continue;
			}
			else
			{
				delete pbf;
				return false;
			}
		}

		if (pOverride)
		{
			MF_ASSERT_DEV(!pStreamer_.hasObject());
			pStreamer_ = pOverride->pStreamer_;
		}

		// and set up the streaming animation if we found one
		bool explicitSave = false;
		if (pStreamer_ && !pOverride)
		{
			if( ei )
				pStreamer_->load( bf, totalTime_,
					StreamedAnimation::s_loadingAnimCache_, ei->fileOffset_ + ei->preloadSize_ );
			else
				explicitSave = true;
		}

		StreamedAnimation::s_currentAnim_ = NULL;
		bool ret = !!bf;
		delete pbf;
		pbf = NULL;

		// if we want to stream but we didn't load from the cache,
		// add ourselves to the cache for next time.
		if (ret && StreamedAnimation::s_savingAnimCache_ != NULL && ( !pStreamer_ && !pOverride || explicitSave ) )
		{
			this->save( resourceID, modifiedTime );
		}

		if( !ret && usingCache )
		{
			usingCache = false;
			StreamedAnimation::s_loadingAnimCache_->deleteOnClose( true );
			continue;
		}
		return ret;
	}

	MF_EXIT( "Animation::load: Should never reach here" );

	return false;
}


/**
 *	Helper class for converting saved animation channels
 */
class StreamedAnimConverter : public InterpolatedAnimationChannel::Converter
{
public:
	StreamedAnimConverter() : pIneligible_( NULL ) { }

	virtual int type()
		{ return 5; }
	virtual bool eligible( const InterpolatedAnimationChannel & iac )
		{ return static_cast<const AnimationChannel*>( &iac ) != pIneligible_; }
	virtual bool saveAs( BinaryFile & bf,
		const InterpolatedAnimationChannel & iac );

	AnimationChannel * pIneligible_;
};

bool StreamedAnimConverter::saveAs( BinaryFile & bf,
	const InterpolatedAnimationChannel & iac )
{
	BW_GUARD;
	StreamedAnimation * pStreamer = StreamedAnimation::s_currentAnim_;

	StreamedAnimationChannel * pSAC = new StreamedAnimationChannel();

	pStreamer->prep( pSAC );
	pSAC->mirror( iac );
	pStreamer->add( pSAC );

	return pSAC->save( bf );
}

static StreamedAnimConverter s_streamedAnimConverter;


/**
 *	Save this animation to the given resource ID
 */
bool Animation::save( const std::string & resourceID, uint64 useModifiedTime )
{
	BW_GUARD;
	std::string	filename = resourceID;
	if (filename.empty())
	{
		ERROR_MSG( "Animation::save: Could not resolve resource ID '%s'\n",
			resourceID.c_str() );
		return false;
	}

	TRACE_MSG( "Animation::save: Writing converted animation: %s\n",
		resourceID.c_str() );

	BinaryFile * pbf = NULL;

	bool convertToStreamed = (StreamedAnimation::s_savingAnimCache_ != NULL);

	if (convertToStreamed)
	{
		// save it into a temporary file
		filename[filename.length()-1] = 't';
	}

	FILE * f = BWResource::instance().fileSystem()->posixFileOpen( filename, "wb" );
	if (f == NULL)
	{
		return false;
	}

	pbf = new BinaryFile( f );
	BinaryFile & bf = *pbf;

	// no early returns from here on

	StreamedAnimationPtr pStreamerSave = pStreamer_;
	if (convertToStreamed)
	{
		pStreamer_ = new StreamedAnimation();
		pStreamer_->load( bf, totalTime_,
			StreamedAnimation::s_savingAnimCache_, uint32(-1) );

		StreamedAnimation::s_currentAnim_ = &*pStreamer_;

		InterpolatedAnimationChannel::s_saveConverter_ =
			&s_streamedAnimConverter;
		ChannelBinder * pCB = this->itinerantRoot();
		s_streamedAnimConverter.pIneligible_ = pCB ? &*pCB->channel() : NULL;
	}

	bf << totalTime_ << identifier_ << internalIdentifier_;

	int ncb = channelBinders_.size();
	bf << ncb;
	for (int i=0; i<ncb; i++)
	{
		AnimationChannelPtr	pAC = channelBinders_[i].channel();
		bf << pAC->type();
		pAC->save( bf );
	}

	if (pStreamer_) pStreamer_->save( bf );

	bool ret = !!bf;

	delete pbf;

	// ok, done with standard saving

	// if we're converting to a streamed animation, then read that temporary
	// file back in, save it to the cache, delete it, and write out the
	// streamed data to the cache also.
	if (convertToStreamed)
	{
		InterpolatedAnimationChannel::s_saveConverter_ = NULL;
		StreamedAnimation::s_currentAnim_ = NULL;

		StreamedDataCache* pCache = StreamedAnimation::s_savingAnimCache_;

		// tell the cache about us
		uint32 preloadOffset = pCache->addEntryData(
			resourceID, StreamedDataCache::CACHE_VERSION, useModifiedTime );

		// save out our preload data
		FILE * f = BWResource::instance().fileSystem()->posixFileOpen( filename, "rb" );
		fseek( f, 0, SEEK_END );
		uint32 preloadSize = ftell( f );
		//const uint32 psSize = sizeof(preloadSize);
		void * preloadData = malloc( /*psSize +*/ preloadSize );
		//*(uint32*)preloadData = preloadSize;
		fseek( f, 0, SEEK_SET );
		fread( ((char*)preloadData) /*+ psSize*/, 1, preloadSize, f );
		fclose( f );

		BWResource::instance().fileSystem()->eraseFileOrDirectory( filename );

		pCache->immsave( preloadOffset, /*psSize +*/ preloadSize, preloadData );

		free( preloadData );

		// save out our streamed data
		uint32 streamedOffset = preloadOffset + /*psSize +*/ preloadSize;
		uint32 streamedSize = pStreamer_->dam( streamedOffset );

		// ok now get rid of the streamer; it was only for conversion
		pStreamer_ = pStreamerSave;

		// tell the cache we've finished writing
		pCache->endEntryData( preloadSize, streamedSize
			/*psSize + preloadSize + streamedSize*/ );

		// and finally get the cache to save out its directory
		pCache->saveSelf();
	}

	return ret;
}

void Animation::translationOverrideAnim( AnimationPtr pBase, AnimationPtr pTranslationReference,
								const std::vector< std::string >& noOverrideChannels )
{
		BW_GUARD_PROFILER( Animation_translationOverrideAnim );
		this->channelBinders_.clear();
		this->pStreamer_ = pBase->pStreamer_;
		this->totalTime_ = pBase->totalTime_;
		this->identifier_ = pBase->identifier_;
		this->internalIdentifier_ = pBase->internalIdentifier_;


		ChannelBinderVector::iterator it = pBase->channelBinders_.begin();
		ChannelBinderVector::iterator end = pBase->channelBinders_.end();

		while (it != end )
		{
			bool override = false;
			Vector3 offset( 0, 0, 0 );
			AnimationChannelPtr pBaseChannel = it->channel();
			AnimationChannelPtr pTransChan = pTranslationReference->findChannel( it->channel()->identifier() );
			if ( pTransChan.hasObject() &&
				std::find( noOverrideChannels.begin(), noOverrideChannels.end(), pBaseChannel->identifier() ) == noOverrideChannels.end() )
			{
				override = true;
				offset = pTransChan->result( 0 )[3];
			}

			AnimationChannelPtr pNew = new TranslationOverrideChannel( offset, override, pBaseChannel, pBase );
			this->addChannelBinder( ChannelBinder( pNew, NULL ) );
			it++;
		}
}




};

// animation.cpp
