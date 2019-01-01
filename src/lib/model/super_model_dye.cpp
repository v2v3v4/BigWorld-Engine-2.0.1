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

#include <set>

#include "super_model_dye.hpp"

#include "model.hpp"
#include "super_model.hpp"
#include "tint.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 *	SuperModelDye constructor
 */
SuperModelDye::SuperModelDye( SuperModel & superModel,
	const std::string & matter, const std::string & tint )
{
	BW_GUARD;
	std::set<int>	gotIndices;

	modelDyes_.reserve( superModel.nModels() );

	for (int i = 0; i < superModel.nModels(); i++)
	{
		// Get the dye
		Tint * pTint = NULL;
		modelDyes_.push_back( superModel.topModel(i)->getDye( matter, tint, &pTint ) );

		// Add all its properties to our list
		if (pTint != NULL)
		{
			for (uint j = 0; j < pTint->properties_.size(); j++)
			{
				DyeProperty & dp = pTint->properties_[j];

				if (gotIndices.insert( dp.index_ ).second)
				{
					properties_.push_back( DyePropSetting(
						dp.index_, dp.default_ ) );
				}
			}
		}
	}
}


/**
 *	This method applies the dyes represented by this class to the
 *	input supermodel. It must be the same as it was created with.
 */
void SuperModelDye::dress( SuperModel & superModel )
{
	BW_GUARD;
	// apply property settings to global table
	if (!properties_.empty())
	{
		// totally sucks that we have to take this lock here!
		SimpleMutexHolder smh( Model::propCatalogueLock() );

		for (uint i = 0; i < properties_.size(); i++)
		{
			DyePropSetting & ds = properties_[i];
			(Model::propCatalogueRaw().begin() + ds.index_)->second = ds.value_;
		}
	}

	// now soak the model in our dyes
	for (int i = 0; i < superModel.nModels(); i++)
	{
		Model * cM = superModel.curModel(i);
		if (cM != NULL && !modelDyes_[i].isNull())
			cM->soak( modelDyes_[i] );
	}
}


/**
 *	This method returns whether or not this dye is at all effective
 *	on supermodels it was fetched for. Ineffictive dyes (i.e. dyes
 *	whose matter wasn't found in any of the models) are not returned
 *	to the user from SuperModel's getDye ... NULL is returned instead.
 */
bool SuperModelDye::effective( const SuperModel & superModel )
{
	BW_GUARD;
	for (int i = 0; i < superModel.nModels(); i++)
	{
		if (!modelDyes_[i].isNull())
			return true;
	}

	return false;
}


// super_model_dye.cpp
