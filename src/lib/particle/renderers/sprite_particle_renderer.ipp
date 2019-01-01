/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

/**
 *	This is the Get-Accessor for the materialFX property. This property
 *	defines how the material texture is drawn with respect to the background.
 *
 *	@return	The current setting for the materialFX property.
 */
INLINE SpriteParticleRenderer::MaterialFX
	SpriteParticleRenderer::materialFX() const
{
	return materialFX_;
}

/**
 *	This is the Set-Accessor for the materialFX property.
 *
 *	@param newMaterialFX	The new setting for the materialFX property.
 */
INLINE void SpriteParticleRenderer::materialFX(
	SpriteParticleRenderer::MaterialFX newMaterialFX )
{
	materialFX_ = newMaterialFX;
	materialSettingsChanged_ = true;
}

/**
 *	This is the Get-Accessor for the texture file name of the sprite.
 *
 *	@return	A string containing the file name of the sprite texture.
 */
INLINE const std::string &SpriteParticleRenderer::textureName() const
{
	return textureName_;
}
