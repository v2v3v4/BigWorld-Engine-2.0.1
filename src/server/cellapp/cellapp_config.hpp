/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_APP_CONFIG_HPP
#define CELL_APP_CONFIG_HPP

#include "server/entity_app_config.hpp"

class CellAppConfig : public EntityAppConfig
{
public:

	static ServerAppOption< float > loadSmoothingBias;
	static ServerAppOption< float > ghostDistance;
	static ServerAppOption< float > defaultAoIRadius;
	static ServerAppOption< bool > fastShutdown;
	static ServerAppOption< bool > shouldResolveMailBoxes;
	static ServerAppOption< int > entitySpamSize;

	static ServerAppOption< int > maxGhostsToDelete;
	static ServerAppOption< float > minGhostLifespan;
	static ServerAppOption< int > minGhostLifespanInTicks;

	static ServerAppOption< bool > loadDominantTextureMaps;

	static ServerAppOption< float > backupPeriod;
	static ServerAppOption< int > backupPeriodInTicks;

	static ServerAppOption< float > checkOffloadsPeriod;
	static ServerAppOption< int > checkOffloadsPeriodInTicks;

	static ServerAppOption< float > chunkLoadingPeriod;

	static ServerAppOption< int > ghostUpdateHertz;
	static ServerAppOption< double > reservedTickFraction;

	static ServerAppOption< int > obstacleTreeDepth;
	static ServerAppOption< float > sendWindowCallbackThreshold;

	static ServerAppOption< float > offloadHysteresis;
	static ServerAppOption< int > offloadMaxPerCheck;
	static ServerAppOption< int > ghostingMaxPerCheck;

	static ServerAppOption< int > expectedMaxControllers;
	static ServerAppOption< int > absoluteMaxControllers;

	static ServerAppOption< bool > enforceGhostDecorators;
	static ServerAppOption< bool > treatAllOtherEntitiesAsGhosts;

	static ServerAppOption< float > maxPhysicsNetworkJitter;

	static bool postInit();

private:
	static void updateDerivedSettings();
	static bool applySettings();
	static bool sanityCheckSettings();
};

#endif // CELL_APP_CONFIG_HPP
