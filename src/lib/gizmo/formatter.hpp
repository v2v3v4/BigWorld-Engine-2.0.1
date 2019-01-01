/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORMATER_HPP_
#define FORMATER_HPP_

#include "resmgr/string_provider.hpp"
#include "cstdmf/string_utils.hpp"

/**
 *	This templatised class formats a label string.
 */
template<class T>
class LabelFormatter
{
public:
	virtual const std::string format( const T& value ) = 0;
protected:
	std::string label_;
};


/**
 *	This class labels a float as a distance.
 */
class DistanceFormatter : public LabelFormatter<float>
{
public:
	const std::string format( const float& value )
	{
		BW_GUARD;

		if ( fabsf( value ) < 1.f )
		{
			label_ = LocaliseUTF8(L"GIZMO/PROPERTIES/FORMATTER/CM", Formatter( value * 100.f, L"%0.0f" ) );
		}
		else if ( fabsf( value ) < 10.f )
		{
			label_ = LocaliseUTF8(L"GIZMO/PROPERTIES/FORMATTER/M", Formatter( value, L"%0.2f" ) );
		}
		else if ( fabsf( value ) < 1000.f )
		{
			label_ = LocaliseUTF8(L"GIZMO/PROPERTIES/FORMATTER/M", Formatter( value, L"%0.1f" ) );
		}
		else
		{
			label_ = LocaliseUTF8(L"GIZMO/PROPERTIES/FORMATTER/KM", Formatter( value / 1000.f, L"%0.3f" ) );
		}

		return label_;
	}

	static DistanceFormatter s_def;
};


/**
 *	This class labels a radians float as a degrees angle.
 */
class AngleFormatter : public LabelFormatter<float>
{
public:
	const std::string format( const float& value )
	{
		BW_GUARD;

		label_ = LocaliseUTF8(L"GIZMO/PROPERTIES/FORMATTER/DEGREES", Formatter( value, L"%0.1f" ) );
		return label_;
	}

	static AngleFormatter s_def;
};

class SimpleFormatter : public LabelFormatter<float>
{
public:
	const std::string format( const float& value )
	{
		BW_GUARD;

		bw_wtoutf8( Formatter( value, L"%0.3f" ).str(), label_ );
		return label_;
	}

	static SimpleFormatter s_def;
};

#endif