/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef MOO_FORWARD_DECLARATIONS_HPP
#define MOO_FORWARD_DECLARATIONS_HPP


namespace Moo
{

	class Animation;
	typedef SmartPointer<Animation>				AnimationPtr;
	typedef ConstSmartPointer<Animation>		ConstAnimationPtr;

	class EffectMaterial;
	typedef SmartPointer<EffectMaterial>		EffectMaterialPtr;
	typedef ConstSmartPointer<EffectMaterial>	ConstEffectMaterialPtr;

	class EffectProperty;
	typedef SmartPointer<EffectProperty>		EffectPropertyPtr;
	typedef ConstSmartPointer<EffectProperty>	ConstEffectPropertyPtr;

	class Material;
	typedef SmartPointer<Material>				MaterialPtr;
	typedef ConstSmartPointer<Material>			ConstMaterialPtr;

	class ManagedEffect;
	typedef SmartPointer<ManagedEffect>			ManagedEffectPtr;
	typedef ConstSmartPointer<ManagedEffect>	ConstManagedEffectPtr;

	class Node;
	typedef SmartPointer<Node>					NodePtr;
	typedef ConstSmartPointer<Node>				ConstNodePtr;

	class StreamedDataCache;

	class Visual;
	typedef SmartPointer<Visual>				VisualPtr;
	typedef ConstSmartPointer<Visual>			ConstVisualPtr;

	class VisualChannel;
	typedef SmartPointer<VisualChannel>			VisualChannelPtr;
	typedef ConstSmartPointer<VisualChannel>	ConstVisualChannelPtr;

}



#endif // MOO_FORWARD_DECLARATIONS_HPP