/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_MAP_HPP
#define TERRAIN_TERRAIN_MAP_HPP

#include "terrain_map_iter.hpp"
#include "moo/image.hpp"
#include "resmgr/datasection.hpp"

namespace Terrain
{
    class BaseTerrainBlock;
}

namespace Terrain
{
    /**
     *  This is the basis for all image/maps in terrain, for example height
     *  maps, texture layer maps, shadow maps and hole maps.
     */
    template<typename TYPE>
    class TerrainMap : public SafeReferenceCount
    {
    public:
		typedef Moo::Image<TYPE>                    ImageType;
        typedef TYPE                                PixelType;
        typedef TerrainMapIter< TerrainMap<TYPE> >  Iterator;

        /**
         *  This function is the TerrainMap destructor.
         */
        virtual ~TerrainMap();

        /**
         *  This function returns the width of the TerrainMap.
         *
         *  @returns            The width of the TerrainMap.
         */
        virtual uint32 width() const = 0;

        /**
         *  This function returns the height of the TerrainMap.
         *
         *  @returns            The height of the TerrainMap.
         */
        virtual uint32 height() const = 0;

		// Only required by editor
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
        virtual bool lock( bool readOnly ) = 0;

        /**
         *  This function accesses the TerrainMap as though it is an image.
         *  This function should only be called within a lock/unlock pair.
         *
         *  @returns            The TerrainMap as an image.
         */
        virtual ImageType &image() = 0;

		/**
		*  This function unlocks the TerrainMap.  It gives derived classes
		*  the chance to repack the underlying data and/or reconstruct
		*  DirectX objects.
		*
		*  @returns            True if the unlocking was successful.
		*/
		virtual bool unlock() = 0;

		/**
		*  This function saves the underlying data to a DataSection that can
		*  be read back in via load.  You should not call this between
		*  lock/unlock pairs.
		*
		*  @param				The DataSection to save to.
		*/
		virtual bool save( DataSectionPtr ) const = 0;
#endif // EDITOR_ENABLED

        /**
         *  This function accesses the TerrainMap as though it is an image.
         *  This function should only be called within a lock/unlock pair.
         *
         *  @returns            The TerrainMap as an image.
         */
        virtual ImageType const &image() const = 0;

        /**
         *  This function returns an iterator over the map.
         *  This function should only be called within a lock/unlock pair.
         *
         *  @param x            The x coordinate.
         *  @param y            The y coordinate.
         *  @returns            An iterator that can be used to access the
         *                      underlying image.
         */
        virtual Iterator iterator(int x, int y);

         /**
         *  This function loads the TerrainMap from a DataSection that was 
         *  saved via TerrainMap::save.  You should not call this between
         *  lock/unlock pairs.
         *
         *  @param dataSection  The DataSection to load from.
         *  @param error        If not NULL and an error occurs then this will
         *                      be set to a reason why the error occured.
         *  @returns            True if the load was successful.
         */
        virtual bool load(DataSectionPtr dataSection, std::string *error = NULL) = 0;
    };
};

/**
 *  Implementation.
 */

template<typename TYPE>
inline
 /*virtual*/ Terrain::TerrainMap<TYPE>::~TerrainMap()
{
}


template<typename TYPE>
inline
typename Terrain::TerrainMap<TYPE>::Iterator
Terrain::TerrainMap<TYPE>::iterator(int x, int y)
{
    return Terrain::TerrainMapIter< TerrainMap<TYPE> >(*this, x, y);
}

#endif // TERRAIN_TERRAIN_MAP_HPP
