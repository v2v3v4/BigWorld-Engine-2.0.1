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

#ifndef DYE_SELECTION_HPP
#define DYE_SELECTION_HPP


#include "dye_prop_setting.hpp"


/**
 *	Inner class to represent a dye selection (for billboards)
 */
class DyeSelection
{
public:
	std::string		matterName_;
	std::string		tintName_;
	std::vector< DyePropSetting > properties_;
};


#endif // DYE_SELECTION_HPP
