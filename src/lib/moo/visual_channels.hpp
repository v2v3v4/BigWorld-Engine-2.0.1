/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_CHANNELS_HPP
#define VISUAL_CHANNELS_HPP

#include "visual.hpp"
#include "effect_state_manager.hpp"

namespace Moo
{

class VisualChannel;
typedef SmartPointer<VisualChannel> VisualChannelPtr;
/**
 * This class is the base class and the manager for the visual channels. A 
 * channel is essentially a bucket of objects with similar rendering rules.
 * The channels are traversed fixed global order, and within each channel items
 * are drawn, possibly in a sorted order. 
 */
class VisualChannel : public SafeReferenceCount
{
public:
	virtual ~VisualChannel(){};
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours ) = 0;

	virtual void clear() = 0;

	static void add( const std::string& name, VisualChannelPtr pChannel );
	static void remove( const std::string& name );
	static VisualChannelPtr get( const std::string& name );
	static void initChannels();
	static void finiChannels();
	static void clearChannelItems();
	static bool enabled() { return enabled_; }
	static void enabled(bool enabled) { enabled_ = enabled; }
private:
	typedef std::map< std::string, VisualChannelPtr > VisualChannels;
	static VisualChannels visualChannels_;
protected:
	static bool enabled_;
};


/**
 * This is the base class for the primitives drawn by the visual channels.
 */
class ChannelDrawItem : public ReferenceCount
{
public:
	virtual ~ChannelDrawItem(){}
	virtual void draw() = 0;
	virtual void fini() {};
	float distance() const { return distance_; }
	void alwaysVisible(bool val) {bAlwaysVisible_=val;}
	bool alwaysVisible() const {return bAlwaysVisible_;}
protected:
	float			distance_;
	bool			bAlwaysVisible_;
};

typedef ChannelDrawItem* ChannelDrawItemPtr;
class VisualDrawItem;
typedef VisualDrawItem* VisualDrawItemPtr;

/**
 * This class implements the draw item used by visuals.
 */
class VisualDrawItem : public ChannelDrawItem
{
public:
	void init( 
		VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		SmartPointer<StateRecorder> pStateRecorder, 
		uint32 primitiveGroup, float distance, 
		StaticVertexColoursPtr pStaticVertexColours = NULL, 
		bool sortInternal = false );

	virtual void draw();
	virtual void fini();

	static bool s_overrideZWrite_;
	static bool overrideZWrite() { return s_overrideZWrite_; }
	static void overrideZWrite(bool val) { s_overrideZWrite_=val; }

	static VisualDrawItemPtr get();
protected:
	void drawSorted();
	void drawUnsorted();
	VertexSnapshotPtr pVSS_;
	PrimitivePtr	pPrimitives_;
	uint32			primitiveGroup_;
	Vector3			partialWorldView_;
	SmartPointer<StateRecorder> pStateRecorder_;
	StaticVertexColoursPtr pStaticVertexColours_;

	bool			sortInternal_;

	static uint32 s_nextAlloc_;
	static std::vector< VisualDrawItemPtr > s_drawItems_;
};

class ShimmerDrawItem;
typedef SmartPointer<ShimmerDrawItem> ShimmerDrawItemPtr;

/**
* This class implements the draw item used by shimmer items.
*/
class ShimmerDrawItem : public VisualDrawItem
{
public:
	void draw();
	static ShimmerDrawItemPtr get();
protected:	
	static uint32 s_nextAlloc_;
	static std::vector< ShimmerDrawItemPtr > s_drawItems_;
};

/**
 * This class implements the shimmer channel, the shimmer channel is used to add 
 * a wavy effect to the scene. The effect is masked by the items from a full
 * screen shimmer effect. 

 * Shimmer visuals are rendered after the main scene draw but before the sorted 
 * channel is rendered.
 */
class ShimmerChannel : public VisualChannel
{
public:
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours );
	virtual void clear();
	static void draw();
	static void addDrawItem( ChannelDrawItem* pItem );
	static uint32 nItems()	{ return s_nItems_; }
	static void incrementShimmerCount()	{ s_nItems_++; }
protected:
	typedef VectorNoDestructor< ChannelDrawItemPtr >	DrawItems;
	static DrawItems s_drawItems_;
	static uint32 s_timeStamp_;
	static uint32 s_nItems_;
};

/**
 * This class implements the sorted channel, sorted objects are rendered after 
 * the main scene and the shimmer.
 */
class SortedChannel : public VisualChannel
{
public:
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours );
	static void draw(bool clear=true);
	virtual void clear();
	static void addDrawItem( ChannelDrawItem* pItem );

	static void push();
	static void pop();

	static void reflecting(bool val) { s_reflecting_ = val; }
protected:
	typedef VectorNoDestructor< ChannelDrawItemPtr >	DrawItems;
	typedef std::vector< DrawItems >					DrawItemStack;

	static DrawItems& drawItems() { return s_drawItems_.back(); }
	static DrawItemStack	s_drawItems_;
	static uint32 s_timeStamp_;

	static bool s_reflecting_;
};

/**
 * This class implements the internal sorted channel, the internal sorted 
 * channel is rendered with the sorted channel, with it's triangles sorted 
 * against each other.
 */
class InternalSortedChannel : public SortedChannel
{
public:
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours );

protected:
};

/**
 * This class implements the sorted shimmer channel, this channel draws to both 
 * the main buffer and the alpha buffer.
 */
class SortedShimmerChannel : public SortedChannel
{
public:
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours );
};

class DistortionDrawItem;
typedef SmartPointer<DistortionDrawItem> DistortionDrawItemPtr;

/**
 * This class holds an item for the distortion channel.
 */
class DistortionDrawItem : public VisualDrawItem
{
public:
	void drawMask();
	static DistortionDrawItemPtr get();

	void initMask( SmartPointer<StateRecorder> maskPass )
	{
		maskPass_ = maskPass;
	}

	virtual void fini();
private:
	SmartPointer<StateRecorder> maskPass_;
protected:
	static uint32 s_nextAlloc_;
	static std::vector< DistortionDrawItemPtr > s_drawItems_;
};

/*
 * The distortion channel holds objects that have a "wobbly glass" like 
 * distortion effect that can be defined by a per-material distortion map
 * (unlike the shimmer channel which is global).
 */
class DistortionChannel : public VisualChannel
{
public:
	virtual void addItem( VertexSnapshotPtr pVSS, PrimitivePtr pPrimitives, 
		EffectMaterialPtr pMaterial, uint32 primitiveGroup, float zDistance, 
		StaticVertexColoursPtr pStaticVertexColours );
	virtual void clear();
	static void draw( bool clear = true );
	static void addDrawItem( DistortionDrawItem* pItem );
	static uint drawCount() { return s_drawItems_.size(); }
	static void enabled( bool state ) { enabled_ = state; }
protected:
	typedef VectorNoDestructor< DistortionDrawItemPtr >	DrawItems;
	static DrawItems s_drawItems_;
	static uint32 s_timeStamp_;

private:
	static void setMaskState( bool enabled = true );
	static bool enabled_;
};

}
#endif // VISUAL_CHANNELS_HPP
