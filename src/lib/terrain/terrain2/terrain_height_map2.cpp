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
#include "terrain_height_map2.hpp"
#include "../terrain_settings.hpp"
#include "../terrain_data.hpp"
#include "../terrain_collision_callback.hpp"
#include "../height_map_compress.hpp"
#include "physics2/worldtri.hpp"
#include "moo/png.hpp"

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2("Moo", 0)

#ifndef MF_SERVER
	#include "terrain_vertex_lod.hpp"
#endif

#ifdef EDITOR_ENABLED
static int s_optimiseCollisionAlongZAxis = 0;
#endif//EDITOR_ENABLED

namespace
{
	bool  disableQuadTree = false;

	const float EXTEND_BIAS = 0.001f;

	// This helper function extends a triangle along all it's edges by
	// the amount specified by extend bias.
	void extend( WorldTriangle& wt )
	{
		Vector3 a = wt.v1() - wt.v0();
		Vector3 b = wt.v2() - wt.v1();
		Vector3 c = wt.v0() - wt.v2();
		a *= EXTEND_BIAS;
		b *= EXTEND_BIAS;
		c *= EXTEND_BIAS;

		wt = WorldTriangle( wt.v0() - a + c,
			 wt.v1() - b + a,
			 wt.v2() - c + b, TRIANGLE_TERRAIN );
	}
}

/**
 * Construct a terrain height map.
 *
 * @param	size			the width of the visible part of the height map.
 * @param	visibleOffset	the border of non-visible samples on each edge.
 * @param	unlockCallback	called when unlock() occurs.
 */
/*explicit*/ TerrainHeightMap2::TerrainHeightMap2( 
		uint32 size,
		uint32 lodLevel,
		UnlockCallback* unlockCallback 
	) :
#ifdef EDITOR_ENABLED
	lockCount_(0),
	lockReadOnly_(true),
#endif
	heights_("Terrain/HeightMap2/Image"),
	minHeight_(0),
	maxHeight_(0),
	diagonalDistanceX4_(0.0f),
	unlockCallback_( unlockCallback ),
	quadTree_("Terrain/HeightMap2/QuadTree"),
	quadTreeInited_( false ),
	visibleOffset_( lodLevel > 0 ? 0 : TerrainHeightMap2::DEFAULT_VISIBLE_OFFSET ),
	lodLevel_( lodLevel ),
	internalBlocksWidth_( 0 ),
	internalBlocksHeight_( 0 ),
	internalSpacingX_( 0.f ),
	internalSpacingZ_( 0.f )
{
	BW_GUARD;

	// resize the image to be the correct size
	heights_.resize( size + visibleOffset_ * 2, size + visibleOffset_ * 2 );

	refreshInternalDimensions();

	// Track memory usage
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("Terrain/HeightMap2", (uint)ResourceCounters::SYSTEM),
		(uint)(sizeof(*this)))
}

/**
 *  This is the TerrainHeightMap2 destructor.
 */
/*virtual*/ TerrainHeightMap2::~TerrainHeightMap2()
{
	BW_GUARD;
	// Track memory usage
	RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("Terrain/HeightMap2", (uint)ResourceCounters::SYSTEM),
						 (uint)(sizeof(*this)))
}


/**
 *  This function returns the width of the TerrainMap.
 *
 *  @returns            The width of the TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap2::width() const
{
    return heights_.width();
}


/**
 *  This function returns the height of the TerrainMap.
 *
 *  @returns            The height of the TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap2::height() const
{
    return heights_.height();
}


#ifdef EDITOR_ENABLED
/**
 *  This function creates an TerrainHeightMap2 using the settings in the
 *  DataSection.
 *
 *  @param settings         DataSection containing the terrain settings.
 *  @param error		    Optional output parameter that receives an error
 *							message if an error occurs.
 *  @returns                True if successful, false otherwise.
 */
bool TerrainHeightMap2::create( uint32 size, std::string *error /*= NULL*/ )
{
	BW_GUARD;
	if ( !powerOfTwo( size ) )
	{
		if ( error )
			*error = "Height map size is not a power of two.";
		return false;
	}

	// add space for the extra height values in the invisible area.
	size += internalVisibleOffset() * 2 + 1;

	minHeight_ = 0;
	maxHeight_ = 0;
	heights_.resize( size, size, 0 );

	refreshInternalDimensions();

	return true;
}


/**
 *  This function locks the TerrainMap to memory and enables it for
 *  editing.
 *
 *  @param readOnly     This is a hint to the locking.  If true then
 *                      we are only reading values from the map.
 *  @returns            True if the lock is successful, false
 *                      otherwise.
 */
/*virtual*/ bool TerrainHeightMap2::lock(bool readOnly)
{
    BW_GUARD;
	// Make sure that it wasn't originaly locked for readOnly and
	// trying to nest a non-readOnly lock:
    MF_ASSERT(lockCount_ == 0 ||
		(lockCount_ != 0 &&
			( lockReadOnly_ == readOnly || lockReadOnly_ == false ) ));

	if ( lockCount_ == 0 )
	    lockReadOnly_ = readOnly;
    ++lockCount_;
    return true;
}


/**
 *  This function accesses the TerrainMap as though it is an image.
 *  This function should only be called within a lock/unlock pair.
 *
 *  @returns            The TerrainMap as an image.
 */
/*virtual*/ TerrainHeightMap2::ImageType &TerrainHeightMap2::image()
{
    BW_GUARD;
	MF_ASSERT(lockCount_ != 0);
    return heights_;
}

/**
*  This function unlocks the TerrainMap.  It gives derived classes
*  the chance to repack the underlying data and/or reconstruct
*  DirectX objects.
*
*  @returns            True if the unlocking was successful.
*/
/*virtual*/ bool TerrainHeightMap2::unlock()
{
	BW_GUARD;
	--lockCount_;
	if (lockCount_ == 0)
	{
		if (!lockReadOnly_)
		{
			recalcMinMax();
			invalidateQuadTree();
			if ( unlockCallback_ )
				unlockCallback_->notify();
		}
	}
	return true;
}

/**
*  This function saves the underlying data to a DataSection that can
*  be read back in via load.  You should not call this between
*  lock/unlock pairs.
*
*/
bool TerrainHeightMap2::save( DataSectionPtr pSection ) const
{
	BW_GUARD;
	MF_ASSERT( pSection );

	// Create header at front of buffer
	HeightMapHeader hmh;
	::ZeroMemory(&hmh, sizeof(hmh));
	hmh.magic_       = HeightMapHeader::MAGIC;
	hmh.compression_ = COMPRESS_RAW;
	hmh.width_       = heights_.width();
	hmh.height_      = heights_.height();
	hmh.minHeight_   = minHeight_;
	hmh.maxHeight_   = maxHeight_;
	hmh.version_     = HeightMapHeader::VERSION_ABS_QFLOAT;

	BinaryPtr compHeight = compressHeightMap(heights_);

	std::vector<uint8> data(sizeof(hmh) + compHeight->len());
	::memcpy(&data[          0], &hmh              , sizeof(hmh)      );
	::memcpy(&data[sizeof(hmh)], compHeight->data(), compHeight->len());
	
	// write to binary block
	BinaryPtr bdata = new BinaryBlock(&data[0], data.size(), "Terrain/HeightMap2/BinaryBlock");
	bdata->canZip( false ); // we are already compressed, don't recompress!
	pSection->setBinary( bdata );

	return true;
}

#endif // EDITOR_ENABLED

/**
*  This function accesses the TerrainMap as though it is an image.
*  This function can be called outside a lock/unlock pair.
*
*  @returns            The TerrainMap as an image.
*/
/*virtual*/ TerrainHeightMap2::ImageType const &TerrainHeightMap2::image() const
{
	return heights_;
}

/**
 *  This function loads the TerrainMap from a DataSection that was
 *  saved via TerrainMap::save.  You should not call this between
 *  lock/unlock pairs.
 *
 *  @param dataSection  The DataSection to load from.
 *  @param error		If not NULL and an error occurs then this will
 *                      be set to a reason why the error occurred.
 *  @returns            True if the load was successful.
 */
/*virtual*/ bool
TerrainHeightMap2::load
(
    DataSectionPtr	dataSection,
    std::string*	error  /*= NULL*/
)
{
	BW_GUARD;
	BinaryPtr pBin = dataSection->asBinary();
	if (!pBin)
	{
		if ( error ) *error = "dataSection is not binary";
		return false;
	}

	if (pBin->isCompressed())
		pBin = pBin->decompress();
	
	return loadHeightMap( pBin->data(), pBin->len(), error );
}

bool TerrainHeightMap2::loadHeightMap(	const void*		data, 
										size_t			length,
										std::string*	error /*= NULL*/)
{
	BW_GUARD;
	const HeightMapHeader* header = (const HeightMapHeader*) data;

	if ( header->magic_ != HeightMapHeader::MAGIC )
	{
		if ( error ) *error = "dataSection is not for a TerrainHeightMap2";
		return false;
	}

	if ( header->version_ == HeightMapHeader::VERSION_REL_UINT16_PNG )
	{
		// Height map is stored in compressed format, we'll need to convert the
		// heights back to floats.
		maxHeight_ = convertHeight2Float( *((int32 *)&header->maxHeight_) );
		minHeight_ = convertHeight2Float( *((int32 *)&header->minHeight_) );
		if (maxHeight_ < minHeight_)
			std::swap(minHeight_, maxHeight_);

		// The compressed image data is immediately after the header
		// in the binary block
		BinaryPtr pImageData = 
			new BinaryBlock
			( 
				header + 1,
				length - sizeof( HeightMapHeader ),
				"Terrain/HeightMap2/BinaryBlock"
			);

		// Decompress the image, if this succeeds, we are good to go
		Moo::Image<uint16> image("Terrain/HeightMap2/Image");
		if( !decompressImage( pImageData, image ) )
		{
			if (error ) *error = "The height map data could not be decompressed.";
			return false;
		}

		const uint16* pRelHeights = image.getRow(0);
		heights_ = Moo::Image<float>( header->width_, header->height_, "Terrain/HeightMap2/Image" );

		for ( uint32 i = 0; i < header->height_ * header->width_; i++ )
		{
			*( heights_.getRow(0) + i ) =
				convertHeight2Float( *pRelHeights++ + *((int32 *)&header->minHeight_) );
		}
	}
	// support raw float loading if new format or in editor 
	else if ( header->version_ == HeightMapHeader::VERSION_ABS_FLOAT )
	{
		maxHeight_ = header->maxHeight_;
		minHeight_ = header->minHeight_;
		if (maxHeight_ < minHeight_)
			std::swap(minHeight_, maxHeight_);

		const float* pHeights = (const float*) (header + 1);
		heights_ = Moo::Image<float>(header->width_, header->height_, "Terrain/HeightMap2/Image");
		heights_.copyFrom(pHeights);
	}
	else if ( header->version_ == HeightMapHeader::VERSION_ABS_QFLOAT )
	{
		maxHeight_ = header->maxHeight_;
		minHeight_ = header->minHeight_;
		if (maxHeight_ < minHeight_)
			std::swap(minHeight_, maxHeight_);

		BinaryPtr compData = 
			new BinaryBlock((void *)(header + 1), length - sizeof(*header), "Terrain/HeightMap2/BinaryBlock");
		heights_ = Moo::Image<float>("Terrain/HeightMap2/Image");
		if (!decompressHeightMap(compData, heights_))
		{
			if (error ) *error = "The height map data could not be decompressed.";
			return false;
		}
		MF_ASSERT(heights_.width () == header->width_ );
		MF_ASSERT(heights_.height() == header->height_);
	}
	else
	{
		char buffer[256];
		bw_snprintf( buffer, ARRAY_SIZE(buffer),
			"Invalid TerrainHeightMap2 format %u."
			"You must re-export from World Editor.",
			header->version_ );
		if (error) *error = buffer;
		return false;
	}

	refreshInternalDimensions();

	return true;
}

/**
 *  This function returns the spacing between each sample.
 *
 *  @returns            The spacing between each sample in the x-direction.
 */
/*virtual*/ float TerrainHeightMap2::spacingX() const
{
    return internalSpacingX();
}


/**
 *  This function returns the spacing between each sample.
 *
 *  @returns            The spacing between each sample in the z-direction.
 */
/*virtual*/ float TerrainHeightMap2::spacingZ() const
{
    return internalSpacingZ();
}


/**
 *  This function returns the width of the visible part of the
 *  TerrainMap in poles.
 *
 *  @returns            The width of the visible part of the
 *                      TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap2::blocksWidth() const
{
    return internalBlocksWidth();
}


/**
 *  This function returns the height of the visible part of the
 *  TerrainMap in poles.
 *
 *  @returns            The height of the visible part of the
 *                      TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap2::blocksHeight() const
{
    return internalBlocksHeight();
}


/**
 *  This function returns the number of vertices wide of the visible
 *  part of the TerrainMap.
 *
 *  @returns            The width of the visible part of the
 *                      TerrainMap in terms of vertices.
 */
/*virtual*/ uint32 TerrainHeightMap2::verticesWidth() const
{
    return heights_.width() - ( internalVisibleOffset() * 2 );
}


/**
 *  This function returns the number of vertices high of the visible
 *  part of the TerrainMap.
 *
 *  @returns            The height of the visible part of the
 *                      TerrainMap in terms of vertices.
 */
/*virtual*/ uint32 TerrainHeightMap2::verticesHeight() const
{
    return heights_.height() - ( internalVisibleOffset() * 2 );
}


/**
 *  This function returns the width of the TerrainMap, including
 *  non-visible portions.
 *
 *  @returns            The width of the TerrainMap, including
 *                      non-visible portions.
 */
/*virtual*/ uint32 TerrainHeightMap2::polesWidth() const
{
    return heights_.width();
}


/**
 *  This function returns the height of the TerrainMap, including
 *  non-visible portions.
 *
 *  @returns            The height of the TerrainMap, including
 *                      non-visible portions.
 */
/*virtual*/ uint32 TerrainHeightMap2::polesHeight() const
{
    return heights_.height();
}


/**
 *  This function returns the x-offset of the visible portion of the
 *  HeightMap.
 *
 *  @returns            The x-offset of the first visible column.
 */
/*virtual*/ uint32 TerrainHeightMap2::xVisibleOffset() const
{
    return internalVisibleOffset();
}


/**
 *  This function returns the z-offset of the visible portion of the
 *  HeightMap.
 *
 *  @returns            The z-offset of the first visible row.
 */
/*virtual*/ uint32 TerrainHeightMap2::zVisibleOffset() const
{
    return internalVisibleOffset();
}


/**
 *  This function returns the minimum height in the height map.
 *
 *  @returns            The minimum height in the height map.
 */
/*virtual*/ float TerrainHeightMap2::minHeight() const
{
    return minHeight_;
}


/**
 *  This function returns the maximum height in the height map.
 *
 *  @returns            The maximum height in the height map.
 */
/*virtual*/ float TerrainHeightMap2::maxHeight() const
{
    return maxHeight_;
}


/**
 *  This function gets the height of the terrain at (x, z).
 *
 *  @param x            The x coordinate.
 *  @param z            The z coordinate.
 *
 *  @returns            The height at (x, z).
 */
float TerrainHeightMap2::heightAt(int x, int z) const
{
    return internalHeightAt( x, z );
}


/**
 *  This function determines the height at the given xz position, interpolated
 *	to fit on the lod0 mesh.
 *
 *  @param x            The x coordinate to get the height at.
 *  @param z            The z coordinate to get the height at.
 *  @returns            The height at x, z.
 */
/*virtual*/ float TerrainHeightMap2::heightAt(float x, float z) const
{
	BW_GUARD;
	// calculate the x and z positions on the height map.
	float xs = (x / internalSpacingX()) + internalVisibleOffset();
	float zs = (z / internalSpacingZ()) + internalVisibleOffset();

	// calculate the fractional coverage for the x/z positions
	float xf = xs - floorf(xs);
	float zf = zs - floorf(zs);

	// find the height map start cell of this height
	int32 xOff = int32(floorf(xs));
	int32 zOff = int32(floorf(zs));

	float res = 0;

	// Work out the diagonals for the height map cell, neighbouring cells
	// have opposing diagonals.
	if ((xOff ^ zOff) & 1)
	{
		// the cells diagonal goes from top left to bottom right.
		// Get the heights for the diagonal
		float h01 = heights_.get( xOff, zOff + 1 );
		float h10 = heights_.get( xOff + 1, zOff );

		// Work out which triangle we are in and calculate the interpolated
		// height.
		if ((1.f - xf) > zf)
		{
			float h00 = heights_.get( xOff, zOff );
			res = h00 + (h10 - h00) * xf + (h01 - h00) * zf;
		}
		else
		{
			float h11 = heights_.get( xOff + 1, zOff + 1 );
			res = h11 + (h01 - h11) * (1.f - xf) + (h10 - h11) * (1.f - zf);
		}
	}
	else
	{
		// the cells diagonal goes from top left to bottom right.
		// Get the heights for the diagonal
		float h00 = heights_.get( xOff, zOff );
		float h11 = heights_.get( xOff + 1, zOff + 1 );

		// Work out which triangle we are in and calculate the interpolated
		// height.
		if (xf > zf)
		{
			float h10 = heights_.get( xOff + 1, zOff );
			res = h10 + (h00 - h10) * (1.f - xf) + (h11 - h10) * zf;
		}
		else
		{
			float h01 = heights_.get( xOff, zOff + 1 );
			res = h01 + (h11 - h01) * xf + (h00 - h01) * (1.f - zf);
		}
	}

	return res;
}


/**
 *  This function gets the normal at the given integer coordinates.
 *
 *  @param x            The x coordinate.
 *  @param z            The z coordinate.
 *  @returns            The normal at x, z.
 */
Vector3 TerrainHeightMap2::normalAt(int x, int z) const
{
	BW_GUARD;
	float spaceX = internalSpacingX();
	float spaceZ = internalSpacingZ();

	//This function will not work if the spacing is not
	//equal in the X and Z directions.
	MF_ASSERT( spaceX == spaceZ )

	const float diagonalMultiplier = 0.7071067811f;// sqrt( 0.5 )

	Vector3 ret;

	// Using the cached distance x 4 to avoid doing a sqrtf every time here.
	ret.y = diagonalDistanceX4_ + (spaceX * 2 + spaceZ * 2);

	float val1 = 0.0f;
	float val2 = 0.0f;
	if (x - 1 < 0 || z - 1 < 0 ||
		x + 1 >= int( heights_.width() ) || z + 1 >= int( heights_.height() ))
	{
		// Slower path, need to clamp in one or more axes.
		ret.x = heights_.get( x-1, z ) - heights_.get( x+1, z );
		ret.z = heights_.get( x, z-1 ) - heights_.get( x, z+1 );

		val1 = heights_.get( x-1, z-1 ) - heights_.get( x+1, z+1 );
		val2 = heights_.get( x-1, z+1 ) - heights_.get( x+1, z-1 );
	}
	else
	{
		// Fast path, everything in range, no need to clamp
		TerrainHeightMap2::PixelType * zMin = heights_.getRow( z - 1 ) + x;
		TerrainHeightMap2::PixelType * zCen = heights_.getRow( z ) + x;
		TerrainHeightMap2::PixelType * zMax = heights_.getRow( z + 1 ) + x;

		ret.x = *(zCen - 1) - *(zCen + 1);
		ret.z = *(zMin) - *(zMax);

		val1 = *(zMin - 1) - *(zMax + 1);
		val2 = *(zMax - 1) - *(zMin + 1);
	}

	val1 *= diagonalMultiplier;

	val2 *= diagonalMultiplier;

	ret.x += val1;
	ret.z += val1;

	ret.x += val2;
	ret.z -= val2;
	ret.normalise();

    return ret;
}


/**
 *  This function determines the normal at the given point.
 *
 *  @param x            The x coordinate to get the normal at.
 *  @param z            The z coordinate to get the normal at.
 *  @returns            The normal at x, z.
 */
/*virtual*/ Vector3 TerrainHeightMap2::normalAt(float x, float z) const
{
	BW_GUARD;
	float xf = x/internalSpacingX() + internalVisibleOffset();
	float zf = z/internalSpacingZ() + internalVisibleOffset();

	Vector3 ret;
    float xFrac = xf - ::floorf(xf);
    float zFrac = zf - ::floorf(zf);

	xf -= xFrac;
	zf -= zFrac;

	int xfi = (int)xf;
	int zfi = (int)zf;

	// We do a bilinear interpolation of the normals.  This is a quite
	// expensive operation.  We optimise the case were either the fractional
	// parts of the coordinates are zero since this occurs frequently.
	const float FRAC_EPSILON = 1e-3f;
	if (fabs(xFrac) < FRAC_EPSILON)
	{
		if (fabs(zFrac) < FRAC_EPSILON)
		{
			return normalAt(xfi, zfi);
		}
		else
		{
			Vector3 n1 = normalAt(xfi, zfi    );
			Vector3 n2 = normalAt(xfi, zfi + 1);
			n1 *= (1.0f - zFrac);
			n2 *= zFrac;
			ret = n1;
			ret += n2;
			ret.normalise();
			return ret;
		}
	}
	else if (fabs(zFrac) < FRAC_EPSILON)
	{
		Vector3 n1 = normalAt(xfi    , zfi);
		Vector3 n2 = normalAt(xfi + 1, zfi);
		n1 *= (1.0f - xFrac);
		n2 *= xFrac;
		ret = n1;
		ret += n2;
		ret.normalise();
		return ret;
	}

	Vector3 n1 = normalAt(xfi    , zfi    );
	Vector3 n2 = normalAt(xfi + 1, zfi    );
	Vector3 n3 = normalAt(xfi    , zfi + 1);
	Vector3 n4 = normalAt(xfi + 1, zfi + 1);

	n1 *= (1 - xFrac)*(1 - zFrac);
	n2 *= (    xFrac)*(1 - zFrac);
	n3 *= (1 - xFrac)*(	   zFrac);
	n4 *= (    xFrac)*(	   zFrac);

	ret  = n1;
	ret += n2;
	ret += n3;
	ret += n4;

	ret.normalise();
	return ret;
}


/**
 *  This function does line-segment intersection tests with the terrain.
 *
 *  @param start		The start of the line-segment.
 *  @param end			The end of the line-segment.
 *  @param pCallback    The terrain collision callback.
 */
bool TerrainHeightMap2::collide
(
    Vector3             const &start,
    Vector3             const &end,
    TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	// Check the distance traveled on the x and z axis if they are smaller
	// than the threshold use the straight collision method
	float xDist = fabsf(end.x - start.x);
	float zDist = fabsf(end.z - start.z);

	float quadtreeThreshold = this->quadtreeThreshold();

#ifdef EDITOR_ENABLED
	if (s_optimiseCollisionAlongZAxis && almostEqual( start.z, end.z ) && pCallback)
	{
		return hmCollideAlongZAxis( start, start, end, pCallback );
	}
#endif//EDITOR_ENABLED

	if ((xDist < quadtreeThreshold &&
		zDist < quadtreeThreshold) ||
		disableQuadTree )
	{
		return hmCollide( start, start, end, pCallback );
	}

	// If the quad tree has not been initialised, init it
	if (!quadTreeInited_)
		recalcQuadTree();
    return quadTree_.collide(start, end, this, pCallback);
}

/**
 *  This function does prism intersection tests with the terrain.
 *
 *  @param start		The start of the prism.
 *  @param end			The first point on end triangle of prism.
 *  @param pCallback    The terrain collision callback.
 */
bool TerrainHeightMap2::collide
(
    WorldTriangle       const &start,
    Vector3             const &end,
    TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	Vector3 delta = end - start.v0();

	float quadtreeThreshold = this->quadtreeThreshold();

	// Check the distance covered on the x and z axis if they are smaller
	// than the threshold use the straight collision method
	if ((fabsf(delta.x) < quadtreeThreshold &&
		fabsf(delta.z) < quadtreeThreshold) ||
		disableQuadTree)
	{
		// if the initial check fits within the quadtree create a bounding box
		// that covers the prism and collide with the height map
		BoundingBox bb;
		bb.addBounds(start.v0());
		bb.addBounds(start.v1());
		bb.addBounds(start.v2());

		Vector3 bbSize = bb.maxBounds() - bb.minBounds();

		if ((fabsf(bbSize.x) < quadtreeThreshold &&
			fabsf(bbSize.z) < quadtreeThreshold)
			|| disableQuadTree)
		{
			bb.addBounds(end);
			bb.addBounds(start.v1() + delta);
			bb.addBounds(start.v2() + delta);

			return hmCollide(start, end, bb.minBounds().x, bb.minBounds().z,
				bb.maxBounds().x, bb.maxBounds().z, pCallback);
		}
	}

	// If the quad tree has not been initialised, init it
	if (!quadTreeInited_)
		recalcQuadTree();
	return quadTree_.collide(start, end, this, pCallback);
}


namespace // Anonymous
{
	// Epsilon value for comparing collision results
	const float COLLISION_EPSILON = 0.0001f;
}


/**
 *  This function does line-segment intersection tests with the terrain
 *	using the height map values
 *
 *  @param originalStart	The source of the unclipped line-segment.
 *  @param start			The start of the line-segment.
 *  @param end				The end of the line-segment.
 *  @param pCallback		The terrain collision callback.
 *	@return true if collision should stop
 */
bool TerrainHeightMap2::hmCollide
(
    Vector3             const &originalStart,
    Vector3             const &start,
    Vector3             const &end,
    TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	Vector3 s = start;
	Vector3 e = end;

    const float BOUNDING_BOX_EPSILON = 0.1f;

	BoundingBox bb( Vector3( 0, minHeight(), 0 ),
		Vector3( BLOCK_SIZE_METRES, maxHeight(),
        BLOCK_SIZE_METRES ) );

	const Vector3 gridAlign( float(blocksWidth()) / BLOCK_SIZE_METRES, 1.f,
		float(blocksHeight()) / BLOCK_SIZE_METRES );

	if (bb.clip(s, e, BOUNDING_BOX_EPSILON ))
	{
		s = s * gridAlign;
		e = e * gridAlign;

		Vector3 dir = e - s;
		if (!(almostZero(dir.x) && almostZero(dir.z)))
		{
			dir *= 1.f / sqrtf(dir.x * dir.x + dir.z * dir.z);
		}

		int32 gridX = int32(floorf(s.x));
		int32 gridZ = int32(floorf(s.z));

		int32 gridEndX = int32(floorf(e.x));
		int32 gridEndZ = int32(floorf(e.z));

		int32 gridDirX = 0;
		int32 gridDirZ = 0;

		if (dir.x < 0) gridDirX = -1; else if (dir.x > 0) gridDirX = 1;
		if (dir.z < 0) gridDirZ = -1; else if (dir.z > 0) gridDirZ = 1;

		while ((gridX != gridEndX &&
			!almostEqual( s.x, e.x, COLLISION_EPSILON)) ||
			(gridZ != gridEndZ &&
			!almostEqual( s.z, e.z, COLLISION_EPSILON )))
		{
			if (checkGrid( gridX, gridZ,
				originalStart, end, pCallback))
				return true;

			float xDistNextGrid = gridDirX < 0 ? float(gridX) - s.x : float(gridX + 1) - s.x;
			float xDistEnd = e.x - s.x;

			bool xEnding = fabsf(xDistEnd) < fabsf(xDistNextGrid);

			float xDist = xEnding ? xDistEnd : xDistNextGrid;

			float zDistNextGrid = gridDirZ < 0 ? float(gridZ) - s.z : float(gridZ + 1) - s.z;
			float zDistEnd = e.z - s.z;

			bool zEnding = fabsf(zDistEnd) < fabsf(zDistNextGrid);

			float zDist = zEnding ? zDistEnd : zDistNextGrid;

			float a = xDist / dir.x;
			float b = zDist / dir.z;

			if (gridDirZ == 0 ||
				a < b)
			{
				if (xEnding)
				{
					s = e;
				}
				else
				{
					gridX += gridDirX;
					s.x += a * dir.x;
					s.y += a * dir.y;
					s.z += a * dir.z;
				}
			}
			else
			{
				if (zEnding)
				{
					s = e;
				}
				else
				{
					gridZ += gridDirZ;
					s.x += b * dir.x;
					s.y += b * dir.y;
					s.z += b * dir.z;
				}
			}
		}
		if (checkGrid( gridX, gridZ,
			originalStart, end, pCallback))
			return true;
	}
	return false;
}


#ifdef EDITOR_ENABLED
/**
 *  This function does line-segment intersection tests with the terrain
 *	using the height map values along Z axis. We have several optimisations
 *	into it:
 *	a. Remove quad tree searching because it is useless for collision along
 *		Z axis.
 *	b. Check for height range before doing the costly world triangle test.
 *	c. Don't extend the triangle before testing to speed up a bit.
 *
 *  @param originalStart	The source of the unclipped line-segment.
 *  @param start			The start of the line-segment.
 *  @param end				The end of the line-segment.
 *  @param pCallback		The terrain collision callback.
 *	@return true if collision should stop
 */
bool TerrainHeightMap2::hmCollideAlongZAxis
(
    Vector3             const &originalStart,
    Vector3             const &start,
    Vector3             const &end,
    TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;

	Vector3 s = start;
	Vector3 e = end;

    const float BOUNDING_BOX_EPSILON = 0.1f;

	BoundingBox bb( Vector3( 0, minHeight(), 0 ),
		Vector3( BLOCK_SIZE_METRES, maxHeight(),
        BLOCK_SIZE_METRES ) );

	const Vector3 gridAlign( float(blocksWidth()) / BLOCK_SIZE_METRES, 1.f,
		float(blocksHeight()) / BLOCK_SIZE_METRES );

	if (bb.clip(s, e, BOUNDING_BOX_EPSILON ))
	{
		s = s * gridAlign;
		e = e * gridAlign;

		int32 gridX = int32(floorf(s.x));
		int32 gridEndX = int32(floorf(e.x));
		int32 gridDirX;

		int32 gridZ = int32(floorf(s.z));

		if (e.x - s.x < 0)
		{
			gridDirX = -1;
		}
		else
		{
			gridDirX = 1;
		}

		// We only need one triangle because in terrain shadow calculation
		// we only care if it hits.
		WorldTriangle tri;
		Vector3 dir = end - originalStart;
		float dist = dir.length();

		dir *= 1.f / dist;
		gridEndX += gridDirX;

		float* rowZ = heights_.getRow( gridZ + internalVisibleOffset() ) + gridX + internalVisibleOffset();
		float* rowZPlusOne = heights_.getRow( gridZ + 1 + internalVisibleOffset() ) + gridX + internalVisibleOffset();

		// y = ax + b
		float a = ( end.y - start.y ) / ( end.x - start.x );
		float b = ( start.y * end.x - start.x * end.y ) / ( end.x - start.x );
		int adjust = ( a > 0 ) ? 1 : 0;

		for (; gridX != gridEndX; gridX += gridDirX)
		{
			float minY = a * ( float( gridX + adjust ) * internalSpacingX() ) + b;

			float bottomLeftHeight = *rowZ;
			float bottomRightHeight = rowZ[ 1 ];
			float topLeftHeight = *rowZPlusOne;
			float topRightHeight = rowZPlusOne[ 1 ];

			rowZ += gridDirX;
			rowZPlusOne += gridDirX;

			if (bottomLeftHeight < minY && bottomRightHeight < minY &&
				topLeftHeight < minY && topRightHeight < minY)
			{
				continue;
			}

			// Construct our four vertices that make up this grid cell
			Vector3 bottomLeft( float(gridX) * internalSpacingX(),
				bottomLeftHeight,
				float(gridZ) * internalSpacingZ() );

			Vector3 bottomRight = bottomLeft;
			bottomRight.x += internalSpacingX();
			bottomRight.y = bottomRightHeight;

			Vector3 topLeft = bottomLeft;
			topLeft.z += internalSpacingZ();
			topLeft.y = topLeftHeight;

			Vector3 topRight = topLeft;
			topRight.x += internalSpacingX();
			topRight.y = topRightHeight;

			// Create the world triangles for the collision intersection tests
			if ((gridX ^ gridZ) & 1)
			{
				tri = WorldTriangle( bottomLeft, topLeft, bottomRight, 
					TRIANGLE_TERRAIN );
			}
			else
			{
				tri = WorldTriangle( bottomLeft, topLeft, topRight, 
					TRIANGLE_TERRAIN );
			}

			// Check if we intersect with either triangle
			float dist1 = dist;

			if (tri.intersects( originalStart, dir, dist1 ) &&
				pCallback->collide( tri, dist1 ))
			{
				return true;
			}
			else
			{
				dist1 = dist;

				if ((gridX ^ gridZ) & 1)
				{
					tri = WorldTriangle( topLeft, topRight, bottomRight, 
						TRIANGLE_TERRAIN );
				}
				else
				{
					tri = WorldTriangle( topRight, bottomRight, bottomLeft, 
						TRIANGLE_TERRAIN );
				}

				if (tri.intersects( originalStart, dir, dist1 ) &&
					pCallback->collide( tri, dist1 ))
				{
					return true;
				}
			}
		}
	}
	return false;
}


#endif//EDITOR_ENABLED

/**
 *  This function collides a prism with a rect in the heightmap
 *
 *	@param triStart the start triangle of the prism
 *	@param triEnd the end location of the prism
 *	@param xStart the x start location of our rect
 *	@param zStart the z start location of our rect
 *	@param xEnd the x end location of our rect
 *	@param zEnd the z end location of our rect
 *	@param pCallback the collision callback
 *
 *	@return true if collision should stop, false otherwise.
 */
bool TerrainHeightMap2::hmCollide
(
	WorldTriangle		const &triStart,
	Vector3             const &triEnd,
	float				xStart,
	float				zStart,
	float				xEnd,
	float				zEnd,
	TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	const float xMul = 1.f / internalSpacingX();
	const float zMul = 1.f / internalSpacingZ();

	int xs = int(xStart * xMul);
	int xe = int(ceilf(xEnd * xMul));
	int zs = int(zStart * zMul);
	int ze = int(ceilf(zEnd * zMul));

	for (int z = zs; z < ze; z++)
	{
		for (int x = xs; x < xe; x++)
		{
			if ( checkGrid( x, z, triStart, triEnd, pCallback ) )
			{
				return true;
			}
		}
	}

	return false;
}


#ifdef EDITOR_ENABLED
/**
 *  This function recalculates the minimum and maximum values of the
 *  height map.
 */
void TerrainHeightMap2::recalcMinMax()
{
	BW_GUARD;
	maxHeight_ = -std::numeric_limits<ImageType::PixelType>::max();
    minHeight_ = +std::numeric_limits<ImageType::PixelType>::max();

    for (uint32 y = 0; y < heights_.height(); ++y)
    {
        const float *p = heights_.getRow(y);
        const float *q = p + heights_.width();
        while (p != q)
        {
            maxHeight_ = std::max(*p, maxHeight_);
            minHeight_ = std::min(*p, minHeight_);
            ++p;
        }
    }
}
#endif

/**
 *  This function recalculates the quad-tree.
 */
void TerrainHeightMap2::recalcQuadTree() const
{
    BW_GUARD;
	quadTree_.init
    (
        this,
        0, 0, blocksWidth(), blocksHeight(),
        0.f, 0.f, BLOCK_SIZE_METRES, BLOCK_SIZE_METRES,
		this->quadtreeThreshold()

    );
	quadTreeInited_ = true;
}

/**
*  This function invalidates the quad-tree.
*/
void TerrainHeightMap2::invalidateQuadTree() const
{
	BW_GUARD;
	quadTree_ = TerrainQuadTreeCell("Terrain/HeightMap2/QuadTree");
	quadTreeInited_ = false;
}


/**
 *  Check to see if the given line-segment goes through any of the triangles
 *  at gridX, gridZ.
 *
 *  @param gridX        The x coordinate of the triangles.
 *  @param gridZ        The z coordinate of the triangles.
 *  @param start		The start of the line-segment.
 *  @param end			The end of the line-segment.
 *  @param pCallback    The collision callback.
 */
bool
TerrainHeightMap2::checkGrid
(
    int					gridX,
    int					gridZ,
    Vector3             const &start,
    Vector3             const &end,
    TerrainCollisionCallback* pCallback
) const
{
	BW_GUARD;
	bool res = false;

	if (gridX >= 0 && gridX < (int)verticesWidth() &&
		gridZ >= 0 && gridZ < (int)verticesHeight() )
	{
		// Construct our four vertices that make up this grid cell
		Vector3 bottomLeft( float(gridX) * internalSpacingX(),
			heightAt(gridX, gridZ),
			float(gridZ) * internalSpacingZ() );

		Vector3 bottomRight = bottomLeft;
		bottomRight.x += internalSpacingX();
		bottomRight.y = heightAt(gridX + 1, gridZ);

		Vector3 topLeft = bottomLeft;
		topLeft.z += spacingZ();
		topLeft.y =  heightAt(gridX, gridZ + 1);

		Vector3 topRight = topLeft;
		topRight.x += internalSpacingX();
		topRight.y =  heightAt(gridX + 1, gridZ + 1);

		// Construct our triangles
		WorldTriangle triA;
		WorldTriangle triB;

		// Create the world triangles for the collision intersection tests
		if ((gridX ^ gridZ) & 1)
		{
			triA = WorldTriangle( bottomLeft, topLeft, bottomRight, 
				TRIANGLE_TERRAIN );
			triB = WorldTriangle( topLeft, topRight, bottomRight, 
				TRIANGLE_TERRAIN );
		}
		else
		{
			triA = WorldTriangle( bottomLeft, topLeft, topRight, 
				TRIANGLE_TERRAIN );
			triB = WorldTriangle( topRight, bottomRight, bottomLeft, 
				TRIANGLE_TERRAIN );
		}

		// Extend the world triangles a bit to make sure edge cases give a 
		// positive result
		extend( triA );
		extend( triB );

		Vector3 dir = end - start;
		float dist = dir.length();
		float dist2 = dist;
		dir *= 1.f / dist;

		// Check if we intersect with either triangle
		bool intersectsA = triA.intersects( start, dir, dist );
		bool intersectsB = triB.intersects( start, dir, dist2 );

		// If we have a callback pass colliding triangles to it
		// We always pass in triangles from closest to furthest away
		// so that we can stop colliding if the callback only wants
		// near collisions.
		if (pCallback)
		{
			if (intersectsA && intersectsB)
			{
				if (dist < dist2)
				{
					res = pCallback->collide( triA, dist );
					if (!res)
					{
						res = pCallback->collide( triB, dist2 );
					}
				}
				else
				{
					res = pCallback->collide( triB, dist2 );
					if (!res)
					{
						res = pCallback->collide( triA, dist );
					}
				}
			}
			else if (intersectsA)
			{
				res = pCallback->collide( triA, dist );
			}
			else if (intersectsB)
			{
				res = pCallback->collide( triB, dist2 );
			}
		}
		else
		{
			res = intersectsA || intersectsB;
		}
	}

	return res;
}

/**
*  Check to see if the given prism goes through any of the triangles
*  at gridX, gridZ.
*
*  @param gridX			The x coordinate of the triangles.
*  @param gridZ			The z coordinate of the triangles.
*  @param start			The start of prism.
*  @param end			The first point on end triangle of prism.
*  @param pCallback		The collision callback.
*/
bool
TerrainHeightMap2::checkGrid
(
	int				gridX,
	int				gridZ,
	WorldTriangle	const &start,
	Vector3			const &end,
	TerrainCollisionCallback* pCallback
) const
{
	BW_GUARD;
	bool res = false;

	if (gridX >= 0 && gridX < (int)blocksWidth() &&
		gridZ >= 0 && gridZ < (int)blocksHeight() )
	{
		// Construct our four vertices that make up this grid cell
		Vector3 bottomLeft( float(gridX) * internalSpacingX(),
			internalHeightAt(gridX, gridZ),
			float(gridZ) * internalSpacingZ() );

		Vector3 bottomRight = bottomLeft;
		bottomRight.x += internalSpacingX();
		bottomRight.y = internalHeightAt(gridX + 1, gridZ);

		Vector3 topLeft = bottomLeft;
		topLeft.z += internalSpacingZ();
		topLeft.y =  internalHeightAt(gridX, gridZ + 1);

		Vector3 topRight = topLeft;
		topRight.x += internalSpacingX();
		topRight.y =  internalHeightAt(gridX + 1, gridZ + 1);

		// Construct our triangles
		WorldTriangle triA;
		WorldTriangle triB;

		if ((gridX ^ gridZ) & 1)
		{
			triA = WorldTriangle( bottomLeft, topLeft,
						bottomRight, TRIANGLE_TERRAIN );
			triB = WorldTriangle( topLeft, topRight,
						bottomRight, TRIANGLE_TERRAIN );
		}
		else
		{
			triA = WorldTriangle( bottomLeft, topLeft,
						topRight, TRIANGLE_TERRAIN );
			triB = WorldTriangle( topRight, bottomRight,
						bottomLeft, TRIANGLE_TERRAIN );
		}

		// Get the offset of other prism end for the intersection
		Vector3 offset = end - start.v0();

		// Check if we intersect with either triangle
		bool intersectsA = triA.intersects( start, offset );
		bool intersectsB = triB.intersects( start, offset );

		// TODO: calculate proper distance
		float dist = 0.f;

		// If we have a callback pass colliding triangles to it
		if (pCallback)
		{
			if (intersectsA && intersectsB)
			{
				res = pCallback->collide( triA, dist );
				if (!res)
				{
					res = pCallback->collide( triB, dist );
				}
			}
			else if (intersectsA)
			{
				res = pCallback->collide( triA, dist );
			}
			else if (intersectsB)
			{
				res = pCallback->collide( triB, dist );
			}
		}
		else
		{
			res = intersectsA || intersectsB;
		}
	}

	return res;
}

/**
 * Returns correct name for a height map, given base name and a lod index.
 */
std::string TerrainHeightMap2::getHeightSectionName(const std::string&	base, 
													uint32				lod )
{
	BW_GUARD;
	std::string name = base;
	
	if ( lod > 0 )
	{
		char buffer[64];
		bw_snprintf( buffer, ARRAY_SIZE(buffer), "%u", lod );
		name += buffer;
	}

	return name;
}

#ifdef EDITOR_ENABLED
/**
 * Enable/Disable the optimisation on collisions along zaxis
 */
void TerrainHeightMap2::optimiseCollisionAlongZAxis( bool enable )
{
	if (enable)
	{
		++s_optimiseCollisionAlongZAxis;
	}
	else
	{
		--s_optimiseCollisionAlongZAxis;
	}
}
#endif//EDITOR_ENABLED

#ifndef MF_SERVER

HeightMapResource::HeightMapResource(	const std::string&	heightsSectionName, 
									 uint32				lodLevel ) :
terrainSectionName_( heightsSectionName ),
lodLevel_( lodLevel )
{
	BW_GUARD;
}

bool HeightMapResource::load()
{
	BW_GUARD;

	std::string heightSectionName = TerrainHeightMap2::getHeightSectionName(
											terrainSectionName_ + "/heights", 
											lodLevel_ );
	DataSectionPtr pHeights = 
		BWResource::instance().openSection( heightSectionName );
	
	if (!pHeights)
	{
		WARNING_MSG("Can't open heights section '%s'.\n", heightSectionName.c_str() );
		return false;
	}

	// Create the height map.
	TerrainHeightMap2* pDHM = new TerrainHeightMap2( 0, lodLevel_ );
	std::string error;
	
	// load, then assign once fully loaded.
	if ( !pDHM->load( pHeights, &error ) )
	{
		WARNING_MSG("%s\n", error.c_str() );
		delete pDHM;
		return false;
	}

	object_ = pDHM;

	return true;
}

#endif // MF_SERVER
