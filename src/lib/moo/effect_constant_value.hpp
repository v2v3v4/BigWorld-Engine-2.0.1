/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_CONSTANT_VALUE_HPP
#define EFFECT_CONSTANT_VALUE_HPP

// DX Headers
#include <d3dx9effect.h>

namespace Moo
{

typedef SmartPointer<class EffectConstantValue> EffectConstantValuePtr;
typedef SmartPointer<class RecordedEffectConstant> RecordedEffectConstantPtr;

/**
 *	This class implements the constant value base class.
 *	
 */
class EffectConstantValue : public ReferenceCount
{
public:
	virtual ~EffectConstantValue(){}
	virtual bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle) = 0;


	static void fini();
	static EffectConstantValuePtr* get( const std::string& identifier, bool createIfMissing = true );
	static void set( const std::string& identifier, EffectConstantValue* pEffectConstantValue );

	virtual class RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle) { return NULL; }
private:
	typedef std::map<std::string, EffectConstantValuePtr*> Mappings;
	static Mappings	mappings_;
};

/**
 *	This class defines the interface for recorded effect constants. Mainly used for
 *	batching / deferred channel state recording.
 */
class RecordedEffectConstant : public ReferenceCount
{
public:
	virtual ~RecordedEffectConstant(){};
	virtual void apply() = 0;
private:
};


}
#endif // EFFECT_CONSTANT_VALUE_HPP
