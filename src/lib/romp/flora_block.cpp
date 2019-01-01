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
#include "flora_block.hpp"
#include "flora_constants.hpp"
#include "flora_renderer.hpp"
#include "ecotype.hpp"
#include "flora.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/avector.hpp"
#include "math/vector2.hpp"
#include "math/planeeq.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )


static float s_ecotypeBlur = 1.f;
void setEcotypeBlur( float amount )
{
	s_ecotypeBlur = amount;
	std::vector< Flora* > floras = Flora::floras();
	for( unsigned i=0; i<floras.size(); i++)
	{
		floras[i]->initialiseOffsetTable(amount);
		floras[i]->floraReset();
	}
}

float getEcotypeBlur()
{
	return s_ecotypeBlur;
}

static float s_positionBlur = 1.3f;
void setPositionBlur( float amount )
{
	s_positionBlur = amount;
	std::vector< Flora* > floras = Flora::floras();
	for( unsigned i=0; i<floras.size(); i++)
	{
		floras[i]->initialiseOffsetTable(amount);
		floras[i]->floraReset();
	}
}

float getPositionBlur()
{
	return s_positionBlur;
}


void initFloraBlurAmounts( DataSectionPtr pSection )
{
	//This occurs when loading/unloading spaces.  Don't call
	//through to the setPositionBlur/setEcotypeBlur fns as
	//we don't want unnecessarily recreate the existing flora
	//which is either already or just about to be unloaded.
	pSection->readFloat( "ecotypeBlur", s_ecotypeBlur );
	pSection->readFloat( "positionBlur", s_positionBlur );
}


/**
 *	Constructor.
 */
FloraBlock::FloraBlock( Flora* flora ):
	bRefill_( true ),
	center_( 0.f, 0.f ),
	culled_( true ),
	blockID_( -1 ),
	flora_( flora )
{	
}


/**
 *	Thie method initialises the flora block.  FloraBlocks must be given
 *	a position.
 *
 *	@param	pos		Vector2 describing the center of the block.
 *	@param	offset	number of vertexes offset from start of vertex memory
 */
void FloraBlock::init( const Vector2& pos, uint32 offset )
{
	static bool s_added = false;
	if (!s_added)
	{		
		s_added = true;
		MF_WATCH( "Client Settings/Flora/Ecotype Blur",
			&::getEcotypeBlur,
			&::setEcotypeBlur,
			"Creates a circle, with radius in metres, centered around each flora item. "
			"The ecotype sample point for an item will be somewhere within this circle." );
		MF_WATCH( "Client Settings/Flora/Position Blur",
			&::getPositionBlur,
			&::setPositionBlur,
			"Multiplier for positioning each flora object.  Set to a higher "
			"value to make flora objects encroach upon neighbouring blocks.");
	}

	offset_ = offset;
	this->center( pos );
}

class FloraItem : public Aligned
{
public:
	FloraItem( Ecotype* e, Matrix& o ):
		ecotype_(e),
		objectToWorld_(o)
	{
	};

	Ecotype* ecotype_;
	Matrix	objectToWorld_;
};

typedef std::avector<FloraItem>	FloraItems;

/**
 *	This method fills a vertex allocation with appropriate stuff.
 *
 *	Note all ecotypes are reference counted.  This is so flora knows
 *	when an ecotype is not used, and can thus free its texture memory
 *	for newly activated ecotypes.
 */
void FloraBlock::fill( uint32 numVertsAllowed )
{
	static DogWatch watch( "Flora fill" );
	ScopedDogWatch dw( watch );

	Matrix objectToChunk;
	Matrix objectToWorld;

	//First, check if there are terrain blocks at our location.
	BoundingBox bb(bb_);
	bb.expandSymmetrically( s_positionBlur, 0.f, s_positionBlur );
	Vector2 pos;
	pos.set( bb.minBounds().x, bb.minBounds().z );
	if ( !Terrain::BaseTerrainBlock::findOutsideBlock(Vector3(pos.x,0.f,pos.y) ).pBlock_ )
		return;	//return now.  fill later.
	pos.set( bb.minBounds().x, bb.maxBounds().z );
	if ( !Terrain::BaseTerrainBlock::findOutsideBlock(Vector3(pos.x,0.f,pos.y) ).pBlock_ )
		return;	//return now.  fill later.
	pos.set( bb.maxBounds().x, bb.maxBounds().z );
	if ( !Terrain::BaseTerrainBlock::findOutsideBlock(Vector3(pos.x,0.f,pos.y) ).pBlock_ )
		return;	//return now.  fill later.
	pos.set( bb.maxBounds().x, bb.minBounds().z );
	if ( !Terrain::BaseTerrainBlock::findOutsideBlock(Vector3(pos.x,0.f,pos.y) ).pBlock_ )
		return;	//return now.  fill later.

	Vector2 center( bb_.centre().x, bb_.centre().z );
	Terrain::TerrainFinder::Details details = 
		Terrain::BaseTerrainBlock::findOutsideBlock( 
			Vector3(center.x,0.f,center.y) );
	if (!details.pBlock_)
		return;	//return now.  fill later.

	const Matrix& chunkToWorld = *details.pMatrix_;
	blockID_ = (int)flora_->getTerrainBlockID( chunkToWorld );	
	Matrix worldToChunk = *details.pInvMatrix_;	
	uint32 numVertices = numVertsAllowed;	

	
	FloraItems items;

	//Seed the look up table of random numbers given
	//the center position.  This means we get a fixed
	//set of offsets given a geographical location.
	flora_->seedOffsetTable( center );

	Matrix transform;
	Vector2 ecotypeSamplePt;
	uint32 idx = 0;

	while (1)
	{
		if (!nextTransform(center, objectToWorld, ecotypeSamplePt))
			break;

		objectToChunk.multiply( objectToWorld, worldToChunk );

		//If the spot underneath the flora object is an 'empty' ecotype, then
		//keep that - we don't want anyone encroaching on an uninhabitable spot.
		//Otherwise, lookup the ecotype at the sample point instead.
		Ecotype& accurate = flora_->ecotypeAt(
			Vector2(objectToWorld.applyToOrigin().x,objectToWorld.applyToOrigin().z));		

		FloraItem fi(NULL, objectToWorld);

		if ( accurate.isEmpty() )
		{
			fi.ecotype_ = &accurate;			
		}
		else
		{
			fi.ecotype_ = &flora_->ecotypeAt( ecotypeSamplePt );
		}

		//The ecotypes covering this FloraBlock are not yet fully loaded.
		//We will come back next frame and try again.  This is the main
		//reason for this initial while loop (check the loaded flags).
		if (fi.ecotype_->isLoading_)
		{				
			return;
		}

		items.push_back(fi);

		//Calling generate with NULL as the first parameter indicates we just want to know
		//the number of vertices that would have been generated, but leave the VB intact.
		uint32 nVerts = fi.ecotype_->generate( NULL, idx, numVertices, fi.objectToWorld_, objectToChunk, bb_ );
		if (nVerts == 0)
			break;			
		numVertices -= nVerts;
		if (numVertices == 0)
			break;			
		idx++;
	}

	//Alright so we have all the information for the items, and all the
	//ecotypes are loaded and valid.  Now produce vertices.
	idx = 0;
	numVertices = numVertsAllowed;
	//we will get a properly calculated bounding box from ecotype generation.
	bb_ = BoundingBox::s_insideOut_;
	FloraVertexContainer* pVerts = flora_->pRenderer()->lock( offset_, numVertsAllowed );

	
	//Seed the look up table of random numbers again
	//This means the ecotype generators will end up using
	//exactly the same random numbers as in the first pass.
	flora_->seedOffsetTable( center );

	for (uint32 i=0; i<items.size(); i++)
	{
		FloraItem& fi = items[i];
		Ecotype& ecotype = *fi.ecotype_;
		ecotypes_.push_back( &ecotype );
		ecotype.incRef();
		objectToChunk.multiply( fi.objectToWorld_, worldToChunk );
		uint32 nVerts = ecotype.generate( pVerts, idx, numVertices, fi.objectToWorld_, objectToChunk, bb_ );
		if (nVerts == 0)			
			break;			
		idx++;
		numVertices -= nVerts;
		if (numVertices == 0)
			break;
	}


	//fill the rest of the given vertices with degenerate triangles.
	MF_ASSERT( (numVertices%3) == 0 );
	pVerts->clear( numVertices );
	flora_->pRenderer()->unlock( pVerts );

	if ( bb_ == BoundingBox::s_insideOut_ )
		blockID_ = -1;
	bRefill_ = false;
}


/**
 *	This method should be called to move a flora block to a new position.
 *
 *	@param	c	The new center of the block.
 */
void FloraBlock::center( const Vector2& c )
{
	center_ = c;
	this->invalidate();
}


/**
 *	This method invalidates the flora block.  It sets the bRefill flag to true,
 *	and releases reference counts for all ecoytpes currently used.
 */
void FloraBlock::invalidate()
{
	bb_.setBounds(	Vector3(	center_.x,
								-20000.f, 
								center_.y ),
					Vector3(	center_.x,
								-20000.f, 
								center_.y) );
	blockID_ = -1;

	std::vector<Ecotype*>::iterator it = ecotypes_.begin();
	std::vector<Ecotype*>::iterator end = ecotypes_.end();

	while ( it != end )
	{
		if (*it)
			(*it)->decRef();
		it++;
	}

	ecotypes_.clear();

	bRefill_ = true;
}


void FloraBlock::cull()
{
	if (blockID_ != -1 )
	{
		bb_.calculateOutcode( Moo::rc().viewProjection() );
		culled_ = !!bb_.combinedOutcode();
	}
	else
	{
		culled_ = true;
	}
}


/**
 *	This method calculates the next random transform
 *	for a block.  Each transform is aligned to the
 *	terrain and positions an object on the terrain.
 *
 *	It also returns an ecotype sample point.  This
 *	suggests where to choose an ecotype from (this
 *	itself is blurred slightly, in order to anti-
 *	alias the ecotype data)
 */
bool FloraBlock::nextTransform( const Vector2& center, Matrix& ret, Vector2& retEcotypeSamplePt )
{
	//get the new position.
	Vector2 off = flora_->nextOffset();
	float rotY( flora_->nextRotation() );
	Vector3 pos( center.x + (off.x * s_positionBlur), 0.f, center.y + (off.y * s_positionBlur) );

	//get the new ecotype.  this is blurred so we can encroach on
	//neighbouring ecotypes
	off = flora_->nextOffset();

	//convert s_ecotypeBlur to metres, to make it easy for artists to tweak.
	const float halfBlockWidth = BLOCK_WIDTH / 2.f;
	static float radius = sqrtf( 2.f * (halfBlockWidth * halfBlockWidth) );
	static float offsetToMetres = (1.f/radius);

	retEcotypeSamplePt.x = off.x * offsetToMetres * s_ecotypeBlur + pos.x;
	retEcotypeSamplePt.y = off.y * offsetToMetres * s_ecotypeBlur + pos.z;

	//get the terrain block, and the relative position of
	//pos within the terrain block.
	Vector3 relPos;
	Terrain::BaseTerrainBlockPtr pBlock = flora_->getTerrainBlock( pos, relPos, NULL );

	if ( !pBlock )
	{
		return false;
	}
	else
	{		
		//sit on terrain
		pos.y = pBlock->heightAt( relPos.x, relPos.z );
		if ( pos.y == Terrain::BaseTerrainBlock::NO_TERRAIN )
			return false;
		Vector3 intNorm = pBlock->normalAt( relPos.x, relPos.z );

		//align to terrain
		const PlaneEq eq( intNorm, intNorm.dotProduct( pos ) );
		Vector3 xyz[2];
		xyz[0].set( 0.f, eq.y( 0.f, 0.f ), 0.f );
		xyz[1].set( 0.f, eq.y( 0.f, 1.f ), 1.f );
		Vector3 up = xyz[1] - xyz[0];
		up.normalise();
		ret.lookAt( Vector3( 0.f, 0.f, 0.f ),
			up, Vector3( eq.normal() ) );
		ret.invertOrthonormal();

		//rotate randomly
		Matrix rot;
		rot.setRotateY( rotY );
		ret.preMultiply( rot );

		//move to terrain block local coords
		ret.translation( pos );	
	}

	return true;
}
