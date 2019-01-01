/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cstdmf/stdmf.hpp"
#include "server/add_to_manager_helper.hpp"
#include "server/cell_app_init_data.hpp"

/**
 *  This class helps to add this app to the CellAppMgr.
 */
class AddToCellAppMgrHelper : public AddToManagerHelper
{
public:
	AddToCellAppMgrHelper( CellApp & cellApp, uint16 viewerPort ) :
		AddToManagerHelper( cellApp.mainDispatcher() ),
		app_( cellApp ),
		viewerPort_( viewerPort )
	{
		// Auto-send on construction.
		this->send();
	}


	void handleFatalTimeout()
	{
		ERROR_MSG( "AddToCellAppMgrHelper::handleFatalTimeout: Unable to add "
			"CellApp to CellAppMgr. Terminating.\n" );
		app_.mainDispatcher().breakProcessing();
	}


	void doSend()
	{
		app_.cellAppMgr().add( app_.interface().address(), viewerPort_, this );
	}


	bool finishInit( BinaryIStream & data )
	{
		CellAppInitData initData;
		data >> initData;
		return app_.finishInit( initData );
	}

private:
	CellApp & app_;
	uint16 viewerPort_;
};
