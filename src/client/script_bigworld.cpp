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
#include "script_bigworld.hpp"

#include "app.hpp"
#include "app_config.hpp"
#include "entity_manager.hpp"
#include "entity_type.hpp"
#include "physics.hpp"
#include "pathed_filename.hpp"

#include "common/py_physics2.hpp"

#include "connection/server_connection.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_overlapper.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"
#include "chunk/user_data_object.hpp"

#include "cstdmf/debug.hpp"

#include "entitydef/constants.hpp"

#include "input/input.hpp"
#include "input/py_input.hpp"

#include "moo/animation_manager.hpp"
#include "moo/graphics_settings.hpp"
#include "moo/texture_manager.hpp"
#include "moo/visual_splitter.hpp"

#include "network/remote_stepper.hpp"

#include "particle/particle_system_manager.hpp"

#include "physics2/worldtri.hpp"

#include "pyscript/personality.hpp"
#include "pyscript/py_data_section.hpp"
#include "pyscript/py_import_paths.hpp"
#include "pyscript/py_output_writer.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"

#include "resmgr/xml_section.hpp"

#include "romp/weather.hpp"
#include "romp/flora.hpp"
#include "romp/console_manager.hpp"
#include "romp/xconsole.hpp"

#include "terrain/base_terrain_block.hpp"
#include "terrain/terrain2/terrain_lod_controller.hpp"

#include "waypoint/chunk_waypoint_set.hpp"
#include "waypoint/navigator.hpp"
#include "waypoint/navigator_cache.hpp"

#include <queue>

DECLARE_DEBUG_COMPONENT2( "Script", 0 )


typedef SmartPointer<PyObject> PyObjectPtr;

typedef std::pair<KeyCode::Key, PyObject*> KeyPyObjectPair;
typedef std::list<KeyPyObjectPair> KeyPyObjectPairList;

namespace {
	KeyPyObjectPairList g_keyEventSinks;
}


// -----------------------------------------------------------------------------
// Section: BigWorld module: Chunk access functions
// -----------------------------------------------------------------------------
typedef SmartPointer<PyObject> PyObjectPtr;

/**
 *	Helper class to store the triangle collided with
 */
class FDPTriangle : public CollisionCallback
{
public:
	WorldTriangle	tri_;

private:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		tri_ = WorldTriangle(
			obstacle.transform_.applyPoint( triangle.v0() ),
			obstacle.transform_.applyPoint( triangle.v1() ),
			obstacle.transform_.applyPoint( triangle.v2() ),
			triangle.flags() );

		return COLLIDE_BEFORE;	// see if there's anything closer...
	}
};

/*~ function BigWorld.findDropPoint
 *
 *	@param spaceID The ID of the space you want to do the raycast in
 *	@param vector3 Start point for the collision test
 *	@return A pair of the drop point, and the triangle it hit,
 *		or None if nothing was hit.
 *
 *	Finds the point directly beneath the start point that collides with
 *	the collision scene and terrain (if present in that location)
 */
/**
 *	Finds the drop point under the input point using the collision
 *	scene and terrain (if present in that location)
 *
 *	@return A pair of the drop point, and the triangle it hit,
 *		or None if nothing was hit.
 */
static PyObject * findDropPoint( SpaceID spaceID, const Vector3 & inPt )
{
	BW_GUARD;
	Vector3		outPt;
//	Vector3		triangle[3];

	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID, false );
	if (!pSpace)
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.findDropPoint(): "
			"No space ID %d", spaceID );
		return NULL;
	}

	FDPTriangle fdpt;
	Vector3 ndPt( inPt.x, inPt.y-100.f, inPt.z );
	float dist = pSpace->collide( inPt, ndPt, fdpt );
	if (dist < 0.f)
	{
		Py_Return;
	}
	outPt.set( inPt.x, inPt.y-dist, inPt.z );
//	triangle[0] = fdpt.tri_.v0();
//	triangle[1] = fdpt.tri_.v1();
//	triangle[2] = fdpt.tri_.v2();

	PyObjectPtr results[4] = {
		PyObjectPtr( Script::getData( outPt ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v0() ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v1() ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v2() ), true ) };
	return Py_BuildValue( "(O,(O,O,O))",
		&*results[0], &*results[1], &*results[2], &*results[3] );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, findDropPoint,
	ARG( SpaceID, ARG( Vector3, END ) ), BigWorld )


static PyObject * collide( SpaceID spaceID,
		const Vector3 & src, const Vector3 & dst )
{
	BW_GUARD;
	return py_collide<FDPTriangle>(spaceID, src, dst);
}
PY_AUTO_MODULE_FUNCTION( RETOWN, collide, ARG( SpaceID,
	ARG( Vector3, ARG( Vector3, END ) ) ), BigWorld )


/*~ function BigWorld.resetEntityManager
 *
 *	Resets the entity manager's state. After disconnecting, the
 *	entity will retain all of it's state. Allowing the world to
 *	still be explored offline, even with entity no longer being
 *	updated by the server. Calling this function resets the entity
 *	manager, removing all entities from the space.
 *
 *	@param	keepPlayerAround	(type bool) True if player should be
 *								spared. False if player should also
 *								be removed together with all entities.
 *
 *  @param	keepClientOnly		(type bool) True to clear all server
 *								entities and keep client only entities.
 */
static void resetEntityManager(bool keepPlayerAround, bool keepClientOnly )
{
	BW_GUARD;

	EntityManager::instance().clearAllEntities(keepPlayerAround, keepClientOnly);
}
PY_AUTO_MODULE_FUNCTION( RETVOID, resetEntityManager, OPTARG( bool, false, OPTARG( bool, false, END ) ), BigWorld )


/*~ function BigWorld.cameraSpaceID
 *
 *	@param spaceID (optional) Sets the space that the camera exists in.
 *	@return spaceID The spaceID that the camera currently exists in.
 *		If the camera is not in any space then 0 is returned.
 */
/**
 *	This function returns the id of the space that the camera is currently in.
 *	If the camera is not in any space then 0 is returned.
 *	You can optionally set the spaceID by passing it as an argument.
 */
static PyObject * cameraSpaceID( SpaceID newSpaceID = 0 )
{
	BW_GUARD;
	if (newSpaceID != 0)
	{
		ChunkSpacePtr pSpace =
			ChunkManager::instance().space( newSpaceID, false );
		if (!pSpace)
		{
			PyErr_Format( PyExc_ValueError, "BigWorld.cameraSpaceID: "
				"No such space ID %d\n", newSpaceID );
			return NULL;
		}

		ChunkManager::instance().camera( Moo::rc().invView(), pSpace );
	}
	else 
	{ 
		ChunkManager::instance().camera( Moo::rc().invView(), NULL ); 
	} 


	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	return Script::getData( pSpace ? pSpace->id() : 0 );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, cameraSpaceID,
	OPTARG( SpaceID, 0, END ), BigWorld )


/// This is a vector of spaces created by the client
static std::vector<ChunkSpacePtr> s_clientSpaces;

/*~ function BigWorld.createSpace
 *
 * @return spaceID The ID of the client-only space that has been created
 *
 *	The space is held in a collection of client spaces for safe keeping,
 *	so that it is not prematurely disposed.
 */
/**
 *	This function creates and returns the id of a new client-only space.
 *
 *	The space is held in a collection of client spaces for safe keeping,
 *	so that it is not prematurely disposed.
 */
static SpaceID createSpace()
{
	BW_GUARD;
	static SpaceID clientSpaceID = (1 << 30);	// say

	ChunkManager & cm = ChunkManager::instance();

	SpaceID newSpaceID = 0;
	do
	{
		newSpaceID = clientSpaceID++;
		if (clientSpaceID == 0) clientSpaceID = (1 << 30);
	} while (cm.space( newSpaceID, false ));

	ChunkSpacePtr pNewSpace = cm.space( newSpaceID );
	s_clientSpaces.push_back( pNewSpace );
	return pNewSpace->id();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, createSpace, END, BigWorld )


/**
 *	This helper function gets the given client space ID.
 */
static ChunkSpacePtr getClientSpace( SpaceID spaceID, const char * methodName )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID, false );
	if (!pSpace)
	{
		PyErr_Format( PyExc_ValueError, "%s: No space ID %d",
			methodName, spaceID );
		return NULL;
	}

	if (std::find( s_clientSpaces.begin(), s_clientSpaces.end(), pSpace ) ==
		s_clientSpaces.end())
	{
		PyErr_Format( PyExc_ValueError, "%s: Space ID %d is not a client space "
			"(or is no longer held)", methodName, spaceID );
		return NULL;
	}

	return pSpace;
}


/*~ function BigWorld.releaseSpace
 *
 *	This function releases the given client space ID.
 *
 *	Note that the space will not necessarily be immediately deleted - that must
 *	wait until it is no longer referenced (including by chunk dir mappings).
 *
 *	Raises a ValueError if the space for the given spaceID is not found.
 *
 *	@param spaceID The ID of the space to release.
 *
 */
/**
 *	This function releases the given client space ID.
 *
 *	Note that the space will probably not be immediately deleted - that must
 *	wait until it is no longer referenced (including by chunk dir mappings)
 *
 *	It sets a python error and returns false if the space ID was not a client
 *	space.
 */
static bool releaseSpace( SpaceID spaceID )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = getClientSpace( spaceID, "BigWorld.releaseSpace()" );
	// getClientSpace sets the Python error state if it fails
	if (!pSpace)
	{
		return false;
	}


	// find its iterator again (getClientSpace just found it...)
	std::vector<ChunkSpacePtr>::iterator found =
		std::find( s_clientSpaces.begin(), s_clientSpaces.end(), pSpace );

	MF_ASSERT_DEV( found != s_clientSpaces.end() );

	// and erase it
	if( found != s_clientSpaces.end() )
		s_clientSpaces.erase( found );

	EntityManager::instance().spaceGone(spaceID);
	pSpace->clear();

	return true;
}
PY_AUTO_MODULE_FUNCTION( RETOK, releaseSpace, ARG( SpaceID, END ), BigWorld )


/*~ function BigWorld.addSpaceGeometryMapping
 *
 *	This function maps geometry into the given client space ID.
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 *
 *	The given transform must be aligned to the chunk grid. That is, it should
 *	be a translation matrix whose position is in multiples of 100 on the
 *	X and Z axis. Any other transform will result in undefined behaviour. 
 *
 *	Any extra space mapped in must use the same terrain system as the first,
 *	with the same settings, the behaviour of anything else is undefined.
 *
 *	Raises a ValueError if the space for the given spaceID is not found.
 *
 *	@param spaceID 		The ID of the space
 *	@param matrix 		The transform to apply to the geometry. None may be
 *						passed in if no transform is required (the identity
 *						matrix).
 *	@param filepath 	The path to the directory containing the space data
 *	@return 			(integer) handle that is used when removing mappings
 *						using BigWorld.delSpaceGeometryMapping().
 *
 */
/**
 *	This function maps geometry into the given client space ID.
 *
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 *
 *	It returns an integer handle which can later be used to unmap it.
 */
static PyObject * addSpaceGeometryMapping( SpaceID spaceID,
	MatrixProviderPtr pMapper, const std::string & path )
{
	BW_GUARD;

	BWResource::WatchAccessFromCallingThreadHolder watchAccess( false );

	ChunkSpacePtr pSpace = getClientSpace( spaceID,
		"BigWorld.addSpaceGeometryMapping()" );
	// getClientSpace sets the Python error state if it fails
	if (!pSpace)
	{
		return false;
	}


	std::string emptyString;

	static uint32 nextLowEntryID = 0;
	uint32 lowEntryID;

	// build the entry id
	SpaceEntryID seid;
	seid.ip = 0;
	do
	{
		lowEntryID = nextLowEntryID++;
		seid.port = uint16(lowEntryID >> 16);
		seid.salt = uint16(lowEntryID);
	}
	// make sure we haven't already used this id,
	// incredibly small chance 'tho it be
	while (pSpace->dataEntry( seid, 1, emptyString ) == uint16(-1));

	// get the matrix
	Matrix m = Matrix::identity;
	if (pMapper) pMapper->matrix( m );

	// see if we can add the mapping
	// NOTE: Currently not using the asynchronous version as we wouldn't be able
	// to tell if there is a script error. We may want to reconsider or make
	// this an argument.
	if (!pSpace->addMapping( seid, m, path ))
	{
		// no good, so remove the data entry
		pSpace->dataEntry( seid, uint16(-1), emptyString );

		PyErr_Format( PyExc_ValueError, "BigWorld.addSpaceGeometryMapping(): "
			"Could not map %s into space ID %d (probably no space.settings)",
			path.c_str(), spaceID );
		return NULL;
	}

	Script::call(
		PyObject_GetAttrString(
			Personality::instance(),
			"onGeometryMapped" ),
		Py_BuildValue( "(is)", spaceID, path.c_str() ),
		"EntityManager::spaceData geometry notifier: ",
		true );

	// everything's hunky dory, so return the (low dword of) the entry ID
	return Script::getData( lowEntryID );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, addSpaceGeometryMapping, ARG( SpaceID,
	ARG( MatrixProviderPtr, ARG( std::string, END ) ) ), BigWorld )


/*~ function BigWorld.delSpaceGeometryMapping
 *
 *	This function unmaps geometry from the given client space ID.
 *
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 *
 *	Raises a ValueError if the space for the given spaceID is not found, or if
 *	the handle does not refer to a mapped space geometry.
 *
 *	@param spaceID 	The ID of the space
 *	@param handle 	An integer handle to the space that was returned when
 *					created
 */
/**
 *	This function unmaps geometry from the given client space ID.
 *
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 */
static bool delSpaceGeometryMapping( SpaceID spaceID, uint32 lowEntryID )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = getClientSpace( spaceID,
		"BigWorld.delSpaceGeometryMapping()" );
	// getClientSpace sets the Python error state if it fails
	if (!pSpace)
	{
		return false;
	}

	std::string emptyString;

	SpaceEntryID seid;
	seid.ip = 0;
	seid.port = uint16(lowEntryID >> 16);
	seid.salt = uint16(lowEntryID);

	uint16 okey = pSpace->dataEntry( seid, uint16(-1), emptyString );
	if (okey == uint16(-1))
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.delSpaceGeometryMapping(): "
			"Could not unmap entry id %d from space ID %d (no such entry)",
			int(lowEntryID), spaceID );
		return false;
	}

	MF_ASSERT_DEV( okey == 1 );	// or else we removed the wrong kind of key!
	pSpace->delMapping( seid );
	return true;
}
PY_AUTO_MODULE_FUNCTION( RETOK, delSpaceGeometryMapping,
	ARG( SpaceID, ARG( uint32, END ) ), BigWorld )


/*~ function BigWorld.clearSpace
 *
 *	This function clears out the given client space ID, including unmapping
 *	all geometry in it. Naturally there should be no entities inside the
 *	space when this is called or else they will be stranded without chunks.
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 *
 *	Raises a ValueError if the space for the given spaceID is not found.
 *
 *	@param spaceID The ID of the space
 */
/**
 *	This function clears out the given client space ID, including unmapping
 *	all geometry in it. Naturally there should be no entities inside the
 *	space when this is called or else they will be stranded without chunks.
 *
 *	It cannot be used with spaces created by the server since the server
 *	controls the geometry mappings in those spaces.
 */
static bool clearSpace( SpaceID spaceID )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = getClientSpace( spaceID, "BigWorld.clearSpace()" );
	// getClientSpace sets the Python error state if it fails
	if (!pSpace)
	{
		return false;
	}

	pSpace->clear();
	return true;
}
PY_AUTO_MODULE_FUNCTION( RETOK, clearSpace, ARG( SpaceID, END ), BigWorld )


/*~ function BigWorld.clearAllSpaces
 *
 *	As the name implies, clears all currently loaded spaces. If the optional
 *	Boolean parameter is True client only spaces will not be cleared.
 *	Default is False.
 *
 *	@param	keepClientOnlySpaces	If true this function will leave spaces
 *									created locally intact.
 */
static void clearAllSpaces( bool keepClientOnlySpaces )
{
	BW_GUARD;
	ChunkManager::instance().clearAllSpaces( keepClientOnlySpaces );
	PyGC_Collect();
}
PY_AUTO_MODULE_FUNCTION( RETVOID, clearAllSpaces,
	OPTARG( bool, false, END ), BigWorld )


/*~ function BigWorld.clearEntitiesAndSpaces
 *
 *	As the name implies, removes all entities (including the player)
 *	and clear all currently loaded spaces. Equivalent to calling
 *	BigWorld.resetEntityManager followed by BigWorld.clearAllSpaces.
 *
 */
static void clearEntitiesAndSpaces()
{
	BW_GUARD;
	EntityManager::instance().clearAllEntities(false);
	ChunkManager::instance().clearAllSpaces();
}
PY_AUTO_MODULE_FUNCTION( RETVOID, clearEntitiesAndSpaces, END, BigWorld )


/*~ function BigWorld.spaceLoadStatus
*
*	This function queries the chunk loader to see how much of the current camera
*	space has been loaded.  It queries the chunk loader to see the distance
*  to the currently loading chunk ( the chunk loader loads the closest chunks
*	first ).  A percentage is returned so that scripts can use the information
*	to create for example a teleportation progress bar.
*
*	@param (optional) distance The distance to check for.  By default this is
*	set to the current far plane.
*	@return float Rough percentage of the loading status
*/
/**
*	This function queries the chunk loader to see how much of the current camera
*	space has been loaded.  It queries the chunk loader to see the distance
*  to the currently loading chunk ( the chunk loader loads the closest chunks
*	first. )  A percentage is returned so that scripts can use the information
*	to create for example a teleportation progress bar.
*/
static float spaceLoadStatus( float distance = -1.f )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();

	// If successful, get closest unloaded chunk distance
	if ( pSpace )
	{
		if (distance < 0.f)
		{
			distance = Moo::rc().camera().farPlane();
		}
		float chunkDist = ChunkManager::instance().closestUnloadedChunk( pSpace );
		float streamDist = Terrain::BasicTerrainLodController::instance().closestUnstreamedBlock();

		float dist = std::min( chunkDist, streamDist );

		return std::min( 1.f, dist / distance );
	}
	else
	{
		// no space, return 0
		return 0.0f;
	}
}
PY_AUTO_MODULE_FUNCTION( RETDATA, spaceLoadStatus, OPTARG( float, -1.f, END ), BigWorld )



/*~ function BigWorld.reloadTextures
 *
 *	This function recompresses all textures to their .dds equivalents,
 *	without restarting the game.  The procedures may take a while, depending
 *	on how many textures are currently loaded by the engine.
 *	This is especially useful if you have updated the texture_detail_levels.xml
 *	file, for example to change normal maps from 32 bit to 16 bits per pixel.
 */
/**
 *	This function recompresses all textures to their .dds equivalents,
 *	without restarting the game.  The procedures may take a while, depending
 *	on how many textures are currently loaded by the engine.
 *	This is especially useful if you have updated the texture_detail_levels.xml
 *	file, for example to change normal maps from 32 bit to 16 bits per pixel.
 */
static void reloadTextures()
{
	BW_GUARD;
	Moo::TextureManager::instance()->recalculateDDSFiles();
}
PY_AUTO_MODULE_FUNCTION( RETVOID, reloadTextures, END, BigWorld )


/*~ function BigWorld.restartGame
 *
 *  This function restarts the game.  It can be used to restart the game
 *	after certain graphics options have been changed that require a restart.
 */
/**
 *  This function restarts the game.  It can be used to restart the game
 *	after certain graphics options have been changed that require a restart.
 */
static void restartGame()
{
	BW_GUARD;
	App::instance().quit(true);
}
PY_AUTO_MODULE_FUNCTION( RETVOID, restartGame, END, BigWorld )


/*~ function BigWorld.listVideoModes
 *
 *	Lists video modes available on current display device.
 *	@return	list of 5-tuples (int, int, int, int, string).
 *				(mode index, width, height, BPP, description)
 */
static PyObject * listVideoModes()
{
	BW_GUARD;
	Moo::DeviceInfo info = Moo::rc().deviceInfo( Moo::rc().deviceIndex() );
	PyObject * result = PyList_New(info.displayModes_.size());

	typedef std::vector< D3DDISPLAYMODE >::const_iterator iterator;
	iterator modeIt = info.displayModes_.begin();
	iterator modeEnd = info.displayModes_.end();
	for (int i=0; modeIt < modeEnd; ++i, ++modeIt)
	{
		PyObject * entry = PyTuple_New(5);
		PyTuple_SetItem(entry, 0, Script::getData(i));
		PyTuple_SetItem(entry, 1, Script::getData(modeIt->Width));
		PyTuple_SetItem(entry, 2, Script::getData(modeIt->Height));

		int bpp = 0;
		switch (modeIt->Format)
		{
		case D3DFMT_A2R10G10B10:
			bpp = 32;
			break;
		case D3DFMT_A8R8G8B8:
			bpp = 32;
			break;
		case D3DFMT_X8R8G8B8:
			bpp = 32;
			break;
		case D3DFMT_A1R5G5B5:
			bpp = 16;
			break;
		case D3DFMT_X1R5G5B5:
			bpp = 16;
			break;
		case D3DFMT_R5G6B5:
			bpp = 16;
			break;
		default:
			bpp = 0;
		}
		PyTuple_SetItem(entry, 3, Script::getData(bpp));
		PyObject * desc = PyString_FromFormat( "%dx%dx%d",
			modeIt->Width, modeIt->Height, bpp);
		PyTuple_SetItem(entry, 4, desc);
		PyList_SetItem(result, i, entry);
	}
	return result;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, listVideoModes, END, BigWorld )


/*~ function BigWorld.changeVideoMode
 *
 *  This function allows you to change between fullscreen and
 *	windowed mode.  If switching to fullscreen mode, the video
 *	mode index is used to determine the new resolution.  If
 *	switching to windowed mode, this parameter is ignored.
 *
 *	The exception to the above is if you set the modeIndex
 *	to -1, then both parameters are ignored, and the device
 *	will simply be reset and remain with its current settings.
 *
 *	The video mode index is reported via the listVideoModes function.
 *	@see BigWorld.listVideoModes 
 *
 *	@param	new			int - fullscreen video mode to use.
 *	@param	windowed	bool - True windowed mode is desired.
 *	@return				bool - True on success. False otherwise.
 */
static bool changeVideoMode( int modeIndex, bool windowed )
{
	BW_GUARD;
	return (
		Moo::rc().device() != NULL &&
		Moo::rc().changeMode( modeIndex, windowed ) );
}
PY_AUTO_MODULE_FUNCTION( RETDATA, changeVideoMode, ARG( int, ARG( bool, END ) ), BigWorld )


/*~ function BigWorld.isVideoVSync
 *
 *  Returns current display's vertical sync status.
 *	@return		bool - True if vertical sync is on. False if it is off.
 */
static bool isVideoVSync()
{
	BW_GUARD;
	return Moo::rc().device() != NULL
		? Moo::rc().waitForVBL()
		: false;
}
PY_AUTO_MODULE_FUNCTION( RETDATA, isVideoVSync, END, BigWorld )


/*~ function BigWorld.setVideoVSync
 *
 *  Turns vertical sync on/off.
 *	@param	doVSync		bool - True to turn vertical sync on.
 *						False to turn if off.
 */
static void setVideoVSync( bool doVSync )
{
	BW_GUARD;
	if (Moo::rc().device() != NULL && doVSync != Moo::rc().waitForVBL())
	{
		Moo::rc().waitForVBL(doVSync);
		Moo::rc().changeMode(Moo::rc().modeIndex(), Moo::rc().windowed());
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, setVideoVSync, ARG( bool, END ), BigWorld )

/*~ function BigWorld.isTripleBuffered
 *
 *  Returns current display's triple buffering status.
 *	@return		bool - True if triple buffering is on. False if it is off.
 */
static bool isTripleBuffered()
{
	BW_GUARD;
	return Moo::rc().device() != NULL
		? Moo::rc().tripleBuffering()
		: false;
}
PY_AUTO_MODULE_FUNCTION( RETDATA, isTripleBuffered, END, BigWorld )


/*~ function BigWorld.setTripleBuffering
 *
 *  Turns triple buffering on/off.
 *	@param	doTripleBuffering bool - True to turn triple buffering on.
 *						False to turn if off.
 */
static void setTripleBuffering( bool doTripleBuffering )
{
	BW_GUARD;
	if (Moo::rc().device() != NULL && doTripleBuffering != Moo::rc().tripleBuffering())
	{
		Moo::rc().tripleBuffering(doTripleBuffering);
		Moo::rc().changeMode(Moo::rc().modeIndex(), Moo::rc().windowed());
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, setTripleBuffering, ARG( bool, END ), BigWorld )

/*~ function BigWorld.videoModeIndex
 *
 *  Retrieves index of current video mode.
 *	@return	int		Index of current video mode or zero if render
 *					context has not yet been initialised.
 */
static int videoModeIndex()
{
	BW_GUARD;
	return
		Moo::rc().device() != NULL
			? Moo::rc().modeIndex()
			: 0;
}
PY_AUTO_MODULE_FUNCTION( RETDATA, videoModeIndex, END, BigWorld )


/*~ function BigWorld.isVideoWindowed
 *
 *  Queries current video windowed state.
 *	@return	bool	True is video is windowed. False if fullscreen.
 */
static bool isVideoWindowed()
{
	BW_GUARD;
	return
		Moo::rc().device() != NULL
		? Moo::rc().windowed()
		: false;
}
PY_AUTO_MODULE_FUNCTION( RETDATA, isVideoWindowed, END, BigWorld )


/*~ function BigWorld.resizeWindow
 *
 *  Sets the size of the application window (client area) when running
 *	in windowed mode. Does nothing when running in fullscreen mode.
 *
 *	@param width	The desired width of the window's client area.
 *	@param height	The desired height of the window's client area.
 */
static void resizeWindow( int width, int height )
{
	BW_GUARD;
	if (Moo::rc().windowed())
	{
		App::instance().resizeWindow(width, height);
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, resizeWindow, ARG( int, ARG( int, END ) ),  BigWorld )


/*~ function BigWorld.windowSize
 *
 *  Returns size of application window when running in windowed mode. This
 *	is different to the screen resolution when running in fullscreen mode
 *	(use listVideoModes and videoModeIndex functions to get the screen
 *	resolution in fullscreen mode).
 *
 *	This function is deprecated. Use screenSize, instead.
 *
 *	@return		2-tuple of floats (width, height)
 */
static PyObject * py_windowSize( PyObject * args )
{
	BW_GUARD;
	float width = Moo::rc().screenWidth();
	float height = Moo::rc().screenHeight();
	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SetItem( pTuple, 0, Script::getData( width ) );
	PyTuple_SetItem( pTuple, 1, Script::getData( height ) );
	return pTuple;
}
PY_MODULE_FUNCTION( windowSize, BigWorld )


/*~ function BigWorld.changeFullScreenAspectRatio
 *
 *  Changes screen aspect ratio for full screen mode.
 *	@param	ratio		the desired aspect ratio: float (width/height).
 */
static void changeFullScreenAspectRatio( float ratio )
{
	BW_GUARD;
	if (Moo::rc().device() != NULL)
	{
		Moo::rc().fullScreenAspectRatio( ratio );
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, changeFullScreenAspectRatio, ARG( float, END ), BigWorld )

/*~ function BigWorld.getFullScreenAspectRatio
 *	@components{ client }
 *
 *	This function returns an estimate of the amount of memory the application
 *	is currently using.
 */
static float getFullScreenAspectRatio()
{
	BW_GUARD;
	return Moo::rc().fullScreenAspectRatio();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, getFullScreenAspectRatio, END, BigWorld )


/*~ function BigWorld.sinkKeyEvents
 *
 *	Adds a global event handler for the given key. The handler will exist up to and
 *	including the next "key up" event for the given key. This is useful if you process 
 *	the	key down event and want to stop	all subsequent key events for a particular key 
 *	from occuring. For example, it is useful when the GUI state is changed in the
 *	GUI component's handleKeyDown and you don't want the new GUI state to receive
 *	the subsequent char or key up events (i.e. the user would have to fully let go
 *	of the key and press it again before anything receives more events from that key).
 *
 *	The handler should be a	class instance with methods "handleKeyEvent" and 
 *	"handleCharEvent". The handler methods should return True to override the
 *	event and stop it from being passed to any other handleres, or False to
 *	allow the event to continue as per normal.
 *
 *	If no event handler is specified, then it will sink all events up to and
 *	including the next key-up event.
 *
 *	When the 
 *
 *	@param key				The key code to be routed to the sink.
 *	@param [optional] sink	The class instance that will process the key events.
 *							If not specified, it will sink all events for the 
 *							given key up to and including the next key up.
 */
static PyObject* py_sinkKeyEvents( PyObject* args )
{
	BW_GUARD;

	int keycode=0;
	PyObject* pyhandler=NULL;

	if (!PyArg_ParseTuple(args, "i|O", &keycode, &pyhandler))
	{
		PyErr_Format(PyExc_TypeError, "Expects key code as first parameter, and an optional handler class instance as second parameter.");
		return NULL;
	}

	g_keyEventSinks.push_back( std::make_pair(KeyCode::Key(keycode), pyhandler) );
	Py_XINCREF(pyhandler);

	Py_RETURN_NONE;
}
PY_MODULE_FUNCTION( sinkKeyEvents, BigWorld )


// -----------------------------------------------------------------------------
// Section: BigWorld module: Basic script services (callbacks and consoles)
// -----------------------------------------------------------------------------

extern double getServerTime();

/*~ function BigWorld.time
 *
 *	This function returns the client time.  This is the time that the player's
 *	entity is currently ticking at.  Other entities are further back in time,
 *	the further they are from the player entity.
 *
 *	@return		a float.  The time on the client.
 */
/**
 *	Returns the current time.
 */
static PyObject * py_time( PyObject * args )
{
	BW_GUARD;
	return PyFloat_FromDouble( App::instance().getTime() );
}
PY_MODULE_FUNCTION( time, BigWorld )


static PyObject * py_serverTime( PyObject * args );
/*~ function BigWorld.stime
 *	@components{ client }
 *	This function is deprecated. BigWorld.serverTime() used be used instead.
 */
/**
 *	Returns the current server time. Retained for backwards-compatibility.
 */
static PyObject * py_stime( PyObject * args )
{
	BW_GUARD;
	return py_serverTime( args );
}
PY_MODULE_FUNCTION( stime, BigWorld )


/*~ function BigWorld.serverTime
 *
 *	This function returns the server time.  This is the time that all entities
 *	are at, as far as the server itself is concerned.  This is different from
 *	the value returned by the BigWorld.time() function, which is the time on
 *	the client.
 *
 *	@return		a float.  This is the current time on the server.
 */
/**
 *	Returns the current server time.
 */
static PyObject * py_serverTime( PyObject * args )
{
	BW_GUARD;
	return PyFloat_FromDouble( getServerTime() );
}
PY_MODULE_FUNCTION( serverTime, BigWorld )

static XConsole * s_pOutConsole = NULL;
static XConsole * s_pErrConsole = NULL;

/**
 *	This class implements a PyOutputWriter with the added functionality of
 *	writing to the Python console.
 */
class BWOutputWriter : public PyOutputWriter
{
public:
	BWOutputWriter( bool errNotOut, const char * fileText ) :
		PyOutputWriter( fileText, bool(ENABLE_PYTHON_LOG) ),
		errNotOut_( errNotOut )
	{
	}

private:
	virtual void printMessage( const std::string & msg );

	bool	errNotOut_;
};


/**
 *	This method implements the default behaviour for printing a message. Derived
 *	class should override this to change the behaviour.
 */
void BWOutputWriter::printMessage( const std::string & msg )
{
	BW_GUARD;
	RemoteStepper::step( msg, false );

	XConsole * pXC = errNotOut_ ? s_pErrConsole : s_pOutConsole;
	if (pXC != NULL) pXC->print( msg );

	this->PyOutputWriter::printMessage( msg );
}

// -----------------------------------------------------------------------------
// Section: BWExceptionHook
// -----------------------------------------------------------------------------
static PyObject * py__BWExceptHook( PyObject * args )
{
	BW_GUARD;	
	PyObject* internalHook = PySys_GetObject( "__excepthook__" );
	if (!internalHook)
	{
		CRITICAL_MSG( "_BWExceptHook: sys.__excepthook__ does not exist!" );
		return NULL;
	}

	// Turn off warnings about accessing files from the main thread and then
	// call the normal exception hook.
	BWResource::WatchAccessFromCallingThreadHolder holder( false );
	return PyObject_CallObject( internalHook, args );
}
PY_MODULE_FUNCTION( _BWExceptHook, BigWorld )


// -----------------------------------------------------------------------------
// Section: class PyLoggerMessageForwarder
// -----------------------------------------------------------------------------

#if ENABLE_DPRINTF
namespace {

class PyLoggerMessageForwarder : public DebugMessageCallback
{
public:
	const static size_t MAX_QUEUED_MESSAGES = 500;
	const static size_t MAX_MESSAGES_PER_TICK	= 5;	

public:
	PyLoggerMessageForwarder() : 
		enabled_( false ),
		handlingMessage_( false )
	{
		DebugFilter::instance().addMessageCallback( this );
	}

	~PyLoggerMessageForwarder()
	{
		DebugFilter::instance().deleteMessageCallback( this );
	}

	virtual bool handleMessage( int componentPriority,
		int messagePriority, const char * format, va_list argPtr )
	{
		if (handlingMessage_)
		{
			return false;
		}

		handlingMessage_ = true;

		if (enabled_ && s_pOutConsole && 
			messagePriority > MESSAGE_PRIORITY_NOTICE &&
			messagePriority != MESSAGE_PRIORITY_SCRIPT)
		{
			char buffer[1024];
			const int written = 
				bw_vsnprintf( buffer, sizeof(buffer), format, argPtr );
			buffer[ sizeof(buffer)-1 ] = NULL;

			std::string prefix = messagePrefix( DebugMessagePriority(messagePriority) );
			std::string fullMsg = prefix + std::string(": ") + buffer;
			
			if (MainThreadTracker::isCurrentThreadMain())
			{
				print( fullMsg );
			}
			else
			{
				SimpleMutexHolder mutexHolder( mutex_ );
				messages_.push_back( fullMsg );
				if (messages_.size() > MAX_QUEUED_MESSAGES)
				{
					messages_.pop_front();
				}
			}
		}

		handlingMessage_ = false;
		return false;
	}

	void flushMessages()
	{
		SimpleMutexHolder mutexHolder( mutex_ );

		size_t numPrinted = 0;
		for (size_t i = 0; i < messages_.size() && i < MAX_MESSAGES_PER_TICK; i++)
		{
			print( messages_.front() );
			messages_.pop_front();
		}
	}

	void print( const std::string& msg )
	{
		MF_ASSERT( MainThreadTracker::isCurrentThreadMain() );

		PyOutputWriter::logToFile( msg );

		if (s_pOutConsole)
		{
			s_pOutConsole->print( msg );
		}
	}

	void enable( bool enable ) { enabled_ = enable; }

private:
	SimpleMutex mutex_;
	std::deque<std::string> messages_;

	bool enabled_;
	bool handlingMessage_;
};

PyLoggerMessageForwarder s_pyLoggerMessageForwarder;

} // anonymous namespace

#endif // #if ENABLE_DPRINTF



// -----------------------------------------------------------------------------
// Section: BigWorldClientScript namespace functions
// -----------------------------------------------------------------------------

/// Reference the modules we want to use, to make sure they are linked in.
extern int BoundingBoxGUIComponent_token;
extern int PySceneRenderer_token;
static int GUI_extensions_token =
	BoundingBoxGUIComponent_token |
	PySceneRenderer_token;

extern int FlexiCam_token;
extern int CursorCamera_token;
extern int FreeCamera_token;
static int Camera_token =
	FlexiCam_token |
	CursorCamera_token |
	FreeCamera_token;

extern int PyChunkModel_token;
static int Chunk_token = PyChunkModel_token;

extern int Math_token;
extern int GUI_token;
extern int ResMgr_token;
extern int Pot_token;
extern int PyScript_token;
static int s_moduleTokens =
	Math_token |
	GUI_token |
	GUI_extensions_token |
	ResMgr_token |
	Camera_token |
	Chunk_token |
	Pot_token |
	PyScript_token;


//extern void memNow( const std::string& token );

/**
 *	Client scripting initialisation function
 */
bool BigWorldClientScript::init( DataSectionPtr engineConfig )
{
	BW_GUARD;	
//	memNow( "Before base script init" );

#if ENABLE_DPRINTF
	if (engineConfig)
	{
		bool enable = engineConfig->readBool( "debug/logMessagesToConsole", false );
		s_pyLoggerMessageForwarder.enable( enable );
	}
#endif // #if ENABLE_DPRINTF

	// Particle Systems are creatable from Python code
	RemoteStepper::step( "ParticleSystemManager::init" );
	MF_VERIFY( ParticleSystemManager::init() );

	RemoteStepper::step( "Script::init" );
	// Call the general init function

	PyImportPaths paths;
	paths.addPath( EntityDef::Constants::entitiesClientPath() );
	paths.addPath( EntityDef::Constants::userDataObjectsClientPath() );

	if (!Script::init( paths, "client" ) )
	{
		return false;
	}

//	memNow( "After base script init" );

	RemoteStepper::step( "Import BigWorld" );

	// Initialise the BigWorld module
	PyObject * pBWModule = PyImport_AddModule( "BigWorld" );

	// Set the 'Entity' class into it as an attribute
	if (PyObject_SetAttrString( pBWModule, "Entity",
			(PyObject *)&Entity::s_type_ ) == -1)
	{
		return false;
	}
	// Set the 'UserDataObject' class into it as an attribute
	if (PyObject_SetAttrString( pBWModule, "UserDataObject",
			(PyObject *)&UserDataObject::s_type_ ) == -1)
	{
		return false;
	}

	// We implement our own stderr / stdout so we can see Python's output
	PyObject * pSysModule = PyImport_AddModule( "sys" );	// borrowed

	char fileText[ 256 ];

#ifdef _DEBUG
		const char * config = "Debug";
#elif defined( _HYBRID )
		const char * config = "Hybrid";
#else
		const char * config = "Release";
#endif

	time_t aboutTime = time( NULL );

	bw_snprintf( fileText, sizeof(fileText), "BigWorld %s Client (compiled at %s) starting on %s",
		config,
		App::instance().compileTime(),
		ctime( &aboutTime ) );

#if !BWCLIENT_AS_PYTHON_MODULE
	RemoteStepper::step( "Overridding stdout/err" );

	BWOutputWriter* pNewStdErr = new BWOutputWriter( true, fileText );
	PyObject_SetAttrString( pSysModule, "stderr", pNewStdErr );
	Py_DecRef(pNewStdErr);

	BWOutputWriter* pNewStdOut = new BWOutputWriter( false, fileText );
	PyObject_SetAttrString( pSysModule, "stdout", pNewStdOut );
	Py_DecRef(pNewStdOut);

	// TODO: Should decrement these new objects.

	RemoteStepper::step( "EntityType init" );
#endif // !BWCLIENT_AS_PYTHON_MODULE

	// Override the exception hook to avoid warnings during tracebacks
	PyObject* bwExceptHook = PyObject_GetAttrString( pBWModule, "_BWExceptHook" );
	if (bwExceptHook)
	{
		PySys_SetObject( "excepthook", bwExceptHook );
		Py_DecRef( bwExceptHook );
	}

	// Insert physics constants
#	define INSERT_PHYSICS_CONSTANT( NAME )									\
	PyModule_AddIntConstant( pBWModule, #NAME, Physics::NAME );				\

	INSERT_PHYSICS_CONSTANT( DUMMY_PHYSICS );
	INSERT_PHYSICS_CONSTANT( STANDARD_PHYSICS );
	INSERT_PHYSICS_CONSTANT( HOVER_PHYSICS );
	INSERT_PHYSICS_CONSTANT( CHASE_PHYSICS );
	INSERT_PHYSICS_CONSTANT( TURRET_PHYSICS );

#	undef INSERT_PHYSICS_CONSTANT

	// Load all the standard entity scripts
	bool ret = EntityType::init();
	// Load all the User Data Object Types
	ret = ret && UserDataObjectType::init();

	RemoteStepper::step( "BigWorldClientScript finished" );

	return ret;
}


/**
 *	Client scripting termination function
 */
void BigWorldClientScript::fini()
{
	BW_GUARD;

	for (KeyPyObjectPairList::iterator it = g_keyEventSinks.begin(); 
		it != g_keyEventSinks.end(); it++)
	{
		Py_XDECREF(it->second);
	}

	g_keyEventSinks.clear();

	PyOutputWriter::fini();
	Script::fini();
	EntityType::fini();
	MetaDataType::fini();
	ParticleSystemManager::fini();
}

/**
 *	Does per-frame house keeping.
 */
void BigWorldClientScript::tick()
{
#if ENABLE_DPRINTF
	s_pyLoggerMessageForwarder.flushMessages();
#endif 
}

/**
 *	Posts the given event off to the script system 
 */
bool BigWorldClientScript::sinkKeyboardEvent( const InputEvent& event )
{
	if (g_keyEventSinks.empty())
	{
		return false;
	}

	bool handled = false;

	// Remember which item is the last one because the Python callbacks
	// may add another item to the list which we don't want to process
	// until next time through.
	KeyPyObjectPairList::iterator lastIt = --g_keyEventSinks.end();

	// Keep going until someone reports it as handled
	KeyPyObjectPairList::iterator it = g_keyEventSinks.begin();

	do
	{
		bool remove = false;
		KeyCode::Key sinkKey = it->first;
		PyObject* pyhandler = it->second;

		switch( event.type_ )
		{
		case InputEvent::KEY:
			{
				const KeyEvent& keyEvent = event.key_;
				if ( sinkKey == keyEvent.key() )
				{
					if ( pyhandler )
					{
						PyObject * ret = 
							Script::ask(PyObject_GetAttrString( pyhandler, "handleKeyEvent" ),
										Script::getData( keyEvent ),
										"Keyboard event sink handleKeyEvent: " );

						Script::setAnswer( ret, handled, "Keyboard event sink handleKeyEvent retval" );
					}
					else
					{
						handled = true;
					}

					// pop the event off if this is the keyup.
					remove = keyEvent.isKeyUp();
				}
				break;
			}

		default:
			break;
		}

		if (remove)
		{
			it = g_keyEventSinks.erase(it);
			Py_XDECREF(pyhandler);
		}
		else
		{
			++it;
		}
	} while (it != lastIt && !handled);

	return handled;
}

/**
 *	Clears and releases all existing spaces.
 */
void BigWorldClientScript::clearSpaces()
{
	BW_GUARD;
	// this has to be called at a different time
	// than fini, that's why it's a separate method
	while (!s_clientSpaces.empty())
	{
		s_clientSpaces.back()->clear();
		s_clientSpaces.pop_back();
	}
}


/**
 *	This function gets the consoles that python output and errors appear on.
 */
void BigWorldClientScript::getPythonConsoles(
	XConsole *& pOutConsole, XConsole *& pErrConsole )
{
	BW_GUARD;
	pOutConsole = s_pOutConsole;
	pErrConsole = s_pErrConsole;
}

/**
 *	This function sets the consoles that python output and errors appear on.
 */
void BigWorldClientScript::setPythonConsoles(
	XConsole * pOutConsole, XConsole * pErrConsole )
{
	BW_GUARD;
	s_pOutConsole = pOutConsole;
	s_pErrConsole = pErrConsole;
}


// TODO:PM This is probably not the best place for this.
/**
 *	This function adds an alert message to the display.
 */
bool BigWorldClientScript::addAlert( const char * alertType, const char * alertName )
{
	BW_GUARD;
	bool succeeded = false;

	PyObjectPtr pModule = PyObjectPtr(
			PyImport_ImportModule( "Helpers.alertsGui" ),
			PyObjectPtr::STEAL_REFERENCE );

	if (pModule)
	{
		PyObjectPtr pInstance = PyObjectPtr(
			PyObject_GetAttrString( pModule.getObject(), "instance" ),
			PyObjectPtr::STEAL_REFERENCE );

		if (pInstance)
		{
			PyObjectPtr pResult = PyObjectPtr(
				PyObject_CallMethod( pInstance.getObject(),
									"add", "ss", alertType, alertName ),
				PyObjectPtr::STEAL_REFERENCE );

			if (pResult)
			{
				succeeded = true;
			}
		}
	}

	if (!succeeded)
	{
		PyErr_PrintEx(0);
		WARNING_MSG( "BigWorldClientScript::addAlert: Call failed.\n" );
	}

	return succeeded;
}

/**
 *	Wrapper for list of strings used by the
 *	createTranslationOverrideAnim method.
 */
class MyFunkySequence : public PySTLSequenceHolder< std::vector<std::string> >
{
public:
	MyFunkySequence() :
		PySTLSequenceHolder< std::vector<std::string> >( strings_, NULL, true )
	{}

	std::vector<std::string>	strings_;
};

/*~ function BigWorld createTranslationOverrideAnim
 *  This function is a tool which can be used to alter skeletal animations so
 *  that they can be used with models which have skeletons of different
 *  proportions. This is achieved by creating a new animation which is based
 *  on a given animation, but replaces the translation component for each node
 *  with that of the beginning of the same node in a reference animation. As
 *  the translation should not change in a skeletal system (bones do not change
 *  size or shape), this effectively re-fits the animation on to a differently
 *  proportioned model. This operates by creating a new animation file, and
 *  is not intended for in-game use.
 *  @param baseAnim A string containing the name (including path) of the
 *  animation file on which the new file is to be based.
 *  @param translationReferenceAnim A string containing the name (including path)
 *  of the animation file whose first frame contains the translation which will
 *  be used for the new animation. This should have the same proportions as are
 *  desired for the new animation.
 *  @param noOverrideChannels A list of strings containing the names of the nodes
 *  that shouldn't have their translation overridden. These nodes will not be
 *  scaled to the proportions provided by translationReferenceAnim in the new
 *  animation.
 *  @param outputAnim A string containing the name (including path) of the
 *  animation file to which the new animation will be saved.
 *  @return None
 */
static void createTranslationOverrideAnim( const std::string& baseAnim,
										  const std::string& translationReferenceAnim,
										  const MyFunkySequence& noOverrideChannels,
										  const std::string& outputAnim )
{
	BW_GUARD;
	Moo::AnimationPtr pBase = Moo::AnimationManager::instance().find( baseAnim );
	if (!pBase.hasObject())
	{
		ERROR_MSG( "createTranslationOverrideAnim - Unable to open animation %s\n", baseAnim.c_str() );
		return;
	}

	Moo::AnimationPtr pTransRef = Moo::AnimationManager::instance().find( translationReferenceAnim );
	if (!pTransRef.hasObject())
	{
		ERROR_MSG( "createTranslationOverrideAnim - Unable to open animation %s\n", translationReferenceAnim.c_str() );
		return;
	}
	Moo::AnimationPtr pNew = new Moo::Animation();

	pNew->translationOverrideAnim( pBase, pTransRef, noOverrideChannels.strings_ );

	pNew->save( outputAnim );
}

PY_AUTO_MODULE_FUNCTION( RETVOID, createTranslationOverrideAnim, ARG(
	std::string, ARG( std::string, ARG( MyFunkySequence, ARG( std::string,
	END ) ) ) ), BigWorld )

#if ENABLE_WATCHERS
/*~ function BigWorld.memUsed
 *	@components{ client }
 *
 *	This function returns an estimate of the amount of memory the application
 *	is currently using.
 */
extern uint32 memUsed();
PY_AUTO_MODULE_FUNCTION( RETDATA, memUsed, END, BigWorld )
#endif

/*~ function BigWorld.screenWidth
 *	Returns the width of the current game window.
 *	@return float
 */
float screenWidth()
{
	BW_GUARD;
	return Moo::rc().screenWidth();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, screenWidth, END, BigWorld )

/*~ function BigWorld.screenHeight
 *	Returns the height of the current game window.
 *	@return float
 */
float screenHeight()
{
	BW_GUARD;
	return Moo::rc().screenHeight();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, screenHeight, END, BigWorld )

/*~ function BigWorld.screenSize
 *	Returns the width and height of the current game window as a tuple.
 *	@return (float, float)
 */
PyObject * screenSize()
{
	BW_GUARD;
	float width = Moo::rc().screenWidth();
	float height = Moo::rc().screenHeight();
	PyObject * pTuple = PyTuple_New( 2 );
	PyTuple_SetItem( pTuple, 0, Script::getData( width ) );
	PyTuple_SetItem( pTuple, 1, Script::getData( height ) );
	return pTuple;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, screenSize, END, BigWorld )


/*~ function BigWorld.screenShot
 *	This method takes a screenshot and writes the image to disk. The output folder
 *	is configured by the 'screenShot/path' section in engine_config.xml.
 *	@param format Optional string. The format of the screenshot to be outputed,
 *  can be on of "bmp", "jpg", "tga", "png" or "dds". The default comes from resources.xml.
 *  @param name Optional string. This is the root name of the screenshot to generate.
 *  A unique number will be postpended to this string. The default comes from resources.xml.
 */
 void screenShot( std::string format, std::string name )
{
	BW_GUARD;
	DataSectionPtr settingsDS = 
		AppConfig::instance().pRoot()->openSection( "screenShot/path" );

	PathedFilename pathedFile( settingsDS,
								"", PathedFilename::BASE_EXE_PATH );

	std::string fullName = pathedFile.resolveName() + "/" + name;
	BWResource::ensureAbsolutePathExists( fullName );

	Moo::rc().screenShot( format, fullName );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, screenShot,
	OPTARG( std::string, AppConfig::instance().pRoot()->readString("screenShot/extension", "bmp"),
	OPTARG( std::string, AppConfig::instance().pRoot()->readString("screenShot/name", "shot"), END ) ), BigWorld )

/*~ function BigWorld.connectedEntity
 *	This method returns the entity that this application is connected to. The
 *	connected entity is the server entity that is responsible for collecting and
 *	sending data to this client application. It is also the only client entity
 *	that has an Entity.base property.
 */
Entity * connectedEntity()
{
	BW_GUARD;
	EntityID connectedID = 0;
	if (EntityManager::instance().pServer())
	{
		connectedID = EntityManager::instance().pServer()->connectedID();
	}
	return EntityManager::instance().getEntity( connectedID );
}
PY_AUTO_MODULE_FUNCTION( RETDATA, connectedEntity, END, BigWorld )



/*~ function BigWorld.savePreferences
 *
 *  Saves the current preferences (video, graphics and script) into
 *	a XML file. The name of  the file to the writen is defined in the
 *	<preferences> field in engine_config.xml.
 *
 *	@return		bool	True on success. False on error.
 */
static bool savePreferences()
{
	BW_GUARD;
	BWResource::WatchAccessFromCallingThreadHolder watchAccess( false );
	return App::instance().savePreferences();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, savePreferences, END, BigWorld )

#include "resmgr/multi_file_system.hpp"
typedef std::map< PyObject* , std::vector<PyObject*> > ReferersMap;
/*~ function BigWorld.dumpRefs
 *	Dumps references to each object visible from the entity tree.
 *	This does not include all objects, and may not include all references.
 */
void dumpRefs()
{
	BW_GUARD;
	// Build a map of the objects that reference each other object.
	ReferersMap	referersMap;

	PyObject * pSeed = PyImport_AddModule( "BigWorld" );
	referersMap[pSeed].push_back( pSeed );	// refers to itself for seeding

	std::vector<PyObject*> stack;
	stack.push_back( pSeed );
	while (!stack.empty())
	{
		PyObject * pLook = stack.back();
		stack.pop_back();

		// go through all objects accessible from here

		// look in the dir and get attributes
		PyObject * pDirSeq = PyObject_Dir( pLook );
		int dlen = 0;
		if (pDirSeq != NULL)
		{
			dlen = PySequence_Length( pDirSeq );
		} else { PyErr_Clear(); }
		for (int i = 0; i < dlen; i++)
		{
			PyObject * pRefereeName = PySequence_GetItem( pDirSeq, i );
			PyObject * pReferee = PyObject_GetAttr( pLook, pRefereeName );
			Py_DECREF( pRefereeName );
			if (pReferee == NULL)
			{
				// shouldn't get errors like this (hmmm)
				PyErr_Clear();
				WARNING_MSG( "%s in dir of 0x%08X but cannot access it\n",
					PyString_AsString( pRefereeName ), pLook );
				continue;
			}
			if (pReferee->ob_refcnt == 1)
			{
				// if it was created just for us we don't care
				Py_DECREF( pReferee );
				continue;
			}
			// ok we have an object that is part of the tree in pReferee

			// find/create the vector of referers to pReferee
			std::vector<PyObject*> & referers = referersMap[pReferee];
			// if pLook is first to refer to this obj then traverse it (later)
			if (referers.empty())
				stack.push_back( pReferee );
			// record the fact that pLook refers to this obj
			referers.push_back( pLook );

			Py_DECREF( pReferee );
		}
		Py_XDECREF( pDirSeq );

		// look in the sequence
		int slen = 0;
		if (PySequence_Check( pLook )) slen = PySequence_Size( pLook );
		for (int i = 0; i < slen; i++)
		{
			PyObject * pReferee = PySequence_GetItem( pLook, i );
			if (pReferee == NULL)
			{
				// _definitely_ shouldn't get errors like this! (but do)
				PyErr_Clear();
				WARNING_MSG( "%d seq in 0x%08X but cannot access item %d\n",
					slen, pLook, i );
				continue;
			}
			MF_ASSERT_DEV( pReferee != NULL );
			if (pReferee->ob_refcnt == 1)
			{
				// if it was created just for us we don't care
				Py_DECREF( pReferee );
				continue;
			}

			// find/create the vector of referers to pReferee
			std::vector<PyObject*> & referers = referersMap[pReferee];
			// if pLook is first to refer to this obj then traverse it (later)
			if (referers.empty())
				stack.push_back( pReferee );
			// record the fact that pLook refers to this obj
			referers.push_back( pLook );

			Py_DECREF( pReferee );
		}

		// look in the mapping
		PyObject * pMapItems = NULL;
		int mlen = 0;
		if (PyMapping_Check( pLook ))
		{
			pMapItems = PyMapping_Items( pLook );
			mlen = PySequence_Size( pMapItems );
		}
		for (int i = 0; i < mlen; i++)
		{
		  PyObject * pTuple = PySequence_GetItem( pMapItems, i );
		  int tlen = PySequence_Size( pTuple );
		  for (int j = 0; j < tlen; j++)
		  {
			PyObject * pReferee = PySequence_GetItem( pTuple, j );
			MF_ASSERT_DEV( pReferee != NULL );
			if (pReferee->ob_refcnt == 2)
			{
				// if it was created just for us we don't care
				Py_DECREF( pReferee );
				continue;
			}

			// find/create the vector of referers to pReferee
			std::vector<PyObject*> & referers = referersMap[pReferee];
			// if pLook is first to refer to this obj then traverse it (later)
			if (referers.empty())
				stack.push_back( pReferee );
			// record the fact that pLook refers to this obj
			referers.push_back( pLook );

			Py_DECREF( pReferee );
		  }
		  Py_DECREF( pTuple );
		}
		Py_XDECREF( pMapItems );
	}

	time_t now = time( &now );
	std::string nowStr = ctime( &now );
	nowStr.erase( nowStr.end()-1 );
	FILE * f = BWResource::instance().fileSystem()->posixFileOpen(
		"py ref table.txt", "a" );
	fprintf( f, "\n" );
	fprintf( f, "List of references to all accessible from 'BigWorld':\n" );
	fprintf( f, "(as at %s)\n", nowStr.c_str() );
	fprintf( f, "-----------------------------------------------------\n" );

	// Now print out all the objects and their referers
	ReferersMap::iterator it;
	for (it = referersMap.begin(); it != referersMap.end(); it++)
	{
		PyObject * pReferee = it->first;
		PyObject * pRefereeStr = PyObject_Str( pReferee );
		fprintf( f, "References to object at 0x%08X type %s "
				"aka '%s' (found %d/%d):\n",
			pReferee, pReferee->ob_type->tp_name,
			PyString_AsString( pRefereeStr ),
			it->second.size(), pReferee->ob_refcnt );
		Py_DECREF( pRefereeStr );

		for (uint i = 0; i < it->second.size(); i++)
			fprintf( f, "\t0x%08X\n", it->second[i] );
		fprintf( f, "\n" );
	}

	fprintf( f, "-----------------------------------------------------\n" );
	fprintf( f, "\n" );
	fclose( f );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, dumpRefs, END, BigWorld )


/*~ function BigWorld.reloadChunks
 *	Unload all chunks, purge all ".chunk" data sections and begin reloading
 *	the space.
 */
void reloadChunks()
{
	BW_GUARD;
	// remember the camera's chunk
	Chunk * pCameraChunk = ChunkManager::instance().cameraChunk();
	if (pCameraChunk)
	{
		GeometryMapping * pCameraMapping = pCameraChunk->mapping();
		ChunkSpacePtr pSpace = pCameraChunk->space();

		// unload every chunk in sight
		ChunkMap & chunks = pSpace->chunks();
		for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); it++)
		{
			for (uint i = 0; i < it->second.size(); i++)
			{
				Chunk * pChunk = it->second[i];
				if (pChunk->isBound())
				{
					BWResource::instance().purge( pChunk->resourceID() );
					pChunk->unbind( false );
					pChunk->unload();
				}
			}
		}

		// now reload the camera chunk
		ChunkManager::instance().loadChunkExplicitly(
			pCameraChunk->identifier(), pCameraMapping );

		// and repopulate the flora
		Flora::floraReset();
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, reloadChunks, END, BigWorld )

#ifdef ENABLE_MEMORY_MAP
void saveMemoryMap( const std::string& filename )
{
	MemTracker::instance().saveMemoryMap( filename.c_str() );
}

PY_AUTO_MODULE_FUNCTION( RETVOID, saveMemoryMap, ARG( std::string, END ), BigWorld );
#endif


/**
 *	This helper function returns a list of points to navigate between the given
 *	source location to the given destination, for the given ChunkSpace. This is
 *	called by BigWorld.navigatePathPoints().
 */ 
PyObject * navigatePathPoints( Navigator & navigator, 
		ChunkSpace * pChunkSpace, const NavLoc & srcLoc, const Vector3 & dst, 
		float maxSearchDistance, float girth )
{
	char positionString[64];
	char girthString[8];

	if (!srcLoc.valid())
	{
		// Py_ErrFormat doesn't support %f
		bw_snprintf( positionString, sizeof( positionString ), 
			"(%.02f, %.02f, %.02f)", 
			srcLoc.point().x,
			srcLoc.point().y,
			srcLoc.point().z );
		bw_snprintf( girthString, sizeof( girthString ),
			"%.02f", girth );

		PyErr_Format( PyExc_ValueError, 
			"Source position %s is not in an area with "
				"valid navmesh (girth=%s)", 
			positionString, girthString );
		return NULL;
	}

	// Py_ErrFormat doesn't support %f
	NavLoc dstLoc( pChunkSpace, dst, girth );
	if (!dstLoc.valid())
	{
		bw_snprintf( positionString, sizeof( positionString ), 
			"(%.02f, %.02f, %.02f)", 
			dst.x, dst.y, dst.z );
		bw_snprintf( girthString, sizeof( girthString ),
			"%.02f", girth );
		PyErr_Format( PyExc_ValueError, 
			"Destination position %s is not in an area with "
				"valid navmesh (girth=%s)", 
			positionString, girthString );
		return NULL;
	}

	dstLoc.clip();

	Vector3Path pathPoints;

	if (!navigator.findFullPath( srcLoc, dstLoc, maxSearchDistance, 
			/* blockNonPermissive: */ true, pathPoints ))
	{
		PyErr_SetString( PyExc_ValueError, 
			"Full navigation path could not be found" );
		return NULL;
	}

	PyObject * pList = PyList_New( pathPoints.size() );

	Vector3Path::const_iterator iPoint = pathPoints.begin();
	while (iPoint != pathPoints.end())
	{
		PyList_SetItem( pList, iPoint - pathPoints.begin(), 
			Script::getData( *iPoint ) );
		++iPoint;
	}

	return pList;

}


/*~	function BigWorld.navigatePathPoints
 * 	@components{ cell }
 *
 * 	Return a path of points between the given source and destination points in
 * 	the camera space.
 *
 * 	@param src	(Vector3)		The source point in the space.
 * 	@param dst	(Vector3)		The destination point in the space.
 * 	@param maxSearchDistance (float)
 * 								The maximum search distance, defaults to 500m.
 * 	@param girth (float) 		The navigation girth grid to use, defaults to
 * 								0.5m.
 *
 * 	@return (list) 	A list of Vector3 points between the source point to the
 * 					destination point.
 *
 */
PyObject * navigatePathPoints( const Vector3 & src, const Vector3 & dst,
							  float maxSearchDistance, float girth )
{
	static Navigator navigator;

	ChunkSpacePtr pChunkSpace =
		ChunkManager::instance().cameraSpace();

	NavLoc srcLoc( pChunkSpace.get(), src, girth );

	// We clip the srcLoc first, unlike in Entity.navigatePathPoints().
	srcLoc.clip();

	PyObject * pList = ::navigatePathPoints( navigator, pChunkSpace.get(), 
		srcLoc, dst, maxSearchDistance, girth );

	if (!pList)
	{
		return NULL;
	}

	if (!almostEqual( srcLoc.point(), src )) 
	{
		// Clipping the point resulted in a different point than the source,
		// insert it into the list at the beginning.
		PyList_Insert( pList, 0, Script::getData( srcLoc.point() ) );
	}

	return pList;
}

PY_AUTO_MODULE_FUNCTION( RETOWN, navigatePathPoints, ARG( Vector3, 
		ARG( Vector3, OPTARG( float, 500.f, OPTARG( float, 0.5f, END ) ) ) ), 
	BigWorld )


// script_bigworld.cpp
