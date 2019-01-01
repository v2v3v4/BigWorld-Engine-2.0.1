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
#include "static_light_fashion.hpp"

#include "model/model_static_lighting.hpp"


// -----------------------------------------------------------------------------
// Section: StaticLightFashion
// -----------------------------------------------------------------------------

/**
 *	Factory method.
 */
SmartPointer<StaticLightFashion> StaticLightFashion::get(
	StaticLightValueCachePtr cache, SuperModel & sm,
	const DataSectionPtr modelLightingSection )
{
	char * pBytes = new char[ sizeof(StaticLightFashion)+
		(sm.nModels()-1)*sizeof(ModelStaticLighting*) ];
	StaticLightFashion * pSLF =
		new ( pBytes ) StaticLightFashion( cache, sm, modelLightingSection );

	for (int i = 0; i < pSLF->nModels_; i++)
	{
		if (pSLF->lighting_[i] == NULL)
		{
			delete pSLF;
			return NULL;
		}
	}

	return pSLF;
}


/**
 *	Constructor.
 */
StaticLightFashion::StaticLightFashion( StaticLightValueCachePtr cache, SuperModel & sm,
									   const DataSectionPtr modelLightingSection )
{
	nModels_ = sm.nModels();

	std::vector<DataSectionPtr> dss;

	modelLightingSection->openSections( "staticLighting", dss );

	if (dss.size() != size_t( nModels_ ))
	{
		for (int i = 0; i < nModels_; i++)
		{
			lighting_[i] = NULL;;
		}
	}
	else
	{
		for (int i = 0; i < nModels_; i++)
		{
			ModelStaticLightingPtr pSL = sm.topModel(i)->getStaticLighting( cache, dss[i] );
			lighting_[i] = &*pSL;
			if (pSL)
			{
				pSL->incRef();
			}
			else
				lighting_[i] = NULL;
		}
	}
}

/**
 *	Destructor.
 */
StaticLightFashion::~StaticLightFashion()
{
	for (int i = 0; i < nModels_; i++)
		if (lighting_[i])
			lighting_[i]->decRef();
}


/**
 *	Dress method.
 */
void StaticLightFashion::dress( SuperModel & superModel )
{
	for (int i = 0; i < nModels_; i++)
	{
		if (lighting_[i] != NULL) lighting_[i]->set();
	}
}

void StaticLightFashion::undress( SuperModel & superModel )
{
	for (int i = 0; i < nModels_; i++)
	{
		if (lighting_[i] != NULL) lighting_[i]->unset();
	}
}


std::vector<StaticLightValuesPtr> StaticLightFashion::staticLightValues()
{
	std::vector<StaticLightValuesPtr> v;
	v.reserve( nModels_ );
	for (int i = 0; i < nModels_; i++)
		if (lighting_[i])
			v.push_back( lighting_[i]->staticLightValues() );
		else
			v.push_back( NULL );


	return v;
}


#ifdef EDITOR_ENABLED
bool StaticLightFashion::isDataValid() const
{
	if (nModels_)
	{
		return lighting_[0]->staticLightValues()->cache()->isDataValid();
	}

	return true;
}
#endif//EDITOR_ENABLED


std::string StaticLightFashion::lightingTag( int index, int count )
{
	MF_ASSERT(index < 100000000);
	char st[10];
	bw_snprintf(st, sizeof(st), "%d", index);
	return st;
}

// static_light_fashion.cpp
