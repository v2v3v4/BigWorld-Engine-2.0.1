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

#ifndef SUPER_MODEL_DYE_HPP
#define SUPER_MODEL_DYE_HPP

#include "forward_declarations.hpp"
#include "dye_prop_setting.hpp"
#include "fashion.hpp"
#include "model_dye.hpp"



/**
 *	This class represents a material override from the SuperModel
 *	perspective
 */
class SuperModelDye : public Fashion
{
	friend class SuperModel;

	SuperModelDye(	SuperModel & superModel,
				const std::string & matter,
				const std::string & tint );
public:
	typedef std::vector< DyePropSetting > DyePropSettings;
	DyePropSettings		properties_;

private:
	virtual void dress( SuperModel & superModel );
	bool	effective( const SuperModel & superModel );

	typedef std::vector< ModelDye >		ModelDyes;
	ModelDyes		modelDyes_;
};




#endif // SUPER_MODEL_DYE_HPP
