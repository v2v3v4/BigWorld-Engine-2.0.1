/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_EDITOR_TERRAIN_LOD_MANAGER_HPP
#define TERRAIN_EDITOR_TERRAIN_LOD_MANAGER_HPP

#ifdef EDITOR_ENABLED

#include "vertex_lod_manager.hpp"
#include "aliased_height_map.hpp"
class BackgroundTask;

namespace Terrain
{
	/**
	 * This class manages LODs in an editor context, if source height map 
	 * changes, required LODs are also regenerated.
	 */
	class EditorVertexLodManager : public VertexLodManager
	{
		friend class TerrainBlock2;

	public:

		/**
		 * Construct an empty EditorTerrainLodManager with space for a given 
		 * number of LODs.
		 *
		 * @param numLods How many LODs to create.
		 */
		EditorVertexLodManager( TerrainBlock2& owner, uint32 numLods );

		/**
		 * Return given lod, if heightMap is dirty this will regenerate all 
		 * required. This always streams synchronously.
		 */
		virtual void stream();

		/**
		 * Flag the height map as being dirty so lods will be regenerated.
		 */
		void setDirty() { isDirty_ = true; }

		/**
		* Save all lods to this terrain section using this height map. 
		*/
		static bool save(	const std::string &		terrainSectionPath, 
							TerrainHeightMap2Ptr	pSourceHeightMap );
		static bool save(	DataSectionPtr			pTerrainSection,
							TerrainHeightMap2Ptr	pSourceHeightMap );
	protected:
		virtual bool load();

	private:
		
		// Data flag
		bool					isDirty_;

		// no copying
		EditorVertexLodManager( const EditorVertexLodManager& );
		EditorVertexLodManager& operator=( const EditorVertexLodManager& );
	};
}

#endif //EDITOR_ENABLED

#endif // TERRAIN_LOD_MANAGER_HPP
