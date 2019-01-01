/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_LIGHTING_SETTER_HPP
#define EFFECT_LIGHTING_SETTER_HPP

#include "moo_dx.hpp"

namespace Moo
{

class EffectMaterial;
class LightContainer;
class EffectConstantValue;
class ManagedEffect;

typedef SmartPointer<LightContainer>		LightContainerPtr;
typedef SmartPointer<EffectConstantValue>	EffectConstantValuePtr;
typedef SmartPointer<ManagedEffect>			ManagedEffectPtr;
/**
 *	This class is a simple mapping between an effect handle and an
 *	EffectConstant, stored with the name of the semantic.
 */
class SemanticMapping
{
public:
	std::string				name_;
	D3DXHANDLE				handle_;
	EffectConstantValuePtr*	value_;
};
typedef std::vector<SemanticMapping>	Semantics;


/**
 *	This class handles setting effect variables by semantic
 *	on an effect material, if the effect material is to be
 *	used multiple times with different values in between
 *	begin() and end().
 *
 *	for example,
 *
 *	EffectLightingSetter::begin( pMat )		//cache semantics if necessary, and cache EffectConstantValues just once.
 *	pMat->begin()
 *	pMat->beginPass()
 *
 *	for (uint32 i=0; i<500; i++)
 *	{
 *		Moo::rc().lightContainer( lightcontiner_[i] )
 *		EffectLightingSetter::apply( pMat )
 *		draw( object_[i] )
 *	}
 *
 *	pMat->endPass()
 *	pMat->end()
 *
 *	Because the EffectMaterial is stored as a normal pointer, creators
 *	of this class must destroy the EffectLightingSetter when they destroy
 *	the material, or else there will be a dangling pointer.
 */
class EffectLightingSetter
{
public:
	typedef int32 BatchCookie;

	EffectLightingSetter( ManagedEffectPtr pEffect );

	void begin();
	//If you want to use batch cookies, then resetBatchCookie must
	//be called whenever the Effect's begin() is called.
	void resetBatchCookie()	{ lastBatch_ = NO_BATCH; }
	void apply(
		LightContainerPtr pDiffuse,
		LightContainerPtr pSpecular,
		bool commitChanges = true,
		BatchCookie batch = NO_BATCHING );
	static const uint32 NO_BATCHING = 0xC00CEE00;
private:
	void addSemantic( const std::string& name );

	Semantics		semantics_;
	ManagedEffectPtr pEffect_;
	BatchCookie		lastBatch_;

	static const uint32 NO_BATCH = 0xC00CEEEE;
};	//class EffectLightingSetter

}	//namespace Moo

#endif