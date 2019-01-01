/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_HEIGHT_MAP2_HPP
#define TERRAIN_TERRAIN_HEIGHT_MAP2_HPP

#include "../terrain_height_map.hpp"
#include "terrain_quad_tree_cell.hpp"
#include "../terrain_data.hpp"
#include "resource.hpp"

namespace Terrain
{
    class TerrainCollisionCallback;
}

namespace Terrain
{
    /**
     *  This class provides access to terrain height map data for the 
     *  second generation of terrain.
     */
    class TerrainHeightMap2 : public TerrainHeightMap
    {
    public:
		
		// These define the non-visible border around each side of the
		// height map.
		static const uint32 DEFAULT_VISIBLE_OFFSET = 2;

		class UnlockCallback
		{
		public:
			virtual ~UnlockCallback(){};
			virtual bool notify() = 0;
		};

   		explicit TerrainHeightMap2( uint32 size = 0,
									uint32 lodLevel = 0,
									UnlockCallback* unlockNotifier = NULL );

        ~TerrainHeightMap2();      

        uint32 width() const;
        uint32 height() const;

#ifdef EDITOR_ENABLED
		// Editor specific functionality
		bool create( uint32 size, std::string *error = NULL );

		bool lock(bool readOnly);
        ImageType &image();        
		bool unlock();
		bool save( DataSectionPtr pSection ) const;
#endif	// EDITOR_ENABLED
		
		ImageType const &image() const;

		bool load(DataSectionPtr dataSection, std::string *error = NULL);
		void unlockCallback( UnlockCallback* ulc ) { unlockCallback_ = ulc; };
        
		float spacingX() const;
        float spacingZ() const;

        uint32 blocksWidth() const;
        uint32 blocksHeight() const;

        uint32 verticesWidth() const;
        uint32 verticesHeight() const;

        uint32 polesWidth() const;
        uint32 polesHeight() const;

		virtual uint32 xVisibleOffset() const;
		virtual uint32 zVisibleOffset() const;

        uint32 internalVisibleOffset() const;
       
        float minHeight() const;
		float maxHeight() const;

		float heightAt(int x, int z) const;
        float heightAt(float x, float z) const;

		Vector3 normalAt(int x, int z) const;
        Vector3 normalAt(float x, float z) const;
        
	    bool collide
        ( 
            Vector3             const &start, 
            Vector3             const &end,
		    TerrainCollisionCallback *pCallback 
        ) const;

	    bool collide
        ( 
            WorldTriangle       const &start, 
            Vector3             const &end,
		    TerrainCollisionCallback *pCallback 
        ) const;

	    bool hmCollide
        (
            Vector3             const &originalSource,
            Vector3             const &start, 
            Vector3             const &end,
		    TerrainCollisionCallback *pCallback
        ) const;

#ifdef EDITOR_ENABLED
		bool hmCollideAlongZAxis
        (
            Vector3             const &originalSource,
            Vector3             const &start, 
            Vector3             const &end,
		    TerrainCollisionCallback *pCallback
        ) const;
#endif//EDITOR_ENABLED

	    bool hmCollide
        (
            WorldTriangle		const &triStart, 
            Vector3             const &triEnd,
			float				xStart,
			float				zStart,
			float				xEnd,
			float				zEnd,
		    TerrainCollisionCallback *pCallback
        ) const;

		uint32					lodLevel() const;
		
		static std::string getHeightSectionName(const std::string&	base, 
												uint32				lod );

#ifdef EDITOR_ENABLED
		static void optimiseCollisionAlongZAxis( bool enable );
#endif//EDITOR_ENABLED

    private:
		bool loadHeightMap( const void*			data, 
							size_t				length,
							std::string *error = NULL );
		void recalcQuadTree() const;
		void invalidateQuadTree() const;
 	
		bool 
		checkGrid
		( 
			int  				gridX, 
			int					gridZ, 
			Vector3				const &start, 
			Vector3				const &end, 
			TerrainCollisionCallback* pCallback 
		) const;

		bool 
		checkGrid
		( 
			int		  			gridX, 
			int					gridZ, 
			WorldTriangle		const &start, 
			Vector3				const &end, 
			TerrainCollisionCallback* pCallback 
		) const;
			
		/**
		 * Internal accessor functions. Use these within class instead of the 
		 * public versions to avoid virtual function calls.
		 */
		void			refreshInternalDimensions();
		inline uint32	internalBlocksWidth()			const;
		inline uint32	internalBlocksHeight()			const;
		inline float	internalSpacingX()				const;
		inline float	internalSpacingZ()				const;
		inline float	internalHeightAt(int x, int z)	const;

		/**
		 *	This method returns the threshold for using the quadtree on this block
		 */
		inline float	quadtreeThreshold()				const;


#ifdef EDITOR_ENABLED
		// editor specific functionality
		void recalcMinMax();

		size_t				lockCount_;
		bool				lockReadOnly_;
#endif

		Moo::Image<float>	heights_;
		float				minHeight_;
		float				maxHeight_;

		// This member is used to cache an expensive calculation in order to
		// optimise "normalAt" (avoiding expensive sqrtf call).
		float				diagonalDistanceX4_;

		// This can notify user when "unlock" is called.
		UnlockCallback*				unlockCallback_;

		// These are mutable as the quad tree can be generated on demand
		// when the collide methods are being called
        mutable TerrainQuadTreeCell quadTree_;
		mutable bool				quadTreeInited_;
		
		uint32						visibleOffset_;
		uint32						lodLevel_;

		uint32						internalBlocksWidth_;
		uint32						internalBlocksHeight_;
		float						internalSpacingX_;
		float						internalSpacingZ_;
	};

	inline uint32 TerrainHeightMap2::internalVisibleOffset() const
	{
		return visibleOffset_;
	}

	inline void TerrainHeightMap2::refreshInternalDimensions()
	{
		internalBlocksWidth_ = heights_.width() - ( internalVisibleOffset() * 2 ) - 1;
		internalBlocksHeight_ = heights_.height() - ( internalVisibleOffset() * 2 ) - 1;
		internalSpacingX_ = BLOCK_SIZE_METRES / internalBlocksWidth_;
		internalSpacingZ_ = BLOCK_SIZE_METRES / internalBlocksHeight_;

		// Cache the diagonal distance X 4, to improve normalAt performance.
		diagonalDistanceX4_ =
			sqrtf( internalSpacingX_ * internalSpacingZ_ * 2 ) * 4;
	}

	inline uint32 TerrainHeightMap2::internalBlocksWidth() const
	{
		return internalBlocksWidth_;
	}

	inline uint32 TerrainHeightMap2::internalBlocksHeight() const
	{
		return internalBlocksHeight_;
	}

	inline float TerrainHeightMap2::internalSpacingX() const
	{
		return internalSpacingX_;
	}

	inline float TerrainHeightMap2::internalSpacingZ() const
	{
		return internalSpacingZ_;
	}

	inline float TerrainHeightMap2::internalHeightAt(int x, int z) const
	{
		return heights_.get( x + internalVisibleOffset(), z + internalVisibleOffset() );
	}

	inline uint32 TerrainHeightMap2::lodLevel() const
	{
		return lodLevel_;
	}

	/**
	 *	Get the quadtree treshold, this value is used as the minimum size
	 *	for the quadtree operations.
	 *	@return the quadtree threshold
	 */
	inline float TerrainHeightMap2::quadtreeThreshold() const
	{
		return this->internalSpacingX() * 4.f;
	}

	typedef TerrainMapIter<TerrainHeightMap2>   TerrainHeightMap2Iter;
	typedef TerrainMapHolder<TerrainHeightMap2> TerrainHeightMap2Holder;

#ifndef MF_SERVER
	class HeightMapResource : public Resource<TerrainHeightMap2>
	{
	public:
		HeightMapResource( const std::string& heightsSectionName, uint32 lodLevel );
	
		inline  ResourceRequired	evaluate(	uint32	requiredVertexGridSize,
												uint32	standardHeightMapSize,
												float	detailHeightMapDistance,
												float	blockDistance,
												uint32 topLodLevel);
	protected:
		virtual bool load();

		std::string terrainSectionName_;

		uint32 lodLevel_;
	};

	inline ResourceRequired HeightMapResource::evaluate(	
		uint32	requiredVertexGridSize,
		uint32	standardHeightMapSize,
		float	detailHeightMapDistance,
		float	blockDistance,
		uint32 topLodLevel)
	{
		required_ = RR_No;

		// Is the distance within the area we need full height maps, OR
		// Is the standard height map too small to create required vertex lod?
		if ( blockDistance < detailHeightMapDistance || 
			( standardHeightMapSize <  requiredVertexGridSize ) )
		{
			required_ = RR_Yes;
			if (object_.exists() && topLodLevel != lodLevel_)
			{
				object_ = NULL;
				lodLevel_ = topLodLevel;
			}
		}

		return required_ ;
	}

#endif
}

#endif // TERRAIN_TERRAIN_HEIGHT_MAP2_HPP
