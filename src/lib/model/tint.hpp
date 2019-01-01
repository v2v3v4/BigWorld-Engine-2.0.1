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

#ifndef MODEL_TINT_HPP
#define MODEL_TINT_HPP

#include "moo/forward_declarations.hpp"

#include "dye_property.hpp"
#include "dye_selection.hpp"


class Tint;
typedef SmartPointer< Tint > TintPtr;


/**
 *	Inner class to represent a tint
 *	@todo update
 */
class Tint : public ReferenceCount
{
public:
	Tint( const std::string & name, bool defaultTint = false );
	virtual ~Tint();

	void applyTint();

	void updateFromEffect();

	std::string name_;

	Moo::EffectMaterialPtr					effectMaterial_;	// newvisual
	std::vector< const DyeProperty >		properties_;		// *visual
	std::vector< const DyeSelection >		sourceDyes_;		// textural

private:
	Tint( const Tint & );
	Tint & operator=( const Tint & );

	bool default_;
};


#endif // MODEL_TINT_HPP
