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

#include "player.hpp"

#include "app.hpp"
#include "entity_flare_collider.hpp"
#include "entity_manager.hpp"
#include "filter.hpp"

#include "chunk/chunk_light.hpp"
#include "chunk/chunk_terrain.hpp"

#include "connection/server_connection.hpp"

#include "duplo/pymodelnode.hpp"

#include "moo/render_target.hpp"
#include "moo/visual_channels.hpp"

#include "particle/py_meta_particle_system.hpp"

#include "romp/lens_effect_manager.hpp"
#include "romp/z_buffer_occluder.hpp"

#include "terrain/base_terrain_block.hpp"


DECLARE_DEBUG_COMPONENT2( "App", 0 )

BW_INIT_SINGLETON_STORAGE( Player )

/// Constructor
Player::Player() :
	pEntity_( NULL ),
	playerFlareCollider_( NULL )
{
}


// Destructor
Player::~Player()
{
	BW_GUARD;

	MF_ASSERT(pEntity_ == NULL);
}

/**
 *	Player singleton initialisation.
 */
bool Player::doInit()
{
	BW_GUARD;
	LensEffectManager::init();
	return true;
}

/**
 *	Player singleton fini.
 */
bool Player::doFini()
{
	BW_GUARD;
	LensEffectManager::fini();
	return true;
}


/**
 *	Change the entity that the client controls.
 *	A NULL entity pointer is permitted to indicate no entity.
 *
 *	@return True on success, otherwise false.
 */
bool Player::setPlayer( Entity * newPlayer )
{
	BW_GUARD;

	MF_ASSERT( Py_IsInitialized() );

	if (newPlayer != NULL && newPlayer->type().pPlayerClass() == NULL)
	{
		ERROR_MSG( "Player::setPlayer: "
			"newPlayer id %d of type %s does not have an associated player class.\n",
			newPlayer->id(), newPlayer->type().name().c_str() );

		return false;
	}

	if (newPlayer == pEntity_)
	{
		return true;
	}

	// if there's already a player tell it its time has come
	if (pEntity_ != NULL)
	{
		pEntity_->makeNonPlayer();
		//EntityManager::instance().onEntityLeave( pEntity_->id() );

		for (uint i = 0; i < attachedPSs_.size(); i++)
		{
			attachedPSs_[i].pNode_->detach( attachedPSs_[i].pAttachment_ );
		}

		attachedPSs_.clear();

		LensEffectManager::instance().delPhotonOccluder( playerFlareCollider_ );
		playerFlareCollider_ = NULL;
	}

	// we are back to no player now
	pEntity_ = NULL;

	// set a new player if instructed
	if (newPlayer != NULL && newPlayer->type().pPlayerClass() != NULL)
	{
		// make the player script object
		pEntity_ = newPlayer;

		//EntityManager::instance().onEntityEnter(
		//	pEntity_->id(), pPlayerSpace->id(), 0 );
		pEntity_->makePlayer();

		// adorn the model with weather particle systems
		if (pEntity_->pSpace())
		{
			updateWeatherParticleSystems(
				pEntity_->pSpace()->enviro().playerAttachments() );
		}

		if (!ZBufferOccluder::isAvailable())
		{		
			playerFlareCollider_ = new EntityPhotonOccluder( *pEntity_ );
			LensEffectManager::instance().addPhotonOccluder( playerFlareCollider_ );
		}
	}

	return true;
}


/**
 *	Grabs the particle systems that want to be attached to the player,
 *	and attaches them to its model.
 */
void Player::updateWeatherParticleSystems( PlayerAttachments & pa )
{
	BW_GUARD;
	for (uint i = 0; i < attachedPSs_.size(); i++)
	{
		attachedPSs_[i].pNode_->detach( attachedPSs_[i].pAttachment_ );
	}

	attachedPSs_.clear();

	if (pa.empty()) return;
	if (pEntity_ == NULL || pEntity_->pPrimaryModel() == NULL) return;

	for (uint i = 0; i < pa.size(); i++)
	{
		PyObject * pPyNode = pEntity_->pPrimaryModel()->node( pa[i].onNode );
		if (!pPyNode || !PyModelNode::Check( pPyNode ))
		{
			PyErr_Clear();
			continue;
		}

		SmartPointer<PyModelNode> pNode( (PyModelNode*)pPyNode, true );

		if (!pNode->attach( pa[i].pSystem ))
		{
			ERROR_MSG( "Player::updateWeatherParticleSystems: Could not attach "
				"weather particle system to new Player Model\n" );

			PyErr_Clear();
			continue;
		}

		AttachedPS aps;
		aps.pNode_ = pNode;
		aps.pAttachment_ = pa[i].pSystem;
		attachedPSs_.push_back( aps );
	}
}

const float GROUND_TOLERANCE = 0.1f;	// 10 cm 

/**
 *	Do anything special which requires the position to be set,
 *	i.e. send it on to the server. Should be called for all
 *	controlled entities.
 */
void Player::poseUpdateNotification( Entity * pEntity )
{
	BW_GUARD;
	bool onGround;

	// check entity
	MF_ASSERT_DEV( pEntity != NULL );

	// check entity is on server
	if (pEntity->isClientOnly())
	{
		return;
	}

	// check that it has a cell entity
	if (!pEntity->isInWorld()) return;

	// check that we are connected to a server
	ServerConnection * pServer = EntityManager::instance().pServer();
	if (pServer == NULL) return;

	// check that we aren't sending too frequently
	if (App::instance().getTime() - pServer->lastSendTime() <
			pServer->minSendInterval())
		return;

	// check that we aren't under physics correction
	if (!!pEntity->physicsCorrected())
	{
		// return if no packet sent since physics corrected
		if (pEntity->physicsCorrected() == pServer->lastSendTime())
		{
			return;
		}

		// ok that correction is all over now
		pEntity->physicsCorrected( 0.0 );
	}

	// ok, send it then!

	// figure out if it's on the ground...
	Vector3 globalPos = pEntity->position();
	Vector3 globalDir = Vector3::zero();
	ChunkSpacePtr pSpace;
	if ((pSpace = pEntity->pSpace()))
		pEntity->pSpace()->transformCommonToSpace( globalPos, globalDir );

	ChunkSpace::Column * csCol;
	Chunk * oCh;
	ChunkTerrain * pTer;
	float terrainHeight = -1000000.f;
	if (pSpace &&
		(csCol = pSpace->column( globalPos, false )) != NULL &&
		(oCh = csCol->pOutsideChunk()) != NULL &&
		(pTer = ChunkTerrainCache::instance( *oCh ).pTerrain()) != NULL)
	{
		const Matrix & trInv = oCh->transformInverse();
		Vector3 terPos = trInv.applyPoint( globalPos );
		terrainHeight = pTer->block()->heightAt( terPos[0], terPos[2] ) -
			trInv.applyToOrigin()[1];
	}

	// ... get the pose in local coordinates ...
	double unused;
	SpaceID spaceID;
	EntityID vehicleID;
	Vector3 localPos( Vector3::zero() );
	Vector3 err( Vector3::zero() );
	float localAuxVolatile[3] = {0.f, 0.f, 0.f};
	pEntity->filter().getLastInput( unused, spaceID, vehicleID,
		localPos, err, localAuxVolatile );

	onGround =	fabs(globalPos[1] - terrainHeight) < GROUND_TOLERANCE &&
				vehicleID == 0;

	// ... and then add the move
	pServer->addMove(
		pEntity->id(),
		spaceID,
		vehicleID,
		localPos,
		localAuxVolatile[0],
		localAuxVolatile[1],
		localAuxVolatile[2],
		onGround,
		globalPos );
}


/**
 *	Find the chunk that the player is in
 */
Chunk * Player::findChunk()
{
	BW_GUARD;
	if (pEntity_ && pEntity_->pSpace())
	{
		const Vector3 & loc = pEntity_->fallbackTransform().applyToOrigin();
		return pEntity_->pSpace()->findChunkFromPoint( loc );
	}

	return NULL;
}




/*~ function BigWorld player
 *  This sets the player entity, or returns the current player entity if the 
 *  entity argument is not provided. The BigWorld engine assumes that there is 
 *  no more than one player entity at any time. Changing whether an entity is 
 *  the current player entity involves changing whether it is an instance of 
 *  its normal class, or its player class. This is a class whose name equals 
 *  the entity's current class name, prefixed with the word "Player". As an 
 *  example, the player class for Biped would be PlayerBiped.
 *  
 *
 *  The following occurs if a new entity is specified:
 *  
 *  The onBecomeNonPlayer function is called on the old player entity.
 *  
 *  The old player entity becomes an instance of its original class, rather 
 *  than its player class.
 *  
 *  The reference to the current player entity is set to None.
 *  
 *  A player class for the new player entity is sought out.
 *  If the player class for the new player entity is found, then the entity 
 *  becomes an instance of this class. Otherwise, the function immediately 
 *  returns None.
 *  
 *  The onBecomePlayer function is called on the new player entity.
 *  
 *  The reference to the current player entity is set the new player entity.
 *  
 *  @param entity An optional entity. If supplied, this entity becomes the 
 *  current player.
 *  @return If the entity argument is supplied then this is None, otherwise 
 *  it's the current player entity.
 */
/**
 *	Returns the player entity.
 */
static PyObject * py_player( PyObject * args )
{
	BW_GUARD;
	// get the player if desired
	if (PyTuple_Size( args ) == 0)
	{
		Entity * pPlayer = Player::entity();

		if (pPlayer != NULL)
		{
			Py_INCREF( pPlayer );
			return pPlayer;
		}
		else
		{
			Py_Return;
		}
	}

	// otherwise switch player control to this entity
	PyObject * pNewPlayer = PyTuple_GetItem( args, 0 );
	if (!Entity::Check( pNewPlayer ))
	{
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.player: Expected an entity\n" );
		return NULL;
	}

	Player::instance().setPlayer( (Entity*)pNewPlayer );
	Py_Return;
}
PY_MODULE_FUNCTION( player, BigWorld )



/*~ class BigWorld.PlayerMProv
 *
 *	This class inherits from MatrixProvider, and can be used to access the 
 *	position of the current player entity.  Multiple PlayerMProvs can be
 *	created, but they will all have the same value.
 *
 *	There are no attributes or functions available on a PlayerMProv, but it
 *	can be used to construct a PyMatrix from which the position can be read.
 *
 *	A new PlayerMProv is created using BigWorld.PlayerMatrix function.
 */
/**
 *	Helper class to access the matrix of the current player position
 */
class PlayerMProv : public MatrixProvider
{
	Py_Header( PlayerMProv, MatrixProvider )

public:
	PlayerMProv() : MatrixProvider( false, &s_type_ ) {}

	virtual void matrix( Matrix & m ) const
	{
		BW_GUARD;
		Entity * pEntity = Player::entity();
		if (pEntity != NULL)
		{
			m = pEntity->fallbackTransform();
		}
		else
		{
			m.setIdentity();
		}
	}

	PY_DEFAULT_CONSTRUCTOR_FACTORY_DECLARE()
};


PY_TYPEOBJECT( PlayerMProv )

PY_BEGIN_METHODS( PlayerMProv )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PlayerMProv )
PY_END_ATTRIBUTES()

/*~ function BigWorld.PlayerMatrix
 *
 *	This function returns a new PlayerMProv which is a MatrixProvider
 *	that can be used to access the player entity's position.
 */
PY_FACTORY_NAMED( PlayerMProv, "PlayerMatrix", BigWorld )

/**
 *	Get the matrix provider for the player's transform
 */
MatrixProviderPtr Player::camTarget()
{
	BW_GUARD;
	return MatrixProviderPtr( new PlayerMProv(), true );
}


/*~ function BigWorld.playerDead
 *
 *	This method sets whether or not the player is dead.  This information is
 *	used by the snow system to stop snow falling if the player is dead.
 *
 *	@param isDead	an integer as boolean. This is 0 if the player is alive, otherwise
 *	it is non-zero.
 */
/**
 *	Set whether or not the player is dead
 */
static void playerDead( bool isDead )
{
	BW_GUARD;
	Entity * pE = Player::entity();
	if (pE == NULL || !pE->pSpace()) return;
	pE->pSpace()->enviro().playerDead( isDead );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, playerDead, ARG( bool, END ), BigWorld )


bool Player::drawPlayer(bool clearSortedChannel)
{
	BW_GUARD;
	Entity * pPlayer = entity();
	if (pPlayer && pPlayer->pPrimaryModel())
	{
		Moo::LightContainerPtr pLC = Moo::rc().lightContainer();
		Moo::LightContainerPtr pSC = Moo::rc().specularLightContainer();

		Chunk * pChunk = findChunk();
		if ( pChunk )
		{
			ChunkLightCache & clc = ChunkLightCache::instance( *pChunk );
			clc.draw();
			pPlayer->pPrimaryModel()->draw( pPlayer->fallbackTransform(), 1.f );
			Moo::SortedChannel::draw(clearSortedChannel);
			return true;
		}
		Moo::rc().lightContainer( pLC );
		Moo::rc().specularLightContainer( pSC );
	}
	return false;
}

// player.cpp
