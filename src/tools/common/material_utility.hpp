/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATERIAL_UTILITY_HPP
#define MATERIAL_UTILITY_HPP

namespace Moo
{
	class EffectMaterial;
};

#include "moo/com_object_wrap.hpp"

class EditorEffectProperty;

class MaterialUtility
{
public:

	/**
	 *	Technique helpers.
	 */
	static int numTechniques( Moo::EffectMaterialPtr material );

	static int listTechniques(
		Moo::EffectMaterialPtr m,
		std::vector<std::string>& retVector );

	static bool MaterialUtility::isTechniqueValid(
		Moo::EffectMaterialPtr m,
		int index );

	static bool MaterialUtility::viewTechnique(
		Moo::EffectMaterialPtr m,
		int index );

	static bool MaterialUtility::viewTechnique(
		Moo::EffectMaterialPtr m,
		const std::string& name );

	static int MaterialUtility::techniqueByName(
		Moo::EffectMaterialPtr material,
		const std::string& name );

	static int currentTechnique( Moo::EffectMaterialPtr material );

	/**
	 *	Tweakable Property helpers.
	 */
	static int numProperties( Moo::EffectMaterialPtr material );

	static int listProperties(
		Moo::EffectMaterialPtr m,
		std::vector<std::string>& retVector );

    static bool artistEditable( EditorEffectProperty* pProperty );
	static bool worldBuilderEditable( EditorEffectProperty* pProperty );
    static std::string UIName( EditorEffectProperty* pPropert );
	static std::string UIDesc( EditorEffectProperty* pPropert );
	static std::string UIWidget( EditorEffectProperty* pPropert );

	static void MaterialUtility::setTexture( Moo::EffectMaterialPtr material,
		int idx, const std::string& textureName );

	/**
	 *	Save a material to a data section.  Note that
	 *	materialProperties / runtimeInitMaterialProperties must have been
	 *	called during application startup in order for the save to work.
	 */
	static void save(
		Moo::EffectMaterialPtr m,
		DataSectionPtr pSection,
		bool worldBuilderEditableOnly = false );
};

#endif