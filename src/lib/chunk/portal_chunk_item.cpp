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

#include "portal_chunk_item.hpp"

#include "chunk.hpp"
#include "chunk_model_obstacle.hpp"
#include "chunk_obstacle.hpp"


// -----------------------------------------------------------------------------
// Section: PortalObstacle
// -----------------------------------------------------------------------------

/**
 *	This class is used to add a collision scene item for a portal. It checks
 *	the permissive state of the associated portal to decide whether to proceed
 *	with the collision test.
 */
class PortalObstacle : public ChunkObstacle
{
public:
	PortalObstacle( PortalChunkItem * pChunkItem );

	virtual bool collide( const Vector3 & source, const Vector3 & extent,
		CollisionState & state ) const;
	virtual bool collide( const WorldTriangle & source, const Vector3 & extent,
		CollisionState & state ) const;

private:
	void buildTriangles();

	PortalChunkItem * pChunkItem() const
	{
		return static_cast< PortalChunkItem * >( this->pItem().get() );
	}

	ChunkBoundary::Portal * pPortal() const
	{
		return this->pChunkItem()->pPortal();
	}

	WorldTriangle::Flags collisionFlags() const
	{
		return this->pPortal()->collisionFlags();
	}

	BoundingBox		bb_;
	mutable std::vector<WorldTriangle>	ltris_;
};


/**
 *	Constructor
 */
PortalObstacle::PortalObstacle( PortalChunkItem * pChunkItem ) :
	ChunkObstacle( pChunkItem->chunk()->transform(), &bb_, pChunkItem )
{
	BW_GUARD;
	// now calculate our bb. fortunately the ChunkObstacle constructor
	// doesn't do anything with it except store it.

	ChunkBoundary::Portal * pPortal = this->pPortal();

	// extend 10cm into the chunk (the normal is always normalised)
	Vector3 ptExtra = pPortal->plane.normal() * 0.10f;

	// build up the bb from the portal points
	for (uint i = 0; i < pPortal->points.size(); i++)
	{
		Vector3 pt =
			pPortal->uAxis * pPortal->points[i][0] +
			pPortal->vAxis * pPortal->points[i][1] +
			pPortal->origin;

		if (!i)
			bb_ = BoundingBox( pt, pt );
		else
			bb_.addBounds( pt );
		bb_.addBounds( pt + ptExtra );
	}

	// and figure out the triangles (a similar process)
	this->buildTriangles();
}


/**
 *	Build the 'world' triangles to collide with
 */
void PortalObstacle::buildTriangles()
{
	BW_GUARD;
	ltris_.clear();

	ChunkBoundary::Portal * pPortal = this->pPortal();

	// extend 5cm into the chunk
	Vector3 ptExOri = pPortal->origin + pPortal->plane.normal() * 0.05f;

	// build the wt's from the points
	Vector3 pto, pta, ptb(0.f,0.f,0.f);
	for (uint i = 0; i < pPortal->points.size(); i++)
	{
		// shuffle and find the next pt
		pta = ptb;
		ptb =
			pPortal->uAxis * pPortal->points[i][0] +
			pPortal->vAxis * pPortal->points[i][1] +
			ptExOri;

		// stop if we don't have enough for a triangle
		if (i < 2)
		{
			// start all triangles from pt 0.
			if (i == 0) pto = ptb;
			continue;
		}

		// make a triangle then
		ltris_.push_back( WorldTriangle( pto, pta, ptb ) );
	}
}


/**
 *	Collision test with an extruded point
 */
bool PortalObstacle::collide( const Vector3 & source, const Vector3 & extent,
	CollisionState & state ) const
{
	BW_GUARD;
	// see if we let anyone through
	if (this->pPortal()->permissive)
		return false;

	// ok, see if they collide then
	// (chances are very high if they're in the bb!)
	Vector3 tranl = extent - source;
	for (uint i = 0; i < ltris_.size(); i++)
	{
		// see if it intersects
		float rd = 1.0f;
		if (!ltris_[i].intersects( source, tranl, rd ) ) continue;

		// see how far we really travelled (handles scaling, etc.)
		float ndist = state.sTravel_ + (state.eTravel_-state.sTravel_) * rd;

		if (state.onlyLess_ && ndist > state.dist_) continue;
		if (state.onlyMore_ && ndist < state.dist_) continue;
		state.dist_ = ndist;

		// call the callback function
		ltris_[i].flags( uint8(this->collisionFlags()) );
		int say = state.cc_( *this, ltris_[i], state.dist_ );

		// see if any other collisions are wanted
		if (!say) return true;

		// some are wanted ... see if it's only one side
		state.onlyLess_ = !(say & 2);
		state.onlyMore_ = !(say & 1);
	}

	return false;
}


/**
 *	Collision test with an extruded triangle
 */
bool PortalObstacle::collide( const WorldTriangle & source, const Vector3 & extent,
	CollisionState & state ) const
{
	BW_GUARD;
	// see if we let anyone through
	if (this->pPortal()->permissive)
		return false;

	// ok, see if they collide then
	// (chances are very high if they're in the bb!)
	Vector3 tranl = extent - source.v0();
	for (uint i = 0; i < ltris_.size(); i++)
	{
		// see if it intersects
		if (!ltris_[i].intersects( source, tranl ) ) continue;

		// see how far we really travelled
		float ndist = state.sTravel_;

		if (state.onlyLess_ && ndist > state.dist_) continue;
		if (state.onlyMore_ && ndist < state.dist_) continue;
		state.dist_ = ndist;

		// call the callback function
		ltris_[i].flags( uint8(this->collisionFlags()) );
		int say = state.cc_( *this, ltris_[i], state.dist_ );

		// see if any other collisions are wanted
		if (!say) return true;

		// some are wanted ... see if it's only one side
		state.onlyLess_ = !(say & 2);
		state.onlyMore_ = !(say & 1);
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: PortalChunkItem
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
PortalChunkItem::PortalChunkItem( ChunkBoundary::Portal * pPortal ) :
	pPortal_( pPortal )
{
}


/**
 *	This methods creates the collision obstacles of a portal.
 */
void PortalChunkItem::toss( Chunk * pChunk )
{
	if (pChunk_ != NULL)
	{
		ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
	}

	this->ChunkItem::toss( pChunk );

	if (pChunk_ != NULL)
	{
		ChunkModelObstacle::instance( *pChunk_ ).addObstacle(
				new PortalObstacle( this ) );
	}
}

#ifndef MF_SERVER
#include "moo/visual_channels.hpp"
#include "moo/render_context.hpp"

/*
 *  This class is a helper class for rendering PortalChunkItem
 */
class PortalDrawItem : public Moo::ChannelDrawItem
{
public:
	PortalDrawItem( const Vector3* pRect, uint32 colour )
	: colour_( colour )
	{
		BW_GUARD;
		memcpy( rect_, pRect, sizeof(Vector3) * 4 );
		distance_ = (pRect[0].z + pRect[1].z + pRect[2].z + pRect[3].z) / 4.f;
	}
	void draw()
	{
		BW_GUARD;
		Moo::Material::setVertexColour();
		Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		Moo::rc().setRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		Moo::rc().setRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );

		Moo::rc().setFVF( Moo::VertexXYZNDS::fvf() );
		Moo::rc().setVertexShader( NULL );
		Moo::rc().device()->SetPixelShader( NULL );
		Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
		Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
		Moo::rc().device()->SetTransform( D3DTS_VIEW, &Matrix::identity );
		Moo::rc().device()->SetTransform( D3DTS_WORLD, &Matrix::identity );

		Moo::VertexXYZNDS pVerts[4];

		for (int i = 0; i < 4; i++)
		{
			pVerts[i].colour_ = colour_;
			pVerts[i].specular_ = 0xffffffff;
		}
		pVerts[0].pos_ = rect_[0];
		pVerts[1].pos_ = rect_[1];
		pVerts[2].pos_ = rect_[3];
		pVerts[3].pos_ = rect_[2];

		Moo::rc().drawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVerts, sizeof( pVerts[0] ) );
	}
	void fini()
	{
		delete this;
	}
private:
	Vector3 rect_[4];
	uint32 colour_;
};

namespace
{
	/*
	 *	Helper class for watcher value for draw portals
	 */
	class ShouldDrawChunkPortals
	{
	public:
		ShouldDrawChunkPortals() :
		  state_( false )
		{
			MF_WATCH( "Render/Draw PortalChunkItems", state_ );
		}
		bool state_;

	};
}

/**
 *	Draw method to debug portal states
 */
void PortalChunkItem::draw()
{
	BW_GUARD;
	
	static ShouldDrawChunkPortals shouldDrawChunkPortals;

	// get out if we don't want to draw them
	if (!shouldDrawChunkPortals.state_) return;

	// get the transformation matrix
	Matrix tran;
	tran.multiply( Moo::rc().world(), Moo::rc().view() );

	// transform all the points
	Vector3 prect[4];
	for (int i = 0; i < 4; i++)
	{
		// project the point straight into clip space
		tran.applyPoint( prect[i], Vector3(
			pPortal_->uAxis * pPortal_->points[i][0] +
			pPortal_->vAxis * pPortal_->points[i][1] +
			pPortal_->origin ) );

	}

	Moo::SortedChannel::addDrawItem( 
		new PortalDrawItem( prect, 
			pPortal_->permissive ? 0xff003300 : 0xff550000 ) 
		);
}
#else
void PortalChunkItem::draw()
{
}
#endif

// portal_chunk_item.cpp
