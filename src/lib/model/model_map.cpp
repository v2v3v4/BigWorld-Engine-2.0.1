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

#include "model_map.hpp"

#include "model.hpp"



DECLARE_DEBUG_COMPONENT2( "Model", 0 )



ModelMap::ModelMap()
{
	BW_GUARD;
}


ModelMap::~ModelMap()
{
	BW_GUARD;
}


/**
 *	Add a model to the map
 */
void ModelMap::add( Model * pModel, const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( sm_ );

	//map_[ resourceID ] = pModel;
	Map::iterator found = map_.find( resourceID );
	if (found == map_.end())
	{
		found = map_.insert( std::make_pair( resourceID, pModel ) ).first;
	}
	else
	{
		found->second = pModel;
	}
}

/**
 *	Delete a model from the map, using resourceID
 */
void ModelMap::del( const std::string& resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( sm_ );

	Map::iterator it = map_.find( resourceID);
	if (it != map_.end())
	{
		map_.erase( it );
	}
}

/**
 *	Delete a model from the map
 */
void ModelMap::del( Model * pModel )
{
	BW_GUARD;
	SimpleMutexHolder smh( sm_ );

	for (Map::iterator it = map_.begin(); it != map_.end(); it++)
	{

		if (it->second == pModel)
		{
			//// Removed this debug message since it clutters
			////      the message window very quickly.
			//DEBUG_MSG( "ModelMap::del: %s\n", it->first.c_str() );

			map_.erase( it );
			return;
		}
	}

	/* // Removed this message since it is often incorrectly reported
        //    and looks ugly ;-)
        if (map_.size())
	{
		WARNING_MSG( "ModelMap::del: "
			"Could not find model '%s' at 0x%08X to delete it "
			"(only ok if it was reloaded or was invalid)\n",
			pModel->resourceID().c_str(), pModel );
	}
        */
}


/**
 *	Find a model in the map
 */
ModelPtr ModelMap::find( const std::string & resourceID )
{
	BW_GUARD;
	SimpleMutexHolder smh( sm_ );

	Map::iterator found = map_.find( resourceID.c_str() );
	if (found == map_.end()) return NULL;

	return ModelPtr( found->second, ModelPtr::FALLIBLE );
	// if ref count was zero then someone is blocked on our
	// sm_ mutex waiting to delete it.
}


/**
 *	Find all the models in the map that are the immediate children of
 *	a model with the given resource ID
 */
void ModelMap::findChildren( const std::string & parentResID,
	std::vector< ModelPtr > & children )
{
	BW_GUARD;
	SimpleMutexHolder smh( sm_ );

	children.clear();

	for (Map::iterator it = map_.begin(); it != map_.end(); it++)
	{
		Model * chPar = it->second->parent();
		if (chPar != NULL && chPar->resourceID() == parentResID)
		{
			ModelPtr mp( it->second, ModelPtr::FALLIBLE );
			// see note in find about the necessity of this
			if (mp) children.push_back( mp );
		}
	}
}




// model_map.cpp
