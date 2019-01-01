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

#include "model_factory.hpp"

#include "resmgr/datasection.hpp"

#include "nodefull_model.hpp"
#include "nodeless_model.hpp"



DECLARE_DEBUG_COMPONENT2( "Model", 0 )



ModelFactory::ModelFactory(	const std::string& resourceID,
							DataSectionPtr& pFile ) :
		resourceID_( resourceID ),
		pFile_( pFile )
{
	BW_GUARD;
}


NodefullModel * ModelFactory::newNodefullModel()
{
	BW_GUARD;
	NodefullModel * model = new NodefullModel( resourceID_, pFile_ );

	if (model && model->valid())
		return model;
	else
		return NULL;
}

NodelessModel * ModelFactory::newNodelessModel()
{
	BW_GUARD;
	return new NodelessModel( resourceID_, pFile_ );
}


//Model * ModelFactory::newBillboardModel()
//{
//	return new BillboardModel( resourceID_, pFile_ );
//}


// model_factory.cpp
