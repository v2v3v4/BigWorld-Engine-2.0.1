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

#include "chunk_format.hpp"
#include "chunk_space.hpp"
#include "geometry_mapping.hpp"

#include "cstdmf/guard.hpp"

#include "math/boundbox.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/datasection.hpp"

#if UMBRA_ENABLE
#include "chunk_umbra.hpp"
#endif

#ifndef MF_SERVER
#include "chunk_manager.hpp"
#endif

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: GeometryMapping
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *	The path supplied should _not_ end with a slash.
 */
GeometryMapping::GeometryMapping( ChunkSpacePtr pSpace, const Matrix & m,
		const std::string & path,
		DataSectionPtr pSettings ) :
	pSpace_( pSpace ),
	mapper_( m ),
	path_( path + "/" ),
	name_( path.substr( 1 + std::min(
		path.find_last_of( '/' ), path.find_last_of( '\\' ) ) ) ),
	pDirSection_( NULL ),
	condemned_( false ),
	singleDir_( false )
{
	BW_GUARD;
	if (!pSettings)
	{
		pSettings = this->openSettings( path );
	}

	invMapper_.invert( mapper_ );
	char addrStr[32];
#ifndef _WIN32
	bw_snprintf( addrStr, sizeof( addrStr ), "@%p", this );
#else
	bw_snprintf( addrStr, sizeof( addrStr ), "@0x%p", this );
#endif

	name_ += addrStr;	// keep the name unique

	if (pSettings)
	{
		// read our grid bounds...
		minLGridX_ = pSettings->readInt( "bounds/minX" );
		minLGridY_ = pSettings->readInt( "bounds/minY" );
		maxLGridX_ = pSettings->readInt( "bounds/maxX" );
		maxLGridY_ = pSettings->readInt( "bounds/maxY" );
		BoundingBox sbb;
		gridToBounds( minLGridX_, minLGridY_, maxLGridX_, maxLGridY_, sbb );

		// ...as mapped by our mapper
		sbb.transformBy( mapper_ );
		boundsToGrid( sbb, minGridX_, minGridY_, maxGridX_, maxGridY_ );

		singleDir_ = pSettings->readBool( "singleDir", false );

#if UMBRA_ENABLE
		// read whether we want to use umbra occlusion or not for this space
		// TODO: Remove this and make a proper fix for occlusion culling in arid space
		UmbraHelper::instance().occlusionCulling(
				pSettings->readBool( "umbraOcclusion", true ) );
#endif


		// tell the space about this exciting new opportunity
		pSpace->mappingSettings( pSettings );
	}
	else
	{
		pSpace_ = NULL;	// we failed
	}
}


/**
 *	Destructor.
 */
GeometryMapping::~GeometryMapping()
{
	BW_GUARD;
	MF_ASSERT( !pSpace_ || !pSpace_->hasChunksInMapping( this ) );

	MF_ASSERT_DEV( this->condemned() );

	pSpace_ = NULL;
}


/*
 *	Marks this mapping as condemned (has been removed) and notifies the
 *	ChunkManager that this has occured.
 */
void GeometryMapping::condemn()
{
	if (!condemned_)
	{
#ifndef MF_SERVER
		ChunkManager::instance().mappingCondemned( this );
#endif
		condemned_ = true;
	}
}


/**
 *	pDirSection accessor.
 *
 *	Should only be called from loading thread as it may block,
 *	especially if chunk data is out of date.
 */
DataSectionPtr GeometryMapping::pDirSection()
{
	BW_GUARD;
	if (!pDirSection_)
		pDirSection_ = BWResource::openSection(
			path_.substr( 0, path_.size()-1 ) );

	return pDirSection_;
}


/**
 *	This method opens the settings file associated with the input path to space
 *	geometry.
 */
DataSectionPtr GeometryMapping::openSettings( const std::string & path )
{
	BW_GUARD;
	DataSectionPtr pSettings = BWResource::openSection( path );
	if (pSettings)
	{
		pSettings = pSettings->openSection( SPACE_SETTING_FILE_NAME );
	}

	return pSettings;
}


/**
 *	This method calculates local space (mapped) grid coordinates
 *	given the world grid coordinates.  It reports the results via
 *	pass-by-reference values.
 *
 *	@param	x		world x grid coordinate.
 *	@param	y		world y grid coordinate.
 *	@param	lx		[out] local x grid coordinate.
 *	@param	ly		[out] local y grid coordinate.
 */
void GeometryMapping::gridToLocal( int x, int y, int& lx, int& ly ) const
{
	//add 50 to make sure the point is fully in the centre of the chunk before we map / rotate it
	//this will give us a much better chance of getting the correct grid location in the next step.
	Vector3 worldPt( BaseChunkSpace::gridToPoint(x) + GRID_RESOLUTION / 2, 0.f,
		BaseChunkSpace::gridToPoint(y) + GRID_RESOLUTION / 2 );
	Vector3 localPt = invMapper_.applyPoint( worldPt );
	lx = BaseChunkSpace::pointToGrid(localPt.x);
	ly = BaseChunkSpace::pointToGrid(localPt.z);
}


/**
 *	This method calculates the grid bounds of the given bounding box.
 *	The returned grid bounds are in the same space as the bounding box,
 *	ie. doesn't differentiate between local or world coordinates.
 */
void GeometryMapping::boundsToGrid(
	const BoundingBox& bb,
	int& minGridX, int& minGridZ,
	int& maxGridX, int& maxGridZ ) const
{
	minGridX = int(floorf( bb.minBounds().x / GRID_RESOLUTION + 0.5f ));
	maxGridX = int(floorf( bb.maxBounds().x / GRID_RESOLUTION + 0.5f ) - 1);
	minGridZ = int(floorf( bb.minBounds().z / GRID_RESOLUTION + 0.5f ));
	maxGridZ = int(floorf( bb.maxBounds().z / GRID_RESOLUTION + 0.5f ) - 1);
}


/**
 *	This method calculates a bounding box, based on the given grid
 *	bounds.  The returned bounding box is in the same space as the grid coords,
 *	ie. doesn't differentiate between local or world coordinates.
 */
void GeometryMapping::gridToBounds(
	int minGridX, int minGridY,
	int maxGridX, int maxGridY,
	BoundingBox& retBB ) const
{
	retBB.setBounds(
		Vector3( float(minGridX), 0, float(minGridY) ) * GRID_RESOLUTION,
		Vector3( float(maxGridX+1), 0, float(maxGridY+1) ) * GRID_RESOLUTION );
}


/**
 *	This method checks whether the given grid coordinates are within our
 *	local grid bounds.
 *	@param	gridX			local X grid coordinate to check
 *	@param	gridZ			local Z grid coordinate to check
 *	@return bool			true iff we contain both gridX and gridZ
 */
bool GeometryMapping::inLocalBounds( const int gridX, const int gridZ ) const
{
	return (gridX >= minLGridX_ && gridX <= maxLGridX_ &&
		gridZ >= minLGridY_ && gridZ <= maxLGridY_);
}


/**
 *	This method checks whether the given grid coordinates are within our
 *	world grid bounds.
 *	@param	gridX			world X grid coordinate to check
 *	@param	gridZ			world Z grid coordinate to check
 *	@return bool			true iff we contain both gridX and gridZ
 */
bool GeometryMapping::inWorldBounds( const int gridX, const int gridZ ) const
{
	return (gridX >= minGridX_ && gridX <= maxGridX_ &&
		gridZ >= minGridY_ && gridZ <= maxGridY_);
}


/**
 *	This method returns the identifier for the outside chunk in which the
 *	given point would lie. If the checkBounds parameter is true, and the
 *	point does not lie within the bounds of the mapping, then the empty
 *	string is returned instead.
 */
std::string GeometryMapping::outsideChunkIdentifier( const Vector3 & localPoint,
	bool checkBounds ) const
{
	BW_GUARD;
	int gridX = int(floorf( localPoint.x / GRID_RESOLUTION ));
	int gridZ = int(floorf( localPoint.z / GRID_RESOLUTION ));

	if (checkBounds && !inLocalBounds(gridX, gridZ))
	{
		return std::string();
	}

	return ChunkFormat::outsideChunkIdentifier( gridX, gridZ, singleDir_ );
}

/**
 *	This method returns the identifier for the outside chunk in which the
 *	given point would lie. If the checkBounds parameter is true, and the
 *	point does not lie within the bounds of the mapping, then the empty
 *	string is returned instead.
 */
std::string GeometryMapping::outsideChunkIdentifier( int x, int z,
	bool checkBounds ) const
{
	BW_GUARD;
	if (checkBounds && !inLocalBounds(x, z))
	{
		return std::string();
	}

	return ChunkFormat::outsideChunkIdentifier( x, z, singleDir_ );
}

class HexLookup
{
public:
	HexLookup()
	{
		for( int i = 0; i < 256; ++i )
		{
			hexLookup_[ i ] = 0;
			hexFactor_[ i ] = 0;
		}

		for (unsigned char i='0'; i<='9'; i++)
		{
			hexLookup_[i] = (uint16)(i-'0');
			hexFactor_[i] = 1;
		}

		for (unsigned char i='a'; i<='f'; i++)
		{
			hexLookup_[i] = (uint16)(i-'a' + 10);
			hexFactor_[i] = 1;
		}

		for (unsigned char i='A'; i<='F'; i++)
		{
			hexLookup_[i] = (uint16)(i-'A' + 10);
			hexFactor_[i] = 1;
		}
	};

	//done this way to avoid conditionals.
	//ascii-hex numbers directly looked up, while the factors are anded.
	//if any ascii-hex lookup is invalid, one of the factors will be 0 and thus
	//anded out, all will be invalid.
	bool fromHex( const unsigned char* f, int16& value )
	{
		value = (hexLookup_[f[3]] +
			(hexLookup_[f[2]]<<4) +
			(hexLookup_[f[1]]<<8) +
			(hexLookup_[f[0]]<<12));
		int8 factor = hexFactor_[f[0]] & hexFactor_[f[1]] & hexFactor_[f[2]] & hexFactor_[f[3]];
		return (!!factor);
	}

	uint16 hexLookup_[256];
	uint8 hexFactor_[256];
};

/*static*/ bool GeometryMapping::gridFromChunkName( const std::string& chunkName, int16& x, int16& z )
{
	BW_GUARD;
	static HexLookup hexLookup;

	//Assume name is dir/path/path/.../chunkNameo for speed
	const unsigned char* f = (const unsigned char*)chunkName.c_str();
	while ( *f ) f++;

	//subtract "xxxxxxxxo" which is always the last part of an
	//outside chunk identifier.
	if(*(f-1) == 'o')
	{
		return (hexLookup.fromHex(f-9,x) && hexLookup.fromHex(f-5,z));
	}

	return false;
}

// geometry_mapping.cpp
