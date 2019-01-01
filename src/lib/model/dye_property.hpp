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

#ifndef DYE_PROPERTY_HPP
#define DYE_PROPERTY_HPP

#include "math/vector4.hpp"


/**
 *	Inner class to represent a dye property
 */
class DyeProperty
{
public:
	int				index_;
	int				controls_;
	int				mask_;
	int				future_;
	Vector4			default_;
};



#endif // DYE_PROPERTY_HPP
