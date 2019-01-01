/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TTL2CACHE_HPP
#define TERRAIN_TTL2CACHE_HPP

#ifdef EDITOR_ENABLED

namespace Terrain
{
	class TerrainTextureLayer2;
}

namespace Terrain
{
	/** 
	 *	This class keeps the last CACHE_SIZE TerrainTextureLayer2 in 
	 *	decompressed form.
	 */
	class TTL2Cache
	{
	public:
		static const uint32 CACHE_SIZE = 1024;

		static TTL2Cache *instance();

		TTL2Cache();

		void clear();

		void onLock(TerrainTextureLayer2 *layer, bool readOnly);

		void onUnlock(TerrainTextureLayer2 *layer);

		void delTextureLayer(TerrainTextureLayer2 *layer);

	private:
		TTL2Cache(TTL2Cache const &); 			 // not allowed
		TTL2Cache &operator=(TTL2Cache const &); // not allowed

	private:
		typedef std::list<TerrainTextureLayer2*> LayerList;

		LayerList			layers_;
	};
}

#endif // EDITOR_ENABLED

#endif // TERRAIN_TTL2CACHE_HPP
