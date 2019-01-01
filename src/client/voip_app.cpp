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
#include "voip_app.hpp"

#include "bwvoip.hpp"
#include "voip.hpp"
#include "player.hpp"



///////////////////////////////////////////////////////////////////////////////
///  Member Documentation for VOIPApp
///////////////////////////////////////////////////////////////////////////////


/**
 *	@property	VOIPClient* VOIPApp::voipClient_
 *
 *	The pointer to VOIPClient instance. This member is set in VOIPApp::init().
 */


///////////////////////////////////////////////////////////////////////////////
///  End Member Documentation for VOIPApp
///////////////////////////////////////////////////////////////////////////////


/**
 *	The singleton instance of VOIPApp
 */
VOIPApp VOIPApp::s_instance_;

int VOIPApp_token = 1;


/**
 *	Constructor for VOIPApp
 *
 *	The VOIPApp adds its self to the pool of main loop tasks under 'VOIP/App'.
 */
VOIPApp::VOIPApp() :
	voipClient_( NULL )
{
	MainLoopTasks::root().add( this, "VOIP/App", NULL );
}


/**
 *	Destructor
 */
VOIPApp::~VOIPApp()
{
}


/**
 *	This method initalises the VOIPApp by creating the VOIPClient from
 *	the voip.dll.
 *
 *	@note	The VOIPClient is not initialised in this method but instead
 *			intended to be initialised through python.
 */
bool VOIPApp::init()
{
	BW_GUARD;
	voipClient_ = VOIPClient::createVOIPClient();

	MF_ASSERT_DEV( voipClient_ );

	return true;
}


/**
 *	This method cleans up the VOIPApp by destroying the previously created voip
 *	client.
 *
 *	@note	The python module BigWorld.VOIP must not be used after this point.
 */
void VOIPApp::fini()
{
	BW_GUARD;
	VOIPClient::destroyVOIPClient( voipClient_ );
	voipClient_ = NULL;
}


/**
 *	This method ticks the VOIPClient each frame. Passing in the current
 *	position and rotation of the player.
 */
void VOIPApp::tick( float dTime )
{
	BW_GUARD;
	const Entity * playerEntity = Player::entity();

	if( !playerEntity || !playerEntity->isInWorld() )
		return;

	Position3D playerPosition( playerEntity->position() );

	Vector3 playerHeadRotation(	playerEntity->auxVolatile()[0],
								playerEntity->auxVolatile()[1],
								playerEntity->auxVolatile()[2] );


	SpaceID playerSpace = playerEntity->pSpace()->id();

	voipClient_->tick(	playerPosition.x,
						playerPosition.y,
						playerPosition.z,
						playerHeadRotation[0],
						playerHeadRotation[1],
						playerHeadRotation[2],
						playerSpace );
}


/**
 *	This method currently does nothing.
 */
void VOIPApp::draw()
{

}



/**
 *	This function retrieves the VOIPClient possessed by the VOIPApp.
 *
 *	@return	The VOIPClient owned by the VOIPApp instance
 *
 *	@note	This function must not be called before the VOIPApp has been
 *			initialised.
 */
VOIPClient & VOIPApp::getVOIPClient()
{
	BW_GUARD;
	MF_ASSERT_DEV( VOIPApp::s_instance_.voipClient_ != NULL );

	return *( VOIPApp::s_instance_.voipClient_ );
}


// voip_app.cpp
