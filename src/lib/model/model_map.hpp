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

#ifndef MODEL_MAP_HPP
#define MODEL_MAP_HPP

#include "forward_declarations.hpp"


/**
 *	This class is a map of all the currently loaded models.
 */
class ModelMap
{
public:
	ModelMap();
	~ModelMap();

	void add( Model * pModel, const std::string & resourceID );
	void del( const std::string& resourceID );
	void del( Model * pModel );

	ModelPtr find( const std::string & resourceID );

	void findChildren(	const std::string & parentResID,
						std::vector< ModelPtr > & children );
private:
	//typedef	StringMap<Model*> Map;
	typedef	std::map<std::string,Model*> Map;

	Map					map_;
	SimpleMutex			sm_;
};


#endif // MODEL_MAP_HPP
