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

#ifndef MODEL_FORWARD_DECLARATIONS_HPP
#define MODEL_FORWARD_DECLARATIONS_HPP



class Fashion;
typedef SmartPointer< Fashion >						FashionPtr;
typedef ConstSmartPointer< Fashion >				ConstFashionPtr;

class Matter;

class Model;
typedef SmartPointer< Model >						ModelPtr;
typedef ConstSmartPointer< Model >					ConstModelPtr;

class ModelAction;
typedef SmartPointer< ModelAction >					ModelActionPtr;
typedef ConstSmartPointer< ModelAction >			ConstModelActionPtr;

class ModelActionsIterator;

class ModelAnimation;
typedef SmartPointer< ModelAnimation >				ModelAnimationPtr;
typedef ConstSmartPointer< ModelAnimation >			ConstModelAnimationPtr;

class ModelStaticLighting;
typedef SmartPointer< ModelStaticLighting >			ModelStaticLightingPtr;
typedef ConstSmartPointer< ModelStaticLighting >	ConstModelStaticLightingPtr;

class ModelMap;

class SuperModel;
typedef SmartPointer< SuperModel >					SuperModelPtr;
typedef ConstSmartPointer< SuperModel >				ConstSuperModelPtr;

class SuperModelAction;
typedef SmartPointer< SuperModelAction >			SuperModelActionPtr;
typedef ConstSmartPointer< SuperModelAction >		ConstSuperModelActionPtr;

class SuperModelAnimation;
typedef SmartPointer< SuperModelAnimation >			SuperModelAnimationPtr;
typedef ConstSmartPointer< SuperModelAnimation >	ConstSuperModelAnimationPtr;

class SuperModelDye;
typedef SmartPointer< SuperModelDye >				SuperModelDyePtr;
typedef ConstSmartPointer< SuperModelDye >			ConstSuperModelDyePtr;

class Tint;
typedef SmartPointer< Tint >						TintPtr;
typedef ConstSmartPointer< Tint >					ConstTintPtr;



#endif // MODEL_FORWARD_DECLARATIONS_HPP
