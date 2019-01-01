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
#include "effect_constant_value.hpp"

namespace Moo
{

// -----------------------------------------------------------------------------
// Section: EffectConstantValue
// -----------------------------------------------------------------------------

/*static*/ void EffectConstantValue::fini()
{
	BW_GUARD;
	Mappings::iterator it = mappings_.begin();
	for (;it != mappings_.end(); it++)
	{
		if (it->second->getObject())
		{
			if (it->second->getObject()->refCount() > 1)
				WARNING_MSG("EffectConstantValue not deleted properly : %s\n", it->first.c_str());

			*(it->second) = NULL;
		}
		delete (it->second);
	}

	mappings_.clear();
}

/**
 * This method returns a pointer to a smartpointer to a EffectConstantValue, this is
 * done so that the material can store the constant value pointer without having
 * to worry about it changing. 
 * @param identifier the name of the constant to get the value for
 * @param createIfMissing create a new constant mapping if one does not exist.
 * @return pointer to a smartpointer to a constant value.
 */
EffectConstantValuePtr* EffectConstantValue::get( const std::string& identifier, bool createIfMissing )
{
	BW_GUARD;
	Mappings::iterator it = mappings_.find( identifier );
    if (it == mappings_.end())
	{
		if (createIfMissing)
		{
			// TODO: make this managed, don't like allocating four byte blocks.
			EffectConstantValuePtr* ppProxy = new EffectConstantValuePtr;
			mappings_.insert( std::make_pair( identifier, ppProxy ) );
			it = mappings_.find( identifier );
		}
		else
		{
			return NULL;
		}
	}

	return it->second;
}

/**
 *	This method sets a constant value for the identifier provided.
 * @param identifier the name of the constant
 * @param pEffectConstantValue the value of the constant
 */
void EffectConstantValue::set( const std::string& identifier, EffectConstantValue* pEffectConstantValue )
{
	BW_GUARD;
	*get(identifier) =  pEffectConstantValue;
}

// The map of constant values.
EffectConstantValue::Mappings	EffectConstantValue::mappings_;

}

// effect_constant_value.cpp
