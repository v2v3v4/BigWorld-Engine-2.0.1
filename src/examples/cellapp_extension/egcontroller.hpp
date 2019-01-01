/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EGCONTROLLER_HPP
#define EGCONTROLLER_HPP

#include "cellapp/controller.hpp"
#include "cellapp/updatable.hpp"

class EgController : public Controller, public Updatable
{
	DECLARE_CONTROLLER_TYPE( EgController );

public:
	virtual void startReal( bool isInitialStart );
	virtual void stopReal( bool isFinalStop );
	virtual void update();
};

#endif // EGCONTROLLER_HPP
