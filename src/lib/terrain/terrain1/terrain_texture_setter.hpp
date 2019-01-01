/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEXTURE_SETTER_HPP
#define TERRAIN_TEXTURE_SETTER_HPP

namespace Terrain
{

/**
 * Abstract base class for a type that sets a vector of textures.
 */
class TerrainTextureSetter
{
public:
	virtual ~TerrainTextureSetter()	{};
	virtual void TerrainTextureSetter::setTextures(
		const std::vector<Moo::BaseTexturePtr>& textures ) = 0;
};

/**
 *	This class sets a vector of textures on a vector of D3DEffect
 *	property handles.
 *
 *	The list of constants is for the terrain effect file renderer,
 *	and this order should be represented in both then handles vector
 *	and the passed in texture vector.
 *
 *	If the effect file needs only 4 textures and the terrain block
 *	passes in 12 textures only the first 4 are set on the effect.
 */
class EffectFileTextureSetter : public TerrainTextureSetter
{
public:
	EffectFileTextureSetter( ComObjectWrap<ID3DXEffect> pEffect )
	{			
		BW_GUARD;
		this->effect( pEffect );
	}

	~EffectFileTextureSetter();

	void effect( ComObjectWrap<ID3DXEffect> pEffect )
	{
		BW_GUARD;
		pEffect_ = pEffect;
		handles_.clear();
		handles_.push_back( pEffect->GetParameterByName(NULL,"Layer0") );
		handles_.push_back( pEffect->GetParameterByName(NULL,"Layer1") );
		handles_.push_back( pEffect->GetParameterByName(NULL,"Layer2") );
		handles_.push_back( pEffect->GetParameterByName(NULL,"Layer3") );		
	}

	void setTextures( const std::vector<Moo::BaseTexturePtr>& textures )
	{
		BW_GUARD;
		for ( uint32 i=0; i<textures.size() && i<handles_.size(); i++ )		
			pEffect_->SetTexture(handles_[i], textures[i]->pTexture());
		pEffect_->CommitChanges();
	}	

private:
	ComObjectWrap<ID3DXEffect> pEffect_;
	std::vector<D3DXHANDLE>	handles_;
};

}

#endif //TERRAIN_TEXTURE_SETTER_HPP
