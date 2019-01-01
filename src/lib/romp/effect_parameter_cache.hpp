/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECTPARAMETERCACHE_HPP
#define EFFECTPARAMETERCACHE_HPP

namespace Moo
{
	class EffectMaterial;
};

#define DECLARE_TYPE_SETTER( X, Y )								\
	HRESULT set##X( const std::string& name, Y v )				\
	{															\
		return spEffect_->Set##X( parameter(name), v );			\
	}															\


/**
 *	This class is a simple helper for setting manual parameters
 *	on effects.  It replaces the D3DXEffect->SetTYPE( paramName )
 *	which is not recommended for runtime use due to perforamance
 *	of the handle lookup, with a simple cache that allows you
 *	to do exactly the same thing.
 */
class EffectParameterCache
{
public:	
	EffectParameterCache():
	  spEffect_(NULL)
	{
	}

	~EffectParameterCache();

	void clear()
	{
		parameters_.clear();
	}

	bool hasEffect() const
	{
		return ( spEffect_.pComObject() != NULL );
	}

	void effect( ComObjectWrap<ID3DXEffect> spEffect )
	{
		if (spEffect.pComObject() != spEffect_.pComObject())
		{
			spEffect_ = spEffect;
			this->clear();
		}
	}

	void commitChanges()
	{
		spEffect_->CommitChanges();
	}

	DECLARE_TYPE_SETTER( Matrix, const Matrix* )
	DECLARE_TYPE_SETTER( Texture, DX::BaseTexture* )
	DECLARE_TYPE_SETTER( Vector, const Vector4* )
	DECLARE_TYPE_SETTER( Float, float )
	DECLARE_TYPE_SETTER( Bool, bool )

	HRESULT setVectorArray( const std::string& name, const Vector4* v, size_t n )
	{
		return spEffect_->SetVectorArray( parameter(name), v, n );
	}	

private:
	void cache(const std::string& name );
	D3DXHANDLE parameter( const std::string& key )
	{
		if (parameters_.find(key) == parameters_.end())
		{
			this->cache(key);
		}
		return parameters_[key];
	}
	std::map<std::string, D3DXHANDLE> parameters_;
	ComObjectWrap<ID3DXEffect> spEffect_;
};

#endif //EFFECTPARAMETERCACHE_HPP