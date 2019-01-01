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

#ifndef MODEL_DYE_HPP
#define MODEL_DYE_HPP


class Model;


/**
 *	This class stores the indexes necessary to apply a dye to a specific
 *	model.
 */
class ModelDye
{
public:
	ModelDye( const Model & model, int matterIndex, int tintIndex );

	ModelDye( const ModelDye & modelDye );
	ModelDye & operator=( const ModelDye & modelDye );

	bool isNull() const;

	int16 matterIndex() const;
	int16 tintIndex() const;

private:
	int16	matterIndex_;
	int16	tintIndex_;
};



#endif // MODEL_DYE_HPP