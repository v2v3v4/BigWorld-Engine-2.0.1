/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "model_dye.hpp"


ModelDye::ModelDye( const Model & model, int matterIndex, int tintIndex )
	:	matterIndex_( matterIndex ),
		tintIndex_( tintIndex )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( matterIndex >= std::numeric_limits< int16 >::min() )
	{
		MF_EXIT( "matterIndex is outside the required range < std::numeric_limits< int16 >::min()" );
	}
	IF_NOT_MF_ASSERT_DEV( matterIndex <= std::numeric_limits< int16 >::max() )
	{
		MF_EXIT( "matterIndex is outside the required range > std::numeric_limits< int16 >::max()" );
	}

	IF_NOT_MF_ASSERT_DEV( tintIndex >= std::numeric_limits< int16 >::min() )
	{
		MF_EXIT( "tintIndex is outside the required range < std::numeric_limits< int16 >::min()" );
	}

	IF_NOT_MF_ASSERT_DEV( tintIndex <= std::numeric_limits< int16 >::max() )
	{
		MF_EXIT( "tintIndex is outside the required range > std::numeric_limits< int16 >::max()" );
	}

	//MF_ASSERT( matterIndex == -1 || matterIndex < model.
}


ModelDye::ModelDye( const ModelDye & modelDye )
	:	matterIndex_( modelDye.matterIndex_ ),
		tintIndex_( modelDye.tintIndex_ )
{
	BW_GUARD;
}


ModelDye & ModelDye::operator=( const ModelDye & modelDye )
{
	BW_GUARD;
	matterIndex_ = modelDye.matterIndex_;
	tintIndex_ = modelDye.tintIndex_;

	return *this;
}


bool ModelDye::isNull() const
{
	return (matterIndex_ == -1) && (tintIndex_ == -1);
}


int16 ModelDye::matterIndex() const
{
	return matterIndex_;
}


int16 ModelDye::tintIndex() const
{
	return tintIndex_;
}


// model_dye.cpp