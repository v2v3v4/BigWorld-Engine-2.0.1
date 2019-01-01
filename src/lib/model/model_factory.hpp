/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef MODEL_FACTORY_HPP
#define MODEL_FACTORY_HPP


#include "resmgr/forward_declarations.hpp"

#include "model.hpp"

class NodefullModel;
class NodelessModel;


// This factory class is passed to Moo::createModelFromFile()
class ModelFactory
{
	const std::string& 	resourceID_;
	DataSectionPtr& 	pFile_;
public:
	typedef Model ModelBase;

	ModelFactory( const std::string& resourceID, DataSectionPtr& pFile );

	NodefullModel * newNodefullModel();

	NodelessModel * newNodelessModel();

	//Model* newBillboardModel();
};


#endif // MODEL_FACTORY_HPP
