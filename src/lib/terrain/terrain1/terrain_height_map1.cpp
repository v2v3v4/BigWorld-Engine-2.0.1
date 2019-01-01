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
#include "terrain_height_map1.hpp"

#include "terrain_block1.hpp"
#include "../terrain_data.hpp"
#include "../terrain_collision_callback.hpp"

#include "cstdmf/debug.hpp"
#include "physics2/worldtri.hpp"

DECLARE_DEBUG_COMPONENT2("Moo", 0)

using namespace Terrain;


/**
 *  This is the TerrainHeightMap1 constructor.
 *
 *  @param terrainBlock1    The parent TerrainBlock1.
 */
/*explicit*/ TerrainHeightMap1::TerrainHeightMap1(
		CommonTerrainBlock1 &terrainBlock1 ):
#ifdef EDITOR_ENABLED
	lockCount_(0),
	lockReadOnly_(true),
#endif
	minHeight_(0),
	maxHeight_(0),
	diagonalDistanceX4_(0.0f),
    terrainBlock1_(&terrainBlock1)
{
	// Cache the diagonal distance X 4, to improve normalAt performance.
	diagonalDistanceX4_ = sqrtf( spacingX() * spacingZ() * 2 ) * 4;
}


/**
 *  This is the TerrainHeightMap1 destructor.
 */
/*virtual*/ TerrainHeightMap1::~TerrainHeightMap1()
{
}


/**
 *  This function returns the width of the TerrainMap.
 *
 *  @returns            The width of the TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap1::width() const
{
    return heights_.width();
}


/**
 *  This function returns the height of the TerrainMap.
 *
 *  @returns            The height of the TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap1::height() const
{
    return heights_.height();
}


#ifdef EDITOR_ENABLED
/**
 *  This function locks the TerrainMap to memory and enables it for
 *  editing.
 *
 *  @param readOnly     This is a hint to the locking.  If true then
 *                      we are only reading values from the map.
 *  @returns            True if the lock is successful, false 
 *                      otherwise.
 */
/*virtual*/ bool TerrainHeightMap1::lock(bool readOnly)
{
	BW_GUARD;
	if (lockCount_ == 0)
	{
	    lockReadOnly_ = readOnly;
	}
    ++lockCount_;
    return true;
}


/**
 *  This function accesses the TerrainMap as though it is an image.
 *  This function should only be called within a lock/unlock pair.
 *
 *  @returns            The TerrainMap as an image.
 */
/*virtual*/ TerrainHeightMap1::ImageType &TerrainHeightMap1::image()
{
    return heights_;
}

/**
 *  This function unlocks the TerrainMap.  It gives derived classes
 *  the chance to repack the underlying data and/or reconstruct
 *  DirectX objects.
 *
 *  @returns            True if the unlocking was successful.
 */
/*virtual*/ bool TerrainHeightMap1::unlock()
{
	BW_GUARD;
	--lockCount_;
	if (lockCount_ == 0)
	{
		if (!lockReadOnly_)
		{
			recalcMinMax();
			terrainBlock1_->setHeightMapDirty();
		}
	}
	return true;
}

/**
 *  This function saves the underlying data to a DataSection that can
 *  be read back in via load.  You should not call this between
 *  lock/unlock pairs.
 *
 *  @param dataSection  The /terrain DataSection in the cdata file.
 *  @returns            true if the save was successful.
*/
bool TerrainHeightMap1::save( DataSectionPtr pSection ) const
{
	BW_GUARD;
	BinaryPtr pTerrainData = pSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	float* heights = (float*)(data + terrainBlock1_->heightsOffset());

	heights_.copyTo( heights );

	return true;
}

#endif // EDITOR_ENABLED

/**
 *  This function accesses the TerrainMap as though it is an image.
 *  This function can be called outside a lock/unlock pair.
 *
 *  @returns            The TerrainMap as an image.
*/
/*virtual*/ TerrainHeightMap1::ImageType const &TerrainHeightMap1::image() const
{
	return heights_;
}

/**
 *  This function loads the TerrainMap from a DataSection that was 
 *  saved via TerrainMap::save.  You should not call this between
 *  lock/unlock pairs.
 *
 *  @param dataSection  The /terrain DataSection in the cdata file.
 *  @param error		If not NULL and an error occurs then this will
 *                      be set to a reason why the error occurred.
 *  @returns            true if the load was successful.
 */
/*virtual*/ bool 
TerrainHeightMap1::load
(
    DataSectionPtr	dataSection, 
    std::string*	error  /*= NULL*/
)
{
	BW_GUARD;
	BinaryPtr pTerrainData = dataSection->asBinary();
	if (!pTerrainData) return false;
	unsigned char* data = (unsigned char*)pTerrainData->data();

	float* heights = (float*)(data + terrainBlock1_->heightsOffset());

	heights_.resize( terrainBlock1_->width(), terrainBlock1_->height() );
	heights_.copyFrom( heights );
	recalcMinMax();

	// Cache the diagonal distance X 4, to improve normalAt performance.
	diagonalDistanceX4_ = sqrtf( spacingX() * spacingZ() * 2 ) * 4;

	return true;
}

/**
 *  This function returns the BaseTxerrainBlock that the TerrainMap is
 *  associated with.
 *
 *  @returns            The BaseTerrainBlock of the TerrainMap.
 */
/*virtual*/ BaseTerrainBlock &TerrainHeightMap1::baseTerrainBlock()
{
    return *terrainBlock1_;
}


/**
 *  This function returns the BaseTerrainBlock that the TerrainMap is
 *  associated with.
 *
 *  @returns            The BaseTerrainBlock of the TerrainMap.
 */
/*virtual*/ BaseTerrainBlock const &TerrainHeightMap1::baseTerrainBlock() const
{
    return *terrainBlock1_;
}


/**
 *  This function returns the spacing between each sample.
 *
 *  @returns            The spacing between each sample in the x-direction.
 */
/*virtual*/ float TerrainHeightMap1::spacingX() const
{
    return BLOCK_SIZE_METRES/blocksWidth();
}


/**
 *  This function returns the spacing between each sample.
 *
 *  @returns            The spacing between each sample in the z-direction.
 */
/*virtual*/ float TerrainHeightMap1::spacingZ() const
{
    return BLOCK_SIZE_METRES/blocksHeight();
}


/**
 *  This function returns the width of the visible part of the 
 *  TerrainMap in poles.
 *
 *  @returns            The width of the visible part of the 
 *                      TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap1::blocksWidth() const
{
    return terrainBlock1_->blocksWidth();
}


/**
 *  This function returns the height of the visible part of the 
 *  TerrainMap in poles.
 *
 *  @returns            The height of the visible part of the 
 *                      TerrainMap.
 */
/*virtual*/ uint32 TerrainHeightMap1::blocksHeight() const
{
	return terrainBlock1_->blocksHeight();
}


/**
 *  This function returns the number of vertices wide of the visible 
 *  part of the TerrainMap.
 *
 *  @returns            The width of the visible part of the 
 *                      TerrainMap in terms of vertices.
 */
/*virtual*/ uint32 TerrainHeightMap1::verticesWidth() const
{
    return terrainBlock1_->verticesWidth();
}


/**
 *  This function returns the number of vertices high of the visible 
 *  part of the TerrainMap.
 *
 *  @returns            The height of the visible part of the 
 *                      TerrainMap in terms of vertices.
 */
/*virtual*/ uint32 TerrainHeightMap1::verticesHeight() const
{
    return terrainBlock1_->verticesHeight();
}


/**
 *  This function returns the width of the TerrainMap, including 
 *  non-visible portions.
 *
 *  @returns            The width of the TerrainMap, including 
 *                      non-visible portions.
 */
/*virtual*/ uint32 TerrainHeightMap1::polesWidth() const
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
/*virtual*/ uint32 TerrainHeightMap1::polesHeight() const
{
    return heights_.height();
}


/**
 *  This function returns the x-offset of the visible portion of the 
 *  HeightMap.
 *
 *  @returns            The x-offset of the first visible column.
 */
/*virtual*/ uint32 TerrainHeightMap1::xVisibleOffset() const
{
    return 1;
}


/**
 *  This function returns the z-offset of the visible portion of the 
 *  HeightMap.
 *
 *  @returns            The z-offset of the first visible row.
 */
/*virtual*/ uint32 TerrainHeightMap1::zVisibleOffset() const
{
    return 1;
}


/**
 *  This function returns the minimum height in the height map.
 *
 *  @returns            The minimum height in the height map.
 */
/*virtual*/ float TerrainHeightMap1::minHeight() const
{
    return minHeight_;
}


/**
 *  This function returns the maximum height in the height map.
 *
 *  @returns            The maximum height in the height map.
 */
/*virtual*/ float TerrainHeightMap1::maxHeight() const
{
    return maxHeight_;
}


/**
 *  This function gets the height of the terrain at (x, z).
 *
 *  @param x            The x coordinate.
 *  @param z            The z coordinate.
 *  @returns            The height at (x, z).
 */
float TerrainHeightMap1::heightAt(int x, int z) const
{
    return heights_.get( x + xVisibleOffset(), z + zVisibleOffset() );
}



/**
 *  This function determines the height at the given xz position, interpolated
 *	to fit on the lod0 mesh.
 *
 *  @param x            The x coordinate to get the height at.
 *  @param z            The z coordinate to get the height at.
 *  @returns            The height at x, z.
 */
/*virtual*/ float TerrainHeightMap1::heightAt(float x, float z) const
{
	BW_GUARD;
	// calculate the x and z positions on the height map.
	float xs = (x / spacingX()) + xVisibleOffset();
	float zs = (z / spacingZ()) + zVisibleOffset();

	// calculate the fractional coverage for the x/z positions
	float xf = xs - floorf(xs);
	float zf = zs - floorf(zs);

	// find the height map start cell of this height
	int32 xOff = int32(floorf(xs));
	int32 zOff = int32(floorf(zs));

	float res = 0;

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

	return res;
}


/**
 *  This function gets the normal at the given integer coordinates.
 *
 *  @param x            The x coordinate.
 *  @param z            The z coordinate.
 *  @returns            The normal at x, z.
 */
Vector3 TerrainHeightMap1::normalAt(int x, int z) const
{   
	BW_GUARD;
	//This function will not work if the spacing is not
	//equal in the X and Z directions.
	MF_ASSERT( spacingX() == spacingZ() )

	const float diagonalMultiplier = 0.7071067811f;// sqrt( 0.5 )

	Vector3 ret;
	ret.y = diagonalDistanceX4_ + ( spacingX() * 2 + spacingZ() * 2 );
	ret.x = heights_.get(x-1,z) - heights_.get(x+1,z);
	ret.z = heights_.get(x,z-1) - heights_.get(x,z+1);	

	float val = heights_.get(x-1,z-1) - heights_.get(x+1,z+1);
	val *= diagonalMultiplier;

	ret.x += val;
	ret.z += val;

	val = heights_.get(x-1,z+1) - heights_.get(x+1,z-1);
	val *= diagonalMultiplier;

	ret.x += val;
	ret.z -= val;
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
/*virtual*/ Vector3 TerrainHeightMap1::normalAt(float x, float z) const
{
	BW_GUARD;
	float xf = x/spacingX() + xVisibleOffset();
	float zf = z/spacingZ() + zVisibleOffset();

	Vector3 ret;
    float xFrac = xf - ::floorf(xf);
    float zFrac = zf - ::floorf(zf);

	xf -= xFrac;
	zf -= zFrac;

	int xfi = (int)xf;
	int zfi = (int)zf;
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
 *  @param source       The source of the line-segment.
 *  @param extent       The end of the line-segment.
 *  @param pCallback    The terrain collision callback.
 */
bool TerrainHeightMap1::collide
( 
    Vector3             const &source, 
    Vector3             const &extent,
    TerrainCollisionCallback *pCallback 
) const
{
	BW_GUARD;
	return hmCollide( source, source, extent, pCallback );
}

/**
 *  This function does prism intersection tests with the terrain.
 *
 *  @param source       The source of the prism.
 *  @param extent       The end point to test.
 *  @param pCallback    The terrain collision callback.
 */
bool TerrainHeightMap1::collide
( 
    WorldTriangle       const &source, 
    Vector3             const &extent,
    TerrainCollisionCallback *pCallback 
) const
{
	BW_GUARD;
	BoundingBox bb;
	bb.addBounds(source.v0());
	bb.addBounds(source.v1());
	bb.addBounds(source.v2());

	Vector3 delta = extent - source.v0();

	bb.addBounds(extent);
	bb.addBounds(source.v1() + delta);
	bb.addBounds(source.v2() + delta);

	return hmCollide(source, extent, bb.minBounds().x, bb.minBounds().z,
		bb.maxBounds().x, bb.maxBounds().z, pCallback);
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
 *  @param originalSource	The source of the unclipped line-segment.
 *  @param source			The source of the line-segment.
 *  @param extent			The end of the line-segment.
 *  @param pCallback		The terrain collision callback.
 *	@return true if collision should stop
 */
bool TerrainHeightMap1::hmCollide
( 
    Vector3             const &originalSource,
    Vector3             const &source, 
    Vector3             const &extent,
    TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	Vector3 s = source;
	Vector3 e = extent;

    const float BOUNDING_BOX_EPSILON = 0.1f;

	BoundingBox bb( Vector3( 0, minHeight(), 0 ), 
		Vector3( BLOCK_SIZE_METRES, maxHeight(), 
        BLOCK_SIZE_METRES ) );

	const Vector3 gridAlign( float(blocksWidth()) / BLOCK_SIZE_METRES, 1.f, 
		float(blocksHeight()) / BLOCK_SIZE_METRES );

	if (bb.clip(s, e, BOUNDING_BOX_EPSILON))
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
				originalSource, extent, pCallback))
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

		// Do boundary check, if we are at the very edge of the block,
		// check against the edge triangles rather than one past the edge
		if (gridX == int32(blocksWidth()) && gridDirX == 0 &&
			almostEqual( BLOCK_SIZE_METRES, source.x, COLLISION_EPSILON ) )
			--gridX;
		
		// Do boundary check, if we are at the very edge of the block,
		// check against the edge triangles rather than one past the edge
		if (gridZ == int32(blocksHeight()) && gridDirZ == 0 &&
			almostEqual( BLOCK_SIZE_METRES, source.z, COLLISION_EPSILON ))
			--gridZ;

		if (checkGrid( gridX, gridZ, 
			originalSource, extent, pCallback))
			return true;
	}
	return false;
}

/**
 *  This function collides a prism with a rect in the heightmap
 *
 *	@param source the start triangle of the prism
 *	@param extent the end location of the prism
 *	@param xStart the x start location of our rect
 *	@param zStart the z start location of our rect
 *	@param xEnd the x end location of our rect
 *	@param zEnd the z end location of our rect
 *	@param pCallback the collision callback
 *
 *	@return true if collision should stop
 */
bool TerrainHeightMap1::hmCollide
(
	WorldTriangle		const &source, 
	Vector3             const &extent,
	float				xStart,
	float				zStart,
	float				xEnd,
	float				zEnd,
	TerrainCollisionCallback *pCallback
) const
{
	BW_GUARD;
	const float xMul = 1.f / spacingX();
	const float zMul = 1.f / spacingZ();

	int xs = int(xStart * xMul);
	int xe = int(ceil(xEnd * xMul));
	int zs = int(zStart * zMul);
	int ze = int(ceil(zEnd * zMul));

	for (int z = zs; z < ze; z++)
	{
		for (int x = xs; x < xe; x++)
		{
			checkGrid( x, z, source, extent, pCallback );
		}
	}

	return false;
}


/**
 *  This function recalculates the minimum and maximum values of the 
 *  height map.
 */
void TerrainHeightMap1::recalcMinMax()
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


namespace
{

	const float EXTEND_BIAS = 0.001f;
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
 *  Check to see if the given line-segment goes through any of the triangles
 *  at gridX, gridZ.
 *
 *  @param gridX        The x coordinate of the triangles.
 *  @param gridZ        The z coordinate of the triangles.
 *  @param source       The start of the line-segment.
 *  @param extent       The end of the line-segment.
 *  @param pCallback    The collision callback.
 */
bool 
TerrainHeightMap1::checkGrid
( 
    int                 gridX, 
    int                 gridZ, 
    Vector3             const &source, 
    Vector3             const &extent, 
    TerrainCollisionCallback* pCallback 
) const
{
	BW_GUARD;
	bool res = false;

	if ( gridX >= 0 && gridX < (signed)verticesWidth() &&
		gridZ >= 0 && gridZ < (signed)verticesHeight() )
	{

		// Construct our four vertices that make up this grid cell
		Vector3 bottomLeft( float(gridX) * spacingX(), 
			heightAt(gridX, gridZ),
			float(gridZ) * spacingZ() );

		Vector3 bottomRight = bottomLeft;
		bottomRight.x += spacingX();
		bottomRight.y = heightAt(gridX + 1, gridZ);

		Vector3 topLeft = bottomLeft;
		topLeft.z += spacingZ();
		topLeft.y =  heightAt(gridX, gridZ + 1);

		Vector3 topRight = topLeft;
		topRight.x += spacingX();
		topRight.y =  heightAt(gridX + 1, gridZ + 1);

		// Construct our triangles
		WorldTriangle triA = WorldTriangle( bottomLeft, topLeft, topRight, TRIANGLE_TERRAIN );
		WorldTriangle triB = WorldTriangle( topRight, bottomRight, bottomLeft, TRIANGLE_TERRAIN );

		extend( triA );
		extend( triB );

		Vector3 dir = extent - source;
		float dist = dir.length();
		float dist2 = dist;
		dir *= 1.f / dist;

		// Check if we intersect with either triangle
		bool intersectsA = triA.intersects( source, dir, dist );
		bool intersectsB = triB.intersects( source, dir, dist2 );

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
*  @param gridX        The x coordinate of the triangles.
*  @param gridZ        The z coordinate of the triangles.
*  @param source       The start of prism.
*  @param extent       The end of the prism.
*  @param pCallback    The collision callback.
*/
bool 
TerrainHeightMap1::checkGrid
( 
 int				gridX, 
 int				gridZ, 
 WorldTriangle		const &source, 
 Vector3			const &extent, 
 TerrainCollisionCallback* pCallback 
 ) const
{
	BW_GUARD;
	bool res = false;

	if ( gridX >= 0 && gridX < (signed)verticesWidth() &&
		gridZ >= 0 && gridZ < (signed)verticesHeight() )
	{
		// Construct our four vertices that make up this grid cell
		Vector3 bottomLeft( float(gridX) * spacingX(), 
			heightAt(gridX, gridZ),
			float(gridZ) * spacingZ() );

		Vector3 bottomRight = bottomLeft;
		bottomRight.x += spacingX();
		bottomRight.y = heightAt(gridX + 1, gridZ);

		Vector3 topLeft = bottomLeft;
		topLeft.z += spacingZ();
		topLeft.y =  heightAt(gridX, gridZ + 1);

		Vector3 topRight = topLeft;
		topRight.x += spacingX();
		topRight.y =  heightAt(gridX + 1, gridZ + 1);

		// Construct our triangles
		WorldTriangle triA = WorldTriangle( bottomLeft, topLeft, 
					topRight, TRIANGLE_TERRAIN );
		WorldTriangle triB = WorldTriangle( topRight, bottomRight, 
					bottomLeft, TRIANGLE_TERRAIN );

		// Get the delta for the intersection
		Vector3 delta = extent - source.v0();

		// Check if we intersect with either triangle
		bool intersectsA = triA.intersects( source, delta );
		bool intersectsB = triB.intersects( source, delta );

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
