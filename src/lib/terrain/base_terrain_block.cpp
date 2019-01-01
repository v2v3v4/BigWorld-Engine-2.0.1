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
#include "cstdmf/debug.hpp"
#include "base_terrain_block.hpp"
#include "terrain1/terrain_block1.hpp"
#include "terrain1/editor_terrain_block1.hpp"
#include "terrain2/terrain_block2.hpp"
#include "terrain2/editor_terrain_block2.hpp"
#include "terrain_height_map.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2("Terrain", 0)

using namespace Terrain;


/*static*/ float          BaseTerrainBlock::NO_TERRAIN     = -1000000.f;
/*static*/ TerrainFinder *BaseTerrainBlock::terrainFinder_ = NULL;


/*static*/
uint32 BaseTerrainBlock::terrainVersion( std::string& resource )
{
	BW_GUARD;
	if ( resource.substr( resource.length() - 8 ) == "/terrain" &&
		BWResource::openSection( resource + '2' ) != NULL )
	{
		// it's got a new terrain section, so treat it first as a new terrain,
		// even if a plan '/terrain' was requested. Also, have to add the '2'
		// at the end of the resource name.
		resource += '2';
		return 200;
	}
	else if ( resource.substr( resource.length() - 8 ) == "/terrain" &&
		BWResource::openSection( resource ) != NULL )
		return 100;
	else if ( resource.substr( resource.length() - 9 ) == "/terrain2" &&
		BWResource::openSection( resource ) != NULL )
		return 200;

	return 0;
}


BaseTerrainBlockPtr BaseTerrainBlock::loadBlock( const std::string& filename, 
	const Matrix& worldTransform, const Vector3& cameraPosition, 
	TerrainSettingsPtr pSettings, std::string* error /* = NULL*/ )
{
	BW_GUARD;	
#ifdef EDITOR_ENABLED
	EditorBaseTerrainBlockPtr result;
#else
	BaseTerrainBlockPtr result;
#endif // EDITOR_ENABLED
	std::string verResource = filename;
	uint32 ver = BaseTerrainBlock::terrainVersion( verResource );
	if ( ver == 100 )
	{
		result = new TERRAINBLOCK1();
	}
	else if ( ver == 200 )
	{
		result = new TERRAINBLOCK2(pSettings);
	}
	else
	{
		std::stringstream msg;
			msg << "BaseTerrainBlock::loadBlock: "; 
			msg << "Unknown terrain version for block " << filename;
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return NULL;
	}

#if defined(MF_SERVER) || defined(EDITOR_ENABLED)
	// TODO:
	result->resourceName( verResource );
#endif

	bool ok = result->load(verResource, worldTransform, cameraPosition, error);
    if (!ok)
    {
		ERROR_MSG( "BaseTerrainBlock::loadBlock: "
			"Failed to load from %s\n", verResource.c_str() );
        result = NULL;
    }
    return result;
}


BaseTerrainBlock::BaseTerrainBlock()
{
}


/*virtual*/ BaseTerrainBlock::~BaseTerrainBlock()
{
}


/*static*/ TerrainFinder::Details
BaseTerrainBlock::findOutsideBlock(Vector3 const &pos)
{
    BW_GUARD;
	if (terrainFinder_ == NULL)
        return TerrainFinder::Details();
    else
        return terrainFinder_->findOutsideBlock(pos);
}


/*static*/ float BaseTerrainBlock::getHeight(float x, float z)
{
	BW_GUARD;
	Vector3 pos(x, 0.0f, z);
	TerrainFinder::Details findDetails =
        BaseTerrainBlock::findOutsideBlock(pos);

    BaseTerrainBlock *block = findDetails.pBlock_;
	if (block != NULL)
	{
		Vector3 terPos = findDetails.pInvMatrix_->applyPoint(pos);
		float height = block->heightAt(terPos[0], terPos[2]);
		if (height != NO_TERRAIN)
		{
			return height - findDetails.pInvMatrix_->applyToOrigin()[1];
		}
	}

	return NO_TERRAIN;
}


/*static*/ void BaseTerrainBlock::setTerrainFinder(TerrainFinder & terrainFinder)
{
    terrainFinder_ = &terrainFinder;
}


#ifdef MF_SERVER

const std::string & BaseTerrainBlock::resourceName() const
{
	return resourceName_;
}


void BaseTerrainBlock::resourceName( const std::string & name )
{
	resourceName_ = name;
}

#endif // MF_SERVER
