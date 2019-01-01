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
#include "subspace.hpp"

#include "moo/light_container.hpp"
#include "moo/render_context.hpp"

#include "world.hpp"
#include "duplo/pymodel.hpp"

#include "cstdmf/debug.hpp"
DECLARE_DEBUG_COMPONENT2( "App", 0 )

/**
 *	Constructor
 */
SubSpace::SubSpace( SceneRoot * pScene, CollisionScene * pObstacles,
		ChunkSpace * pSpace ) :
	pScene_( pScene ),
	pObstacles_( pObstacles ),
	pSpace_( pSpace ),
	transform_( Matrix::identity ),
	transformInverse_( Matrix::identity ),
	transformNonIdentity_( false ),
	pEngineModel_( NULL )
{
}


/**
 * Destructor
 */
SubSpace::~SubSpace()
{
	if (pEngineModel_ != NULL)
	{
		//ModelManager::instance().removeFromScene( pEngineModel_ );
		HACK_MSG( "Commented line that shouldn't be" );
		Py_DECREF( pEngineModel_ );
	}
}


/**
 *	Dark space is inhabited by the models of subspaces
 */
static SubSpace * darkSpace()
{
	static SubSpace * s_darkSpace = NULL;
	if (s_darkSpace == NULL)
	{
		s_darkSpace = new SubSpace( NULL, NULL, NULL );
	}
	return s_darkSpace;
}


/**
 *	Tick this subspace's scene
 */
void SubSpace::tick( float dTime )
{
	if (pEngineModel_ != NULL && this != &World::outside())
	{
		pEngineModel_->move( dTime );
		pEngineModel_->tick( dTime );
		this->transform( pEngineModel_->worldTransform() );
	}
}


/**
 *	Draw this subspace's scene
 */
void SubSpace::draw()
{
}


/**
 *	Engine model accessor. An engine model is created if one did
 *	not previously exist.
 *
 *	@note: Returns a new reference
 */
PyModel * SubSpace::pEngineModel()
{
	if (pEngineModel_ == NULL)
	{
		//pEngineModel_ = ModelManager::instance().model( "models/null.xml" );
		//ModelManager::instance().addToScene( darkSpace(), pEngineModel_ );
		HACK_MSG( "Removed lines that should not be commented" );

		return NULL;
	}

	// Py_INCREF( pEngineModel_ );
	return pEngineModel_;
}

// subspace.cpp
