/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROXIMITY_CONTROLLER_HPP
#define PROXIMITY_CONTROLLER_HPP

#include "controller.hpp"
#include "pyscript/script.hpp"

class ProximityRangeTrigger;

/**
 *  This class is the controller for placing proximity based triggers.
 *  or traps.
 *  When an entity crosses within a range of the source entity it
 *  will call the python script method onEnterTrap() and when
 *  an entity leaves the range, it will call the method onLeaveTrap()
 */

class ProximityController : public Controller
{
	DECLARE_CONTROLLER_TYPE( ProximityController )

public:
	ProximityController( float range = 20.f );
	virtual ~ProximityController();

	virtual void	startReal( bool isInitialStart );
	virtual void	stopReal( bool isFinalStop );

	void	writeRealToStream( BinaryOStream & stream );
	bool 	readRealFromStream( BinaryIStream & stream );

	float	range()			{ return range_; }

	void setRange( float range );

	static FactoryFnRet New( float range, int userArg = 0 );
	PY_AUTO_CONTROLLER_FACTORY_DECLARE( ProximityController,
		ARG( float, OPTARG( int, 0, END ) ) )

private:
	float	range_;
	ProximityRangeTrigger* pProximityTrigger_;

	std::vector< EntityID > * pOnloadedSet_;
};

#endif // PROXIMITY_CONTROLLER_HPP
